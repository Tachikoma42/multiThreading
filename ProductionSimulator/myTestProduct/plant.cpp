#include "plant.h"




partWork::partWork(int workerId, int waitLength, int partNumber, string logLocation) {
    this->id = workerId;
    this->fileLocation = logLocation;
    this->partSize = partNumber;
    this->oldOrder = { 0, 0, 0, 0, 0 };
    this->loadOrder = { 0, 0, 0, 0, 0 };
    this->accumulatedWaitTime = chrono::microseconds(0);
    this->waitTimeLength = waitLength;
    this->waitTime = chrono::microseconds(waitTimeLength);
    this->newOrder = true;
}

void partWork::freshStart() {
    this->loadOrder.clear();
    this->loadOrder = { 0, 0, 0, 0, 0 };
    this->accumulatedWaitTime = chrono::microseconds(0);
    this->start = globalStart;
    this->waitTime = chrono::microseconds(waitTimeLength);
    partOrderGenerator();
    //debug
    newOrder = true;
    debugUpdateCount = 0;
    debugUpdateTime.clear();
}

void partWork::partOrderGenerator() {
    int currentHave = std::accumulate(this->oldOrder.begin(), this->oldOrder.end(), 0);
    loadOrder.clear();
    loadOrder = oldOrder;
    //this->loadOrder.assign(this->oldOrder.begin(), this->oldOrder.end());
    for (int i = 0; i < this->partSize - currentHave; ++i) {
        this->loadOrder[random(0, 4)]++;
    }
}

int partWork::buildPartTime() {
    int totalTime = 0;
    for (int i = 0; i < partSize; ++i) {
        totalTime += produceTime[i] * (loadOrder[i] - oldOrder[i]);
    }
    return totalTime;
}

int partWork::movePartTime() {
    int totalTime = 0;
    for (int i = 0; i < partSize; ++i) {
        totalTime += moveTime[i] * loadOrder[i];
    }
    return totalTime;
}

//bool partWork::canLoadToBuffer(vector<int>& bufferStates) {
//    for (int i = 0; i < partSize; ++i) {
//        if (bufferStates[i] < maxBufferState[i] && loadOrder[i] != 0) {
//            return true;
//        }
//        else {
//            continue;
//        }
//    }
//    return false;
//}
//
//
//void partWork::loadToBuffer(vector<int>& bufferStates) {
//
//    for (int i = 0; i < partSize; ++i) {
//        if (bufferStates[i] < maxBufferState[i] && loadOrder[i] != 0) {
//            int itemToLoad = min(loadOrder[i], maxBufferState[i] - bufferStates[i]);
//            loadOrder[i] -= itemToLoad;
//            bufferStates[i] += itemToLoad;
//        }
//    }
//
//}

bool partWork::isEmptyOrder() {
    return std::all_of(this->loadOrder.begin(), this->loadOrder.end(), [](int item) { return item == 0; });
}



void partWork::transferOrder() {
    oldOrder.clear();
    oldOrder = loadOrder;
    /*for (int i = 0; i < partSize; ++i) {
        oldOrder[i] = loadOrder[i];
    }*/
}


void partWork::updateWaitTime(durationTimeVariable waitedTime) {
    //if (waitedTime.count()<0||waitTime.count()<0)
    //{
    //    cout << "Wrong" << endl;
    //}
    this->accumulatedWaitTime += waitedTime;
    //debugUpdateTime.push_back(waitedTime.count());
    this->waitTime -= waitedTime;
    //debugUpdateCount++;
    if (waitTime.count() < 0)
    {
        waitTime = chrono::microseconds(0);
    }
}

