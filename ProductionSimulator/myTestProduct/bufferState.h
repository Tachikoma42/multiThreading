#pragma once
#include "plant.h"


class bufferState {
private:
    mutex m5;
    vector<int> buffer;
    condition_variable cv3, cv4;
public:
    void loadToBuffer(vector<int>& loadOrder);
    void loadToCart(vector<int>& cartState, vector<int>& pickUpOrder);
    bool canLoadToBuffer(vector<int>& loadOrder);
    bool canLoadToCart(vector<int>& pickUpOrder);
    bool threadLoadToCart(productWork &worker, productWorkerPrinter& printer, bool &newOrder);
    bool threadLoadToBuffer(partWork &worker, int iteration);
    bool productThreadWait(unique_lock<mutex>& lck, condition_variable& cv, durationTimeVariable &waitTime);
    bool partThreadWait(unique_lock<mutex> &lck, condition_variable &cv, timePointVariable &before, timePointVariable &after,durationTimeVariable waitTime);

    const vector<int> &getBuffer() const;
};
