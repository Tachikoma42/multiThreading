#include "plant.h"


partWork::partWork(int workerId, int waitLength, int partNumber, string logLocation) {
    this->id = workerId;
    this->fileLocation = std::move(logLocation);
    this->partSize = partNumber;
    this->oldOrder = {0, 0, 0, 0, 0};
    this->loadOrder = {0, 0, 0, 0, 0};
    this->accumulatedWaitTime = chrono::microseconds(0);
    this->waitTimeLength = waitLength;
    this->waitTime = chrono::microseconds(waitTimeLength);
}

void partWork::freshStart() {
    this->loadOrder.clear();
    this->loadOrder = {0, 0, 0, 0, 0};
    this->accumulatedWaitTime = chrono::microseconds(0);
    this->start = globalStart;
    this->waitTime = chrono::microseconds(waitTimeLength);
    partOrderGenerator();
}

void partWork::partOrderGenerator() {
    int currentHave = std::accumulate(this->oldOrder.begin(), this->oldOrder.end(), 0);
    this->loadOrder.assign(this->oldOrder.begin(), this->oldOrder.end());
    for (int i = 0; i < this->partSize - currentHave; ++i) {
        this->loadOrder[random(0, 4)]++;
    }
}

int partWork::buildPartTime() {
    int totalTime = 0;
    for (int i = 0; i < this->partSize; ++i) {
        totalTime += produceTime[i] * (this->loadOrder[i] - oldOrder[i]);
    }
    return totalTime;
}

int partWork::movePartTime() {
    int totalTime = 0;
    for (int i = 0; i < this->partSize; ++i) {
        totalTime += moveTime[i] * this->loadOrder[i];
    }
    return totalTime;
}

bool partWork::canLoadToBuffer(vector<int> &bufferStates) {
    for (int i = 0; i < this->partSize; ++i) {
        if (bufferStates[i] < bufferSize[i] && this->loadOrder[i] != 0) {
            return true;
        } else {
            continue;
        }
    }
    return false;
}
//bool canLoadToBuffer() {
//    for (int i = 0; i < 5; ++i) {
//        if (bufferState[i] < bufferSize[i] && globalLoadOrder[i] != 0) {
//            return true;
//        }
//        else {
//            continue;
//        }
//    }
//    return false;
//}

void partWork::loadToBuffer(vector<int> &bufferStates) {
    for (int i = 0; i < this->partSize; ++i) {
        if (bufferStates[i] < bufferSize[i] && this->loadOrder[i] != 0) {
            int itemToLoad = min(this->loadOrder[i], bufferSize[i] - bufferStates[i]);
            this->loadOrder[i] -= itemToLoad;
            bufferStates[i] += itemToLoad;
        }
    }

}

bool partWork::isEmptyOrder() {
    return std::all_of(this->loadOrder.begin(), this->loadOrder.end(), [](int item) { return item != 0; });
}

//void partWork::emptyOldOrder() {
//    this->oldOrder.clear();
//    this->oldOrder = {0,0,0,0,0};
//}

void partWork::transferOrder() {
    for (int i = 0; i < this->partSize; ++i) {
        oldOrder[i] = loadOrder[i];
    }
}


void partWork::updateWaitTime(durationTimeVariable waitedTime) {
    this->accumulatedWaitTime += waitedTime;
    this->waitTime -= waitedTime;
}

void partWork::printBeforeStatus(int iteration, int status, const vector<int> &bufferStates) {
    /// this function need protection
    lock_guard<mutex> LG1(m2);
    this->end = chrono::high_resolution_clock::now();
    ofstream log;
    log.open(this->fileLocation);
    log << "Current Time: " << calTimePassed(this->start, this->end).count() << " us" << endl;
    log << "Iteration: " << iteration << endl;
    log << "Part Worker ID: " << this->id << endl;
    log << statusVector[status] << endl;
    log << "Accumulated Wait Time: " << this->accumulatedWaitTime.count() << " us" << endl;
    log << "Buffer State: (" << bufferStates[0] << "," << bufferStates[1] << ",";
    log << bufferStates[2] << "," << bufferStates[3] << "," << bufferStates[4] << ")" << endl;
    log << "Load Order: (" << this->loadOrder[0] << "," << this->loadOrder[1] << ",";
    log << this->loadOrder[2] << "," << this->loadOrder[3] << "," << this->loadOrder[4] << ")" << endl;
    log.close();
}

