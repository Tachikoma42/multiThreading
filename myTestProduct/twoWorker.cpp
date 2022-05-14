#include "plant.h"


void PartWorker(int id) {
    partWork worker(id, MaxTimePart, maxPartSize, logLocation);

    for (int i = 0; i < iteration; ++i) {
        worker.freshStart();/// start timer
        //worker.partOrderGenerator();
        /// sleep for generate part time
        this_thread::sleep_for(chrono::microseconds((worker.buildPartTime())));
        /// sleep for move part time
        this_thread::sleep_for(chrono::microseconds((worker.movePartTime())));


        bool result = true;
        while (!worker.isEmptyOrder() && result) {
            result = globalBuffer.threadLoadToBuffer(worker,i);
        }
        if (worker.isEmptyOrder()) {
            worker.transferOrder();
        } else {
            this_thread::sleep_for(chrono::microseconds((worker.movePartTime())));
            /// wakeup due to timeout // TODO: print output
            worker.printBeforeStatus(i + 1, 2, globalBuffer.getBuffer());
            worker.printAfterState(globalBuffer.getBuffer());
            worker.transferOrder();
        }

    }

}

void ProductWorker(int id) {
    productWork worker(id, MaxTimePart, maxPartSize, logLocationP);
    for (int i = 0; i < iteration; ++i) {

        worker.freshStart();
        bool newOrder = true;
        bool result = true;
        productWorkerPrinter printer;

        while (!worker.finishedLoad()&& result) {
            printer.iteration = i+1;
            result = globalBuffer.threadLoadToCart(worker, printer,newOrder);
        }
        if (worker.finishedLoad()) {
            this_thread::sleep_for(chrono::microseconds(worker.movePartTime()));
            this_thread::sleep_for(chrono::microseconds(worker.assemblePartTime()));
            printer.returnedTimer = chrono::high_resolution_clock::now();
            worker.buildNewParts(printer);

            printer.endCart = worker.cartState;
            printer.endLocal = worker.localState;
            // it is possible two product worker increase the total number of product
            // that will result in a jump in number when print to log.txt
            printer.totalCount = totalCompleteProduct;

            printer.printMode = 1;
            worker.structPrinter(printer);
            printer = {};

            // whenever complete product print
        } else {
            /// timeout
            this_thread::sleep_for(chrono::microseconds(worker.movePartTime()));
            printer.returnedTimer = chrono::high_resolution_clock::now();
            worker.waitForParts();

            printer.endCart = worker.cartState;
            printer.endLocal = worker.localState;
            printer.totalCount = totalCompleteProduct;
            printer.printMode = 1;
            worker.structPrinter(printer);
            printer = {};
        }

    }
    //cout << totalCompleteProduct << endl;
}

durationTimeVariable calTimePassed(timePointVariable start, timePointVariable end) {
    return chrono::duration_cast<chrono::microseconds>(end - start);
}


void printStates(ostream &log, const vector<int> &orderStates) {

    //for (const auto& item : orderStates) {
    //    log << item << ",";
    //}
    for (int i = 0; i < 5; i++) {
        log << orderStates[i];
        if (i != 4) {
            log << ",";
        }
    }
    log << ")" << endl;
}

void debugPrintBuffer(int id, int type) {
    lock_guard<mutex> LG1(m4);
    cout << "total Complete Product " << totalCompleteProduct << endl;
    if (type == 1) {
        cout << "ProductWorker " << id;
    } else {
        cout << "PartWorker " << id;
    }
    cout << " Buffer State" << ": ";
    for (auto &item: globalBuffer.getBuffer()) {
        cout << item << " ";
    }
    cout << endl;
}

int random(int range_from, int range_to) {
    // srand(system_clock::now().time_since_epoch().count());
    // return rand() % 5; // this method will generate duplicate order

    // random number generator from cppreference.com
    // https://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distrib(range_from, range_to);
    return distrib(gen);
}