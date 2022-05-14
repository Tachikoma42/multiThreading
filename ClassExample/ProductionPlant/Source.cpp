#include "plant.h"

int main() {
//    const int m = 20, n = 16;
//    vector<thread> PartW, ProductW;
//
//    for (int i = 0; i < m; ++i) {
//        PartW.emplace_back(PartWorker, i + 1);
//    }
//    for (int i = 0; i < n; ++i) {
//        ProductW.emplace_back(ProductWorker, i + 1);
//    }
//
//    for (auto& i : PartW) i.join();
//    for (auto& i : ProductW) i.join();
//    cout << "Finish!" << endl;
    partOrderGenerator(bufferState);
    return 0;
}