#pragma once

#include <iostream>
#include <thread>
#include <random>
#include <vector>
#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <numeric>
#include <fstream>

using namespace std;

// thread parameters
static mutex m1, m2, m3;
static condition_variable cv1, cv2;

typedef chrono::duration<long long int, ratio<1, 1000000>> durationTimeVariable;
typedef chrono::time_point<chrono::steady_clock> timePointVariable;

// plant parameters
static const int MaxTimePart{18000}, MaxTimeProduct{20000};
static vector<int> bufferState(5);
static int iteration = 5;
static int maxPartSize = 5;
static int maxTypeCount = 3;
static int totalCompleteProduct = 0;
static vector<int> bufferSize{5, 5, 4, 3, 3};
static vector<int> produceTime{500, 500, 600, 600, 700};
static vector<int> assemblyTime{600, 600, 700, 700, 800};
static vector<int> moveTime{200, 200, 300, 300, 400};
static string logLocation = "log.txt";
static vector<string> statusVector{"New Load Order", "Wakeup-Notified", "Wakeup-Timeout", "New Pickup Order",};
static timePointVariable globalStart;

/// part&product function

void PartWorker(int id);

void ProductWorker(int id);

/// support class & function

template<typename T>
T random(T range_from, T range_to) {
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_int_distribution<T> distr(range_from, range_to);
    return distr(generator);
}

class partWork {
private:
    int id;
    string fileLocation;
    int waitTimeLength;
    int partSize;
    vector<int> oldOrder;
    durationTimeVariable accumulatedWaitTime;
    timePointVariable start, end;

public:
    vector<int> loadOrder;
    durationTimeVariable waitTime;

    partWork(int workerId, int waitLength, int partNumber, string logLocation);

    void freshStart();

    /// function about order
    void partOrderGenerator();

    void transferOrder();

    bool isEmptyOrder();

    /// function about time
    void updateWaitTime(durationTimeVariable waitedTime);

    int buildPartTime();

    int movePartTime();

    /// function about load to buffer
    bool canLoadToBuffer(vector<int> &bufferStates);

    void loadToBuffer(vector<int> &bufferStates);

    /// function about output
    void printBeforeStatus(int iteration, int status, const vector<int> &bufferStates);

    void printAfterState(const vector<int> &bufferStates);


    /// function about thread wait
    bool partThreadWait(unique_lock<mutex>&lck, condition_variable &cv, vector<int> &bufferStates);
};

class productWork {
private:
    int id;
    string fileLocation;
    int waitTimeLength;
    int partSize;
    vector<int> localState;
    vector<int> pickUpOrder;
    vector<int> cartState;
    vector<int> tmpLoad;
    durationTimeVariable accumulatedWaitTime;
    timePointVariable start, end;
public:
    durationTimeVariable waitTime;

    productWork(int workerId, int waitLength, int partNumber, string logLocation);

    void freshStart();

    /// function about order
    void assemblyOrderGenerator();

    bool finishedLoad();

    void buildNewParts(int &totalCompleteProductCount);

    void waitForParts();

    /// function about time
    void updateWaitTime(durationTimeVariable waitedTime);

    int movePartTime();

    int assemblePartTime();

    /// function about get from buffer
    bool canLoadToCart(vector<int> &bufferStates);

    void loadToCart(vector<int> &bufferStates);

    /// function about output
    void printBeforeStatus(int iteration, int status, const vector<int> &bufferStates);

    void printAfterState(const vector<int> &bufferStates);
    void printTotalFinished();
    void printFinished();

    /// function about thread wait
    bool productThreadWait(unique_lock<mutex>&lck, condition_variable &cv,vector<int> &bufferStates);

};

durationTimeVariable calTimePassed(timePointVariable start, timePointVariable end);
void printStates(ostream & log, const vector<int>& orderStates);
//bool canLoadToBuffer();
