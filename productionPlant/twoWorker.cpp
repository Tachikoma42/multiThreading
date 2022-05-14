#include "plant.h"

durationTimeVariable calTimePassed(timePointVariable start, timePointVariable end) {
    return chrono::duration_cast<chrono::microseconds>(end - start);
}

void PartWorker(int id) {
    partWork worker(id, MaxTimePart, maxPartSize, logLocation);

    for (int i = 0; i < iteration; ++i) {
        worker.freshStart();/// start timer
        //worker.partOrderGenerator();

        /// sleep for generate part time
        this_thread::sleep_for(chrono::microseconds((worker.buildPartTime())));
        /// sleep for move part time
        this_thread::sleep_for(chrono::microseconds((worker.movePartTime())));

        unique_lock<mutex> lck(m1);
        while (!worker.canLoadToBuffer(bufferState))
        {
            cv1.wait(lck);
        }
        //cv1.wait(lck, canLoadToBuffer);

        worker.printBeforeStatus(i + 1, 0, bufferState);
        worker.loadToBuffer(bufferState);
        worker.printAfterState(bufferState);
        /// new load order // TODO: print output
        cv2.notify_all();// TODO: notify producer
        while (!worker.isEmptyOrder()) {
            auto before = chrono::high_resolution_clock::now();
            bool result = worker.partThreadWait(lck,cv1,bufferState);
            auto after = chrono::high_resolution_clock::now();
            worker.updateWaitTime(calTimePassed(before, after));
            if (result) {
                ///wakeup do something // TODO: print output
                worker.printBeforeStatus(i + 1, 1, bufferState);
                worker.loadToBuffer(bufferState);
                worker.printAfterState(bufferState);
                cv2.notify_all();// TODO: notify producer
            } else {
                //timeout
                ///wakeup due to timeout
                break;
            }
        }
        if (worker.isEmptyOrder()) {
            /// order is empty
            worker.transferOrder();
        } else {
            this_thread::sleep_for(chrono::microseconds((worker.movePartTime())));
            /// wakeup due to timeout // TODO: print output
            worker.printBeforeStatus(i + 1, 1, bufferState);
            worker.printAfterState(bufferState);
            worker.transferOrder();
        }
    }

}

void ProductWorker(int id) {
    productWork worker(id, MaxTimePart, maxPartSize, logLocation);
    for (int i = 0; i < iteration; ++i) {
        worker.freshStart();

        unique_lock<mutex> lck(m1);
        while(!worker.canLoadToCart(bufferState)){
            cv2.wait(lck);
        }
        //cv2.wait(lck, worker.canLoadToCart(bufferState));

        worker.printBeforeStatus(i + 1, 3, bufferState);
        worker.loadToCart(bufferState);
        worker.printAfterState(bufferState);
        worker.printTotalFinished();
        this_thread::sleep_for(chrono::microseconds((worker.movePartTime())));
        cv1.notify_all();// TODO: notify builder
        while (!worker.finishedLoad()) {
            auto before = chrono::high_resolution_clock::now();
            bool result = worker.productThreadWait(lck, cv2,bufferState);
//            bool result = cv2.wait_for(lck, worker.waitTime, worker.canLoadToCart(bufferState));
            auto after = chrono::high_resolution_clock::now();
            worker.updateWaitTime(calTimePassed(before, after));
            if (result) {
                worker.printBeforeStatus(i + 1, 1, bufferState);
                worker.loadToCart(bufferState);
                worker.printAfterState(bufferState);
                worker.printTotalFinished();

                this_thread::sleep_for(
                        chrono::microseconds(worker.movePartTime()));
                cv1.notify_all();// TODO: notify builder
            } else {
                /// timeout
                break;
            }
        }
        if (worker.finishedLoad()) {
            this_thread::sleep_for(chrono::microseconds(worker.assemblePartTime()));
            worker.buildNewParts(totalCompleteProduct);
            worker.printFinished();
            // whenever complete product print
        } else {
            worker.waitForParts();

        }
    }
}

void printStates(ostream & log, const vector<int> &orderStates) {

    for (const auto &item: orderStates) {
        log << item << ",";
    }
    log << ")" << endl;
}
