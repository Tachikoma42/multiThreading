int buildPartTime(const vector<int> &oldPickupOrder, const vector<int> &newPickupOrder);

int assemblePartTime(const vector<int> &PickupOrder);

int movePartTime(const vector<int> &oldPickupOrder, const vector<int> &newPickupOrder);

int movePartTime(const vector<int> &newPickupOrder);

/// buffer
bool canLoadToBuffer(const vector<int> &loadOrder, const vector<int> &bufferStates);

bool canLoadToCart(const vector<int> &pickUpOrder, const vector<int> &bufferStates, const vector<int> &cartState);

bool isEmptyOrder(const vector<int> &loadOrder);

void loadToBuffer(vector<int> &loadOrder, vector<int> &bufferStates);

vector<int> loadToCart(const vector<int> &pickUpOrder, vector<int> &bufferStates, vector<int> &cartState);


#include "plant.h"

void partWork::partOrderGenerator(const vector<int> &oldLoadOrder, vector<int> &newLoadOrder) {
    int currentHave = std::accumulate(oldLoadOrder.begin(), oldLoadOrder.end(), 0);
    newLoadOrder.assign(oldLoadOrder.begin(), oldLoadOrder.end());
    for (int i = 0; i < maxPartSize - currentHave; ++i) {
        newLoadOrder[random(0, 4)]++;
    }
}