void partWork::printBeforeStatus(int iteration, int status, const vector<int>& bufferStates) {
    /// this function need protection
    lock_guard<mutex> LG1(m2);
    this->end = chrono::high_resolution_clock::now();
    ofstream log;
    log.open(fileLocation, std::ios_base::app);
    log << "Current Time: " << calTimePassed(this->start, this->end).count() << " us" << endl;
    log << "Iteration: " << iteration << endl;
    log << "Part Worker ID: " << this->id << endl;
    log << "Status: " << statusVector[status] << endl;
    log << "Accumulated Wait Time: " << this->accumulatedWaitTime.count() << " us" << endl;
    log << "Buffer State: (" << bufferStates[0] << "," << bufferStates[1] << ",";
    log << bufferStates[2] << "," << bufferStates[3] << "," << bufferStates[4] << ")" << endl;
    log << "Load Order: (" << this->loadOrder[0] << "," << this->loadOrder[1] << ",";
    log << this->loadOrder[2] << "," << this->loadOrder[3] << "," << this->loadOrder[4] << ")" << endl;

    // debug
    /*log << "Accumulated counter: " << debugUpdateCount << endl;
    for (auto& item : debugUpdateTime) {
        log << item << endl;
    }*/
    log.close();
}

void partWork::printAfterState(const vector<int>& bufferStates) {
    lock_guard<mutex> LG1(m2);
    ofstream log;
    log.open(fileLocation, std::ios_base::app);
    log << "Updated Buffer State: (" << bufferStates[0] << "," << bufferStates[1] << ",";
    log << bufferStates[2] << "," << bufferStates[3] << "," << bufferStates[4] << ")" << endl;
    log << "Updated Load Order: (" << loadOrder[0] << "," << loadOrder[1] << ",";
    log << loadOrder[2] << "," << loadOrder[3] << "," << loadOrder[4] << ")" << endl;
    log << endl;
    log.close();
}

//bool partWork::partThreadWait(unique_lock<mutex>& lck, condition_variable& cv, vector<int>& bufferStates) {
//	before = chrono::high_resolution_clock::now();
//    while (!canLoadToBuffer(bufferStates)) {
//        if (cv.wait_for(lck, waitTime) == cv_status::timeout)
//			after = chrono::high_resolution_clock::now();
//            return canLoadToBuffer(bufferStates);
//    }
//    after = chrono::high_resolution_clock::now();
//    return true;
//}

//bool partWork::partThreadWait(unique_lock<mutex>& lck, condition_variable& cv, vector<int>& bufferStates) {
//    before = chrono::high_resolution_clock::now();
//    while (!canLoadToBuffer(bufferStates)) {
//        if (cv.wait_until(lck, before + waitTime) == cv_status::timeout) {
//            after = chrono::high_resolution_clock::now();
//            return canLoadToBuffer(bufferStates);
//        }
//    }
//    after = chrono::high_resolution_clock::now();
//    return true;
//}

void partWork::debugPrintLoadOrder()
{
    lock_guard<mutex> LG1(m4);
    cout << "PartWorker " << isEmptyOrder();
    cout << " Load Order" << ": ";
    for (auto& item : loadOrder)
    {
        cout << item << " ";
    }
    cout << endl;
}


productWork::productWork(int workerId, int waitLength, int partNumber, string logLocation) {
    this->id = workerId;
    this->fileLocation = logLocation;
    this->partSize = partNumber;
    this->localState = { 0, 0, 0, 0, 0 };
    this->pickUpOrder = { 0, 0, 0, 0, 0 };
    this->cartState = { 0, 0, 0, 0, 0 };
    this->tmpLoad = { 0, 0, 0, 0, 0 };
    this->accumulatedWaitTime = chrono::microseconds(0);
    this->waitTimeLength = waitLength;
    this->waitTime = chrono::microseconds(waitTimeLength);
}

void productWork::freshStart() {
    this->accumulatedWaitTime = chrono::microseconds(0);
    this->start = globalStart;
    this->waitTime = chrono::microseconds(waitTimeLength);
    this->cartState.clear();
    this->cartState = { 0,0,0,0,0 }; //TODO: need check if works.
    this->pickUpOrder.clear();
    this->pickUpOrder = { 0, 0, 0, 0, 0 };
    this->tmpLoad = { 0, 0, 0, 0, 0 };
    assemblyOrderGenerator();
}

