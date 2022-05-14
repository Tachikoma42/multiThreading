#include "plant.h"
#include "bufferState.h"

int main() {
    const int m = 20, n = 16;   //m: number of Part Workers
    //n: number of Product Workers 20 16
    //Different numbers might be used during grading.

    // clean out the log file
    ofstream log;
    log.open(logLocation);
    log.close();
    /*log.open(logLocationP);
    log.close();*/
    vector<thread> PartW, ProductW;
    globalStart = chrono::high_resolution_clock::now();
    for (int i = 0; i < m; ++i) {
        PartW.emplace_back(PartWorker, i + 1);
    }
    for (int i = 0; i < n; ++i) {
        ProductW.emplace_back(ProductWorker, i + 1);
    }
    for (auto& i : PartW) i.join();
    for (auto& i : ProductW) i.join();
    cout << "Finish!" << endl;
    //cout << totalCompleteProduct << endl;
    return 0;
}
