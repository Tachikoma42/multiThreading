//
// Created by Sihao Ren on 3/31/2022.
//

#ifndef PLANT_H
#define PLANT_H

#include <iostream>
#include <thread>
#include <random>
#include <vector>
#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <numeric>

using namespace std;

// thread parameters
mutex m1, m2;
condition_variable cv1, cv2;
atomic<int> c_wait {0}, p_wait {0};

// plant parameters
const int MaxTimePart{ 1800 }, MaxTimeProduct{ 2000 };
vector<int> bufferState(5);
int maxAnyType = 5;
enum bufferSize{maxA = 5, maxB = 5, maxC = 4, maxD = 3, maxE = 3};




/// part&product function

void PartWorker(int id);
void ProductWorker(int id);

/// support class & function

template<typename T>
T random(T range_from, T range_to) {
    std::random_device                  rand_dev;
    std::mt19937                        generator(rand_dev());
    std::uniform_int_distribution<T>    distr(range_from, range_to);
    return distr(generator);
}

void partOrderGenerator(vector<int>&oldOrder);
void productOrderGenerator(vector<int>&oldOrder, bool hasLeftOver);


/// reference function
//void Consumer(int id) {
//    unique_lock<mutex> UG1(m1);//for accessing V
//    ++c_wait;
//    while (V.size() < 6) cv1.wait(UG1);//a producer needs all 6 elements to process.
//    --c_wait;
//    //do something
//    this_thread::sleep_for(chrono::milliseconds(5));
//    V.clear();
//    {
//        lock_guard<mutex> LG1(m2);
//        cout << "Consumer " << id << " V.size() = " << V.size() << " c_wait = " << c_wait << " p_wait = " << p_wait << endl;
//    }
//    cv2.notify_all();//OK
//}
//void Producer(int id) {
//    unique_lock<mutex> UG1(m1);
//    ++p_wait;
//    while (V.size() == 6) cv2.wait(UG1);//the maximum capacity of V is 6
//    --p_wait;
//    //do something
//    this_thread::sleep_for(chrono::milliseconds(5));
//    V.push_back(id);
//    {
//        lock_guard<mutex> LG1(m2);
//        cout << "Producer " << id << " V.size() = "<< V.size() << " c_wait = " << c_wait << " p_wait = " << p_wait<<endl;
//    }
//    cv1.notify_all();
//    //if (V.size() == 6) cv1.notify_one();
//    //if (V.size() < 6) cv2.notify_one();
//}


#endif //PLANT_H