void productWork::assemblyOrderGenerator() {
    int curHave = std::accumulate(localState.begin(), localState.end(), 0);
    int typeCount = 0;
    if (curHave != 0) {
        for (const auto& part : localState) {
            if (part != 0) {
                typeCount++;
            }
        }
    }
    /// generate new order
    vector<int> tmpOrder;
    while (true) {
        tmpOrder.clear();
        tmpOrder = localState;

        int tmpTypeCount = typeCount;
        for (int i = 0; i < partSize - curHave;) {
            int newPart = random(0, 4);
            if (tmpOrder[newPart] == 0 && tmpTypeCount < maxTypeCount) {
                tmpTypeCount++;
                tmpOrder[newPart]++;
                ++i;
            }
            else if (tmpOrder[newPart] != 0) {
                tmpOrder[newPart]++;
                ++i;
            }
            else {
                continue;
            }
        }
        if (tmpTypeCount != 1) {
            break;
        }
    }
    this->pickUpOrder.clear();
    pickUpOrder = { 0,0,0,0,0 };
    for (int i = 0; i < partSize; ++i) {
        pickUpOrder[i] = tmpOrder[i] - localState[i];
    }
    //this->pickUpOrder.assign(tmpOrder.begin(), tmpOrder.end());
}



//bool productWork::canLoadToCart(vector<int>& bufferStates) {
//    for (int i = 0; i < partSize; ++i) {
//        if (pickUpOrder[i] > 0 && bufferStates[i] > 0) {
//            return true;
//        }
//        else {
//            continue;
//        }
//    }
//    return false;
//
//}

bool productWork::finishedLoad() {
    //    return (cartState == pickUpOrder);
    //return pickUpOrder == localState;
    return std::all_of(this->pickUpOrder.begin(), this->pickUpOrder.end(), [](int item) { return item == 0; });

}

//void productWork::loadToCart(vector<int>& bufferStates) {
//    tmpLoad = { 0,0,0,0,0 };
//    for (int i = 0; i < partSize; ++i) {
//        if (pickUpOrder[i] > 0 && bufferStates[i] > 0) {
//            tmpLoad[i] = min(pickUpOrder[i], bufferStates[i]);
//            bufferStates[i] -= tmpLoad[i];
//            cartState[i] += tmpLoad[i];
//            pickUpOrder[i] -= tmpLoad[i];
//        }
//    }
//    //for (int i = 0; i < partSize; ++i) {
//    //    cartState[i] += tmpLoad[i];
//    //    pickUpOrder[i] -= tmpLoad[i];
//    //}
//}

void productWork::updateWaitTime(durationTimeVariable waitedTime) {
    this->accumulatedWaitTime += waitedTime;
    this->waitTime -= waitedTime;
}

int productWork::movePartTime() {
    int totalTime = 0;
    for (int i = 0; i < partSize; ++i) {
        totalTime += moveTime[i] * cartState[i];
    }
    tmpLoad.clear(); // TODO: empty tmpLoad
    return totalTime;
}

int productWork::assemblePartTime() {
    int totalTime = 0;
    for (int i = 0; i < partSize; ++i) {
        totalTime += assemblyTime[i] * (cartState[i] + localState[i]);
    }

    return totalTime;
}

void productWork::buildNewParts(productWorkerPrinter& tobePrinted) {
    lock_guard<mutex> LG1(m3);

    ++totalCompleteProduct; // TODO: may need protection
    //cout << totalCompleteProduct << endl;
    cartState = { 0,0,0,0,0 };
    localState = { 0,0,0,0,0 };
    //printFinished(*completeProductCount);
}

void productWork::waitForParts() {
    for (int i = 0; i < partSize; ++i) {
        localState[i] += cartState[i];
    }
    cartState = { 0,0,0,0,0 };
    //localState = cartState; // TODO: may not work
}

