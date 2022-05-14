//
// Created by Sihao Ren on 3/31/2022.
//

#include "plant.h"

void partOrderGenerator(vector<int> &oldOrder) {
    int currentHave = std::accumulate(oldOrder.begin(), oldOrder.end(), 0);
    for (int i = 0; i < maxAnyType-currentHave; ++i) {
        oldOrder[random(0,4)]++;
    }
}

void productOrderGenerator(vector<int> &oldOrder, bool hasLeftOver) {

}