void partWork::printAfterState(const vector<int> &bufferStates) {
    lock_guard<mutex> LG1(m2);
    ofstream log;
    log.open(this->fileLocation);
    log << "Updated Buffer State: (" << bufferStates[0] << "," << bufferStates[1] << ",";
    log << bufferStates[2] << "," << bufferStates[3] << "," << bufferStates[4] << ")" << endl;
    log << "Updated Load Order: (" << this->loadOrder[0] << "," << this->loadOrder[1] << ",";
    log << this->loadOrder[2] << "," << this->loadOrder[3] << "," << this->loadOrder[4] << ")" << endl;
    log.close();
}

bool partWork::partThreadWait(unique_lock<mutex>&lck, condition_variable &cv, vector<int> &bufferStates) {
    while (!canLoadToBuffer(bufferStates))
        if ( cv.wait_for(lck,waitTime) == cv_status::timeout)
            return canLoadToBuffer(bufferStates);
    return true;
}


productWork::productWork(int workerId, int waitLength, int partNumber, string logLocation) {
    this->id = workerId;
    this->fileLocation = std::move(logLocation);
    this->partSize = partNumber;
    this->localState = {0, 0, 0, 0, 0};
    this->pickUpOrder = {0, 0, 0, 0, 0};
    this->cartState = {0, 0, 0, 0, 0};
    this->tmpLoad = {0, 0, 0, 0, 0};
    this->accumulatedWaitTime = chrono::microseconds(0);
    this->waitTimeLength = waitLength;
    this->waitTime = chrono::microseconds(waitTimeLength);
}

void productWork::freshStart() {
    this->accumulatedWaitTime = chrono::microseconds(0);
    this->start = globalStart;
    this->waitTime = chrono::microseconds(waitTimeLength);
    this->cartState = this->localState; //TODO: need check if works.
    this->pickUpOrder.clear();
    this->pickUpOrder = {0, 0, 0, 0, 0};
    this->tmpLoad = {0, 0, 0, 0, 0};
    assemblyOrderGenerator();
}

void productWork::assemblyOrderGenerator() {
    int curHave = std::accumulate(localState.begin(), localState.end(), 0);
    int typeCount = 0;
    if (curHave != 0) {
        for (const auto &part: localState) {
            if (part != 0) {
                typeCount++;
            }
        }
    }
    /// generate new order
    vector<int> tmpOrder(maxPartSize);
    while (true) {
        tmpOrder.clear();
        tmpOrder.assign(localState.begin(), localState.end());
        int tmpTypeCount = typeCount;
        for (int i = 0; i < maxPartSize - curHave;) {
            int newPart = random(0, 4);
            if (tmpOrder[newPart] == 0 && tmpTypeCount < maxTypeCount) {
                tmpTypeCount++;
                tmpOrder[newPart]++;
                ++i;
            } else if (tmpOrder[newPart] != 0) {
                tmpOrder[newPart]++;
                ++i;
            } else {
                continue;
            }
        }
        if (tmpTypeCount != 1) {
            break;
        }
    }
    this->pickUpOrder.clear();
    for (int i = 0; i < 5; ++i) {
        pickUpOrder[i] = tmpOrder[i]-localState[i];
    }
    //this->pickUpOrder.assign(tmpOrder.begin(), tmpOrder.end());
}



bool productWork::canLoadToCart(vector<int> &bufferStates) {
    for (int i = 0; i < maxPartSize; ++i) {
        if (pickUpOrder[i]>0 && bufferStates[i] != 0) {
            return true;
        } else {
            continue;
        }
    }
    return false;

}

bool productWork::finishedLoad() {
//    return (cartState == pickUpOrder);
    //return pickUpOrder == localState;
    return std::all_of(this->pickUpOrder.begin(), this->pickUpOrder.end(), [](int item) { return item == 0; });

}