void productWork::printBeforeStatus(int iteration, int status, const vector<int>& bufferStates) {
    /// this function need protection
    lock_guard<mutex> LG1(m2);
    this->end = chrono::high_resolution_clock::now();
    ofstream log;
    log.open(this->fileLocation, std::ios_base::app);
    log << "Current Time: " << calTimePassed(this->start, this->end).count() << " us" << endl;
    log << "Iteration: " << iteration << endl;
    log << "Part Worker ID: " << this->id << endl;
    log << "Status: " << statusVector[status] << endl;
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

void productWork::printAfterState(const vector<int>& bufferStates) {
    lock_guard<mutex> LG1(m2);
    ofstream log;
    log.open(this->fileLocation, std::ios_base::app);
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


void productWork::printTotalFinished(const int CompleteProductCount) {
    lock_guard<mutex> LG1(m2);
    ofstream log;
    log.open(this->fileLocation, std::ios_base::app);
    log << "Total Completed Products: " << CompleteProductCount << endl << endl;
    log.close();
}

void productWork::printFinished(const int CompleteProductCount) {
    lock_guard<mutex> LG1(m2);
    ofstream log;
    log.open(this->fileLocation, std::ios_base::app);
    this->end = chrono::high_resolution_clock::now();
    log << "Current Time: " << calTimePassed(this->start, this->end).count() << " us" << endl;
    log << "Updated Local State: (";
    printStates(log, localState);
    log << "Updated Cart State: (";
    printStates(log, cartState);
    log << "Total Completed Products: " << CompleteProductCount << endl << endl;
    log.close();
    //printTotalFinished();
}

void productWork::structPrinter(const productWorkerPrinter& tobePrinted)
{
    lock_guard<mutex> LG1(m2);
    ofstream log;
    log.open(this->fileLocation, std::ios_base::app);
    log << "Current Time: " << calTimePassed(start, tobePrinted.bufferTimer).count() << " us" << endl;
    log << "Iteration: " << tobePrinted.iteration << endl;
    log << "Product Worker ID: " << id << endl;
    log << "Status: " << statusVector[tobePrinted.Status] << endl;
    log << "Accumulated Wait Time: " << tobePrinted.accWaitTime.count() << " us" << endl;
    log << "Buffer State: (";
    printStates(log, tobePrinted.beforeBuffer);
    log << "Pickup Order: (";
    printStates(log, tobePrinted.beforePickUp);
    log << "Local State: (";
    printStates(log, tobePrinted.beforeLocal);
    log << "Cart State: (";
    printStates(log, tobePrinted.beforeCart);

    log << "Updated Buffer State: (";
    printStates(log, tobePrinted.updateBuffer);
    log << "Updated Pickup Order: (";
    printStates(log, tobePrinted.updatePickUp);
    log << "Updated Local State: (";
    printStates(log, tobePrinted.updateLocal);
    log << "Updated Cart State: (";
    printStates(log, tobePrinted.updateCart);
    if (tobePrinted.printMode == 1)
    {
        log << "Current Time: " << calTimePassed(start, tobePrinted.returnedTimer).count() << " us" << endl;
        log << "Updated Local State: (";
        printStates(log, tobePrinted.endLocal);
        log << "Updated Cart State: (";
        printStates(log, tobePrinted.endCart);
    }
    log << "Total Completed Products: " << tobePrinted.totalCount << endl << endl;
    log.close();
}



void productWork::debugPrintPickUpOrder()
{
    lock_guard<mutex> LG1(m4);
    cout << "ProductWorker " << id;
    cout << " PickUp Order" << ": ";
    for (auto& item : pickUpOrder)
    {
        cout << item << " ";
    }
    cout << endl;
}

//bool productWork::productThreadWait(unique_lock<mutex>& lck, condition_variable& cv, vector<int>& bufferStates) {
//    while (!canLoadToCart(bufferStates))
//        if (cv.wait_for(lck, waitTime) == cv_status::timeout)
//            return canLoadToCart(bufferStates);
//    return true;
//}

