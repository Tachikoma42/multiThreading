
#include "bufferState.h"

bool bufferState::canLoadToBuffer(vector<int> &loadOrder) {
    for (int i = 0; i < maxPartSize; ++i) {
        if (buffer[i] < maxBufferState[i] && loadOrder[i] != 0) {
            return true;
        }
        else {
            continue;
        }
    }
    return false;
}

bool bufferState::canLoadToCart(vector<int>& pickUpOrder) {
    for (int i = 0; i < maxPartSize; ++i) {
        if (pickUpOrder[i] > 0 && buffer[i] > 0) {
            return true;
        }
        else {
            continue;
        }
    }
    return false;
}



void bufferState::loadToBuffer(vector<int> &loadOrder) {
    for (int i = 0; i < maxPartSize; ++i) {
        if (buffer[i] < maxBufferState[i] && loadOrder[i] != 0) {
            int itemToLoad = min(loadOrder[i], maxBufferState[i] - buffer[i]);
            loadOrder[i] -= itemToLoad;
            buffer[i] += itemToLoad;
        }
    }
}





bool bufferState::partThreadWait(unique_lock<mutex> &lck, condition_variable &cv, timePointVariable &before, timePointVariable &after,durationTimeVariable waitTime) {
    before = chrono::high_resolution_clock::now();
    while (!canLoadToBuffer(buffer)) {
        if (cv.wait_until(lck, before + waitTime) == cv_status::timeout) {
            after = chrono::high_resolution_clock::now();
            return canLoadToBuffer(buffer);
        }
    }
    after = chrono::high_resolution_clock::now();
    return true;
}

bool bufferState::threadLoadToBuffer(partWork &worker, int iteration) {
    unique_lock<mutex> lck(m1);
    bool result = partThreadWait(lck, cv3, worker.before, worker.after, worker.waitTime);

    worker.updateWaitTime(calTimePassed(worker.before, worker.after));
    if (result) {
        worker.printBeforeStatus(iteration + 1, worker.newOrder ? 0 : 1, buffer);
        loadToBuffer(buffer);
        worker.printAfterState(buffer);
        cv4.notify_all();
        worker.newOrder = false;
        return true;
    }
    else {
        return false;
    }
}
bool bufferState::productThreadWait(unique_lock<mutex> &lck, condition_variable &cv,durationTimeVariable &waitTime) {
    while (!canLoadToCart(buffer))
        if (cv.wait_for(lck, waitTime) == cv_status::timeout)
            return canLoadToCart(buffer);
    return true;
}

void bufferState::loadToCart(vector<int> &cartState, vector<int> &pickUpOrder) {
    vector<int> tempLoad = { 0,0,0,0,0 };
    for (int i = 0; i < maxPartSize; ++i) {
        if (pickUpOrder[i] > 0 && buffer[i] > 0) {
            tempLoad[i] = min(pickUpOrder[i], buffer[i]);
            buffer[i] -= tempLoad[i];
            cartState[i] += tempLoad[i];
            pickUpOrder[i] -= tempLoad[i];
        }
    }
}

bool bufferState::threadLoadToCart(productWork &worker,productWorkerPrinter &printer, bool &newOrder) {
    unique_lock<mutex> lck(m1);
    auto before = chrono::high_resolution_clock::now();
    printer.bufferTimer = before;
    bool result = productThreadWait(lck, cv4, worker.waitTime);
    auto after = chrono::high_resolution_clock::now();
    worker.updateWaitTime(calTimePassed(before, after));
    printer.accWaitTime = worker.accumulatedWaitTime;


    printer.beforeBuffer = buffer;
    printer.beforePickUp = worker.pickUpOrder;
    printer.beforeLocal = worker.localState;
    printer.beforeCart = worker.cartState;

    if (result) {

        printer.Status = newOrder ? 3 : 1;
        newOrder = false;


        loadToCart(worker.cartState, worker.pickUpOrder);

        printer.updateBuffer = buffer;
        printer.updatePickUp = worker.pickUpOrder;
        printer.updateLocal = worker.localState;
        printer.updateCart = worker.cartState;
        cv3.notify_one();// TODO: notify builder
        if (!worker.finishedLoad()) {
            printer.totalCount = totalCompleteProduct;
            printer.printMode = 0;
            worker.structPrinter(printer);
            printer = {};
        }
        return true;
    } else {
        /// timeout
        printer.Status = 2;
        printer.updateBuffer = buffer;
        printer.updatePickUp = worker.pickUpOrder;
        printer.updateLocal = worker.localState;
        printer.updateCart = worker.cartState;
        printer.printMode = 1;
        return false;
    }
}

const vector<int> &bufferState::getBuffer() const {
    return buffer;
}