void productWork::assemblyOrderGenerator(const vector<int> &oldPickupOrder, vector<int> &newPickupOrder) {
    int curHave = std::accumulate(oldPickupOrder.begin(), oldPickupOrder.end(), 0);
    int typeCount = 0;
    if (curHave != 0) {
        for (const auto &part: oldPickupOrder) {
            if (part != 0) {
                typeCount++;
            }
        }
    }
    /// generate new order
    vector<int> tmpOrder(maxPartSize);
    while (true) {
        tmpOrder.clear();
        tmpOrder.assign(oldPickupOrder.begin(), oldPickupOrder.end());
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
    newPickupOrder.clear();
    newPickupOrder.assign(tmpOrder.begin(), tmpOrder.end());
}

void PartWorker(int id) {
    vector<int> oldOrder{0, 0, 0, 0, 0};
    auto waitTime = chrono::milliseconds(MaxTimePart);
    for (int i = 0; i < iteration; ++i) {
        vector<int> loadOrder(maxPartSize);
        partOrderGenerator(oldOrder, loadOrder);

        /// sleep for generate part time
        this_thread::sleep_for(chrono::milliseconds((buildPartTime(oldOrder, loadOrder))));
        /// sleep for move part time
        this_thread::sleep_for(chrono::milliseconds((movePartTime(loadOrder))));
        // TODO: print output

        unique_lock<mutex> lck(m1);
        cv1.wait(lck, canLoadToBuffer(loadOrder, bufferState));
        loadToBuffer(loadOrder, bufferState);
        cv2.notify_all();// TODO: notify producer
        while (!isEmptyOrder(loadOrder)) {
            auto before = chrono::high_resolution_clock::now();
            //bool result = cv1.wait_until(lck, before+waitTime, canLoadToBuffer(loadOrder, bufferSize));
            bool result = cv1.wait_for(lck, waitTime, canLoadToBuffer(loadOrder, bufferSize));
            auto after = chrono::high_resolution_clock::now();
            waitTime -= chrono::duration_cast<chrono::milliseconds>(after - before);
            if (result) {
                loadToBuffer(loadOrder, bufferState);
                cv2.notify_all();// TODO: notify producer
            } else {
                //timeout
                break;
            }
        }
        if (isEmptyOrder(loadOrder)) {
            /// order is empty
            oldOrder = {0, 0, 0, 0, 0};
        } else {
            this_thread::sleep_for(chrono::milliseconds((movePartTime(loadOrder))));
            oldOrder = loadOrder;
        }
    }

}

void ProductWorker(int id) {
    int waitTime = MaxTimeProduct;
    vector<int> localState = {0, 0, 0, 0, 0};
    for (int i = 0; i < iteration; ++i) {
        vector<int> pickupOrder(maxPartSize);
        assemblyOrderGenerator(localState, pickupOrder);
        vector<int> cartState = localState;

        unique_lock<mutex> lck(m1);
        cv2.wait(lck, canLoadToCart(pickupOrder, bufferState, cartState));

        this_thread::sleep_for(chrono::milliseconds((movePartTime(loadToCart(pickupOrder, bufferState, cartState)))));
        cv1.notify_all();// TODO: notify builder
        while (cartState != pickupOrder) {
            if (cv2.wait_for(lck, chrono::milliseconds(waitTime), canLoadToCart(pickupOrder, bufferState, cartState))) {
                this_thread::sleep_for(
                        chrono::milliseconds((movePartTime(loadToCart(pickupOrder, bufferState, cartState)))));
                cv1.notify_all();// TODO: notify builder
            } else {
                /// timeout
                break;
            }
        }
        if (cartState == pickupOrder) {
            this_thread::sleep_for(chrono::milliseconds(assemblePartTime(pickupOrder)));
            totalCompleteProduct++; // TODO: may need protection
            localState = {0, 0, 0, 0, 0};
        } else {
            localState = cartState;
        }
    }
}

int partWork::buildPartTime(const vector<int> &oldPickupOrder, const vector<int> &newPickupOrder) {
    int totalTime = 0;
    for (int i = 0; i < maxPartSize; ++i) {
        totalTime += produceTime[i] * (newPickupOrder[i] - oldPickupOrder[i]);
    }
    return totalTime;
}

int partWork::movePartTime(const vector<int> &oldPickupOrder, const vector<int> &newPickupOrder) {
    int totalTime = 0;
    for (int i = 0; i < maxPartSize; ++i) {
        totalTime += moveTime[i] * (newPickupOrder[i] - oldPickupOrder[i]);
    }
    return totalTime;
}

int partWork::movePartTime(const vector<int> &newPickupOrder) {
    return movePartTime({0, 0, 0, 0, 0}, newPickupOrder);
}

bool partWork::canLoadToBuffer(const vector<int> &loadOrder, const vector<int> &bufferStates) {
    for (int i = 0; i < maxPartSize; ++i) {
        if (bufferStates[i] < bufferSize[i] && loadOrder[i] != 0) {
            return true;
        } else {
            continue;
        }
    }
    return false;
}

void partWork::loadToBuffer(vector<int> &loadOrder, vector<int> &bufferStates) {
    for (int i = 0; i < maxPartSize; ++i) {
        if (bufferStates[i] < bufferSize[i] && loadOrder[i] != 0) {
            int itemToLoad = min(loadOrder[i], bufferSize[i] - bufferStates[i]);
            loadOrder[i] -= itemToLoad;
            bufferStates[i] += itemToLoad;
        }
    }
}

bool partWork::isEmptyOrder(const vector<int> &loadOrder) {
    return std::all_of(loadOrder.begin(), loadOrder.end(), [](int item) { return item != 0; });
    // TODO: not sure
//    for (const auto &item: loadOrder) {
//        if(item!=0){
//            return false;
//        }else{
//            continue;
//        }
//    }
//    return true;
}

bool canLoadToCart(const vector<int> &pickUpOrder, const vector<int> &bufferStates, const vector<int> &cartState) {
    for (int i = 0; i < maxPartSize; ++i) {
        if (cartState[i] < pickUpOrder[i] && bufferStates[i] != 0) {
            return true;
        } else {
            continue;
        }
    }
    return false;
}





vector<int> loadToCart(const vector<int> &pickUpOrder, vector<int> &bufferStates, vector<int> &cartState) {
    vector<int> tempOrder(maxPartSize);
    for (int i = 0; i < maxPartSize; ++i) {
        if (pickUpOrder[i] != cartState[i] && bufferStates[i] != 0) {
            tempOrder[i] = min(pickUpOrder[i] - cartState[i], bufferStates[i]);
            bufferStates[i] -= tempOrder[i];
        }
    }
    for (int i = 0; i < maxPartSize; ++i) {
        cartState[i] += tempOrder[i];
    }
    return tempOrder;
}

int assemblePartTime(const vector<int> &PickupOrder) {
    int totalTime = 0;
    for (int i = 0; i < maxPartSize; ++i) {
        totalTime += assemblyTime[i] * PickupOrder[i];
    }
    return totalTime;
}

