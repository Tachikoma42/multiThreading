#pragma once

#include <iostream>
#include <thread>
#include <random>
#include <vector>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <numeric>
#include <fstream>
#include <atomic>
#include "bufferState.h"

using namespace std;

mutex m1, m2, m3, m4; // m4 is for debug
condition_variable cv3,cv4;

typedef chrono::duration<long long int, ratio<1, 1000000>> durationTimeVariable;
typedef chrono::time_point<chrono::steady_clock> timePointVariable;

// plant parameters
static const int MaxTimePart{ 18000 }, MaxTimeProduct{ 20000 };
//static vector<int> bufferState(5);
static int iteration = 5;
static int maxPartSize = 5;
static int maxTypeCount = 3;
static atomic<int> totalCompleteProduct = 0;
static vector<int> maxBufferState{ 5, 5, 4, 3, 3 };
static vector<int> produceTime{ 500, 500, 600, 600, 700 };
static vector<int> assemblyTime{ 600, 600, 700, 700, 800 };
static vector<int> moveTime{ 200, 200, 300, 300, 400 };
static string logLocation = "log.txt";
static string logLocationP = "log.txt";
static vector<string> statusVector{ "New Load Order", "Wakeup-Notified", "Wakeup-Timeout", "New Pickup Order" };
static timePointVariable globalStart;

/// part&product function
static class bufferState globalBuffer;
void PartWorker(int id);

void ProductWorker(int id);

/// support class & function


struct productWorkerPrinter {
    timePointVariable bufferTimer, returnedTimer;
    int Status;
    int iteration;
    durationTimeVariable accWaitTime;
    vector<int> beforeBuffer, beforePickUp, beforeLocal, beforeCart;
    vector<int> updateBuffer, updatePickUp, updateLocal, updateCart;
    int totalCount;
    int printMode; //0->wake-up notify, 1
    vector<int> endLocal, endCart;

};
class partWork {
private:
    int id;
    string fileLocation;
    int waitTimeLength;
    int partSize;
    vector<int> oldOrder;
    durationTimeVariable accumulatedWaitTime;
    timePointVariable start, end;
    int debugUpdateCount;
    vector<int> debugUpdateTime;

public:

    bool newOrder;
    timePointVariable before, after;

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
    ///this function is replaced by the global buffer
//    bool canLoadToBuffer(vector<int>& bufferStates);
    ///this function is replaced by the global buffer

//    void loadToBuffer(vector<int>& bufferStates);

    /// function about output
    void printBeforeStatus(int iteration, int status, const vector<int>& bufferState);

    void printAfterState(const vector<int>& bufferState);


    /// function about thread wait
    ///this function is replaced by the global buffer
    //bool partThreadWait(unique_lock<mutex>& lck, condition_variable& cv, vector<int>& bufferStates);

    /// function about debug
    void debugPrintLoadOrder();
};

class productWork {
private:
    int id;
    string fileLocation;
    int waitTimeLength;
    int partSize;

public:
    vector<int> localState;
    vector<int> pickUpOrder;
    vector<int> cartState;
    vector<int> tmpLoad;
    timePointVariable start, end;

    durationTimeVariable waitTime;
    durationTimeVariable accumulatedWaitTime;

    productWork(int workerId, int waitLength, int partNumber, string logLocation);

    void freshStart();

    /// function about order
    void assemblyOrderGenerator();

    bool finishedLoad();

    void buildNewParts(productWorkerPrinter& tobePrinted);

    void waitForParts();

    /// function about time
    void updateWaitTime(durationTimeVariable waitedTime);

    int movePartTime();

    int assemblePartTime();

    /// function about get from buffer
    /// this function is replaced by the global buffer
//    bool canLoadToCart(vector<int>& bufferStates);
    /// this function is replaced by the global buffer
//    void loadToCart(vector<int>& bufferStates);

    /// function about output
    void printBeforeStatus(int iteration, int status, const vector<int>& bufferState);
    void printAfterState(const vector<int>& bufferState);
    void printTotalFinished(const int CompleteProductCount);
    void printFinished(const int CompleteProductCount);
    void structPrinter(const productWorkerPrinter& tobePrinted);


    /// function about thread wait
    /// this function is replaced by the global buffer
    //bool productThreadWait(unique_lock<mutex>& lck, condition_variable& cv, vector<int>& bufferStates);

    /// function about debug
    void debugPrintPickUpOrder();

};

durationTimeVariable calTimePassed(timePointVariable start, timePointVariable end);
void printStates(ostream& log, const vector<int>& orderStates);

void debugPrintBuffer(int id, int type);

int random(int range_from, int range_to);