void productWork::loadToCart(vector<int> &bufferStates) {

    for (int i = 0; i < maxPartSize; ++i) {
        if (pickUpOrder[i] > 0 && bufferStates[i] != 0) {
            tmpLoad[i] = min(pickUpOrder[i] , bufferStates[i]);
            bufferStates[i] -= tmpLoad[i];
        }
    }
    for (int i = 0; i < maxPartSize; ++i) {
        cartState[i] += tmpLoad[i];
        pickUpOrder[i]-=tmpLoad[i];
    }
}

void productWork::updateWaitTime(durationTimeVariable waitedTime) {
    this->accumulatedWaitTime += waitedTime;
    this->waitTime -= waitedTime;
}

int productWork::movePartTime() {
    int totalTime = 0;
    for (int i = 0; i < maxPartSize; ++i) {
        totalTime += moveTime[i] * tmpLoad[i];
    }
    tmpLoad.clear(); // TODO: empty tmpLoad
    return totalTime;
}

int productWork::assemblePartTime() {
    int totalTime = 0;
    for (int i = 0; i < maxPartSize; ++i) {
        totalTime += assemblyTime[i] * (cartState[i]+ localState[i]);
    }

    return totalTime;
}

void productWork::buildNewParts(int &totalCompleteProductCount) {
    lock_guard<mutex> LG1(m3);
    totalCompleteProduct++; // TODO: may need protection
    cartState = {0,0,0,0,0};
    localState = {0,0,0,0,0};

}

void productWork::waitForParts() {
    localState = cartState; // TODO: may not work
}

void productWork::printBeforeStatus(int iteration, int status, const vector<int> &bufferStates) {
/// this function need protection
    lock_guard<mutex> LG1(m2);
    this->end = chrono::high_resolution_clock::now();
    ofstream log;
    log.open(this->fileLocation);
    log << "Current Time: " << calTimePassed(this->start, this->end).count() << " us" << endl;
    log << "Iteration: " << iteration << endl;
    log << "Part Worker ID: " << this->id << endl;
    log << statusVector[status] << endl;
    log << "Accumulated Wait Time: " << this->accumulatedWaitTime.count() << " us" << endl;
    log << "Buffer State: (";
    printStates(log, bufferStates);
    log << "Pickup Order: (";
    printStates(log, pickUpOrder);
    log << "Local State: (";
    printStates(log, localState);
    log << "Cart State: (";
    printStates(log, cartState);
    log.close();
}

void productWork::printAfterState(const vector<int> &bufferStates) {
    lock_guard<mutex> LG1(m2);
    ofstream log;
    log.open(this->fileLocation);
    log << "Updated Buffer State: (";
    printStates(log, bufferStates);
    log << "Updated Pickup Order: (";
    printStates(log, pickUpOrder);
    log << "Updated Local State: (";
    printStates(log, localState);
    log << "Updated Cart State: (";
    printStates(log, cartState);
    log.close();
}


void productWork::printTotalFinished() {
    lock_guard<mutex> LG1(m2);
    ofstream log;
    log.open(this->fileLocation);
    log << "Total Completed Products: " << totalCompleteProduct << endl << endl;
    log.close();
}

void productWork::printFinished() {
    lock_guard<mutex> LG1(m2);
    ofstream log;
    log.open(this->fileLocation);
    this->end = chrono::high_resolution_clock::now();
    log << "Current Time: " << calTimePassed(this->start, this->end).count() << " us" << endl;
    log << "Updated Local State: (";
    printStates(log, localState);
    log << "Updated Cart State: (";
    printStates(log, cartState);
    log.close();
    printTotalFinished();
}

bool productWork::productThreadWait(unique_lock<mutex> &lck, condition_variable &cv,vector<int> &bufferStates) {
    while (!canLoadToCart(bufferStates))
        if ( cv.wait_for(lck,waitTime) == cv_status::timeout)
            return canLoadToCart(bufferStates);
    return true;
}

