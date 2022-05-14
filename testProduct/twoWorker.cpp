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

        //unique_lock<mutex> lck(m1);
        //while (!worker.canLoadToBuffer(bufferState))
        //{
        //    cv1.wait(lck);
        //}    

        ////cv1.wait(lck, canLoadToBuffer);
        //worker.printBeforeStatus(i + 1, 0, bufferState);
        //worker.loadToBuffer(bufferState);

        //worker.printAfterState(bufferState);
        ///// new load order // TODO: print output
        //cv2.notify_all();// TODO: notify producer
        bool newOrder = true;
        while (!worker.isEmptyOrder()) {
            unique_lock<mutex> lck(m1);
            /*auto before = chrono::high_resolution_clock::now();*/
            bool result = worker.partThreadWait(lck, cv1, bufferState);
           /* auto after = chrono::high_resolution_clock::now();*/
 /*           if (!result)
            {
                lock_guard<mutex> LG1(m4);
                cout << endl;
                cout << "accu Time:" << calTimePassed(before, after).count() << endl;
                cout << "wait:" << worker.waitTime.count() << endl;
                cout << endl;
                
            }*/
            worker.updateWaitTime(calTimePassed(worker.before, worker.after));
            if (result) {
                ///wakeup do something // TODO: print output
                worker.printBeforeStatus(i + 1, newOrder?0:1, bufferState);
                newOrder = false;
                worker.loadToBuffer(bufferState);
                worker.printAfterState(bufferState);
                cv2.notify_one();// TODO: notify producer
            }
            else {
                //timeout
                ///wakeup due to timeout
                //worker.debugPrintLoadOrder();
                
                break;
            }
        }
        if (worker.isEmptyOrder()) {
            /// order is empty
           
            worker.transferOrder();
        }
        else {
            this_thread::sleep_for(chrono::microseconds((worker.movePartTime())));
            /// wakeup due to timeout // TODO: print output
            worker.printBeforeStatus(i + 1, 2, bufferState);
            worker.printAfterState(bufferState);
            worker.transferOrder();
        }

    }

}

void ProductWorker(int id) {
    productWork worker(id, MaxTimePart, maxPartSize, logLocationP);
    for (int i = 0; i < iteration; ++i) {
        
        worker.freshStart();
        //debugPrintBuffer(id, 1);
       // unique_lock<mutex> lck(m1);
       // while (!worker.canLoadToCart(bufferState)) {
       //     cv2.wait(lck);
       // }

       // //cv2.wait(lck, worker.canLoadToCart(bufferState));
       // //worker.debugPrintPickUpOrder();

       //// worker.printBeforeStatus(i + 1, 3, bufferState);
       // worker.loadToCart(bufferState);
       // //worker.printAfterState(bufferState);
       // //worker.printTotalFinished();
       // this_thread::sleep_for(chrono::microseconds((worker.movePartTime())));
       // cv1.notify_all();// TODO: notify builder
        //debugPrintBuffer(id,1);
        //cout << totalCompleteProduct << endl;
        bool newOrder = true;
        while (!worker.finishedLoad()) {
            unique_lock<mutex> lck(m1);
            auto before = chrono::high_resolution_clock::now();
            bool result = worker.productThreadWait(lck, cv2, bufferState);
            auto after = chrono::high_resolution_clock::now();
            worker.updateWaitTime(calTimePassed(before, after));

            if (result) {
                worker.printBeforeStatus(i + 1, newOrder?3:1, bufferState);
                newOrder = false;
                worker.loadToCart(bufferState);

                worker.printAfterState(bufferState);
                worker.printTotalFinished(totalCompleteProduct);
                cv1.notify_one();// TODO: notify builder
                //this_thread::sleep_for(chrono::microseconds(worker.movePartTime()));
           
            }
            else {
                /// timeout
                break;
            }
        }
        if (worker.finishedLoad()) {
            this_thread::sleep_for(chrono::microseconds(worker.movePartTime()));
            this_thread::sleep_for(chrono::microseconds(worker.assemblePartTime()));

            worker.buildNewParts(&totalCompleteProduct);
            worker.printAfterState(bufferState);
            //worker.printTotalFinished();
            worker.printFinished(totalCompleteProduct);
            // whenever complete product print
        }
        else {
            /// timeout
            worker.printBeforeStatus(i + 1, 2, bufferState);
            worker.printAfterState(bufferState);
            this_thread::sleep_for(chrono::microseconds(worker.movePartTime()));
            worker.waitForParts();
            worker.printFinished(totalCompleteProduct);
        }
        cout << totalCompleteProduct << endl;
    }
}

void printStates(ostream& log, const vector<int>& orderStates) {

    //for (const auto& item : orderStates) {
    //    log << item << ",";
    //}
    for (int i = 0; i < 5; i++)
    {
        log << orderStates[i];
        if (i!=4)
        {
            log << ",";
        }
    }
    log << ")" << endl;
}

void debugPrintBuffer(int id,int type)
{
    lock_guard<mutex> LG1(m4);
    cout << "total Complete Product " << totalCompleteProduct << endl;
    if (type == 1) {
        cout << "ProductWorker " << id;
    }
    else {
        cout << "PartWorker " << id;
    }
    cout << " Buffer State" << ": ";
    for (auto& item : bufferState)
    {
        cout << item << " ";
    }
    cout << endl;
}
