#include <iostream>
#include <vector>
#include <thread>
#include <fstream>
#include <chrono>

using namespace std;
using namespace chrono;

class Timer {
public:
   chrono::system_clock::time_point Begin, End;
   system_clock::duration Runtime;
   Timer() {//constructor
       Begin = system_clock::now();
   }
   ~Timer() {
       End = system_clock::now();
       Runtime = End - Begin;
       cout << "Runtime = " << chrono::duration_cast<chrono::milliseconds>(Runtime).count() << endl;
   }
};
// I removed all the int end in the function since they are not needed.
void bitonicSort(vector<int>& A, int size, int start, int dir);
void compareTwo(vector<int>& A, int n1, int n2, bool dir);// 0->1 true 1->0 false
void bitonicSorter(vector<int>& A, int size, int start, int dir);
void bitonicMerger(vector<int>& A, int size, int start, int dir);

int main() {
   int n{ 4194304 }; // n should be bigger than 2 to parallel.

   vector<int>A;
   for (int i = 0; i < n; ++i) {
       A.emplace_back(rand() % 10000);
   }

   {
       Timer tm1;
       bitonicSort(A, n, 0, true); // non threaded bitonicSort
   }

   ofstream outFile;
   outFile.open("output1.txt");
   if (outFile.is_open()) {
       for (const auto& result : A) {
           outFile << result << "\n";
       }
   }
   else {
       cout << "Unable to write to file" << "\n";
       return 1;
   }
   return 0;
}

void compareTwo(vector<int>& A, int n1, int n2, bool dir) {
   //0->1 true 1->0 false
   //this function is same as exam, but can be integered into Merger
   if ((A[n1] > A[n2]) == dir) {
       swap(A[n1], A[n2]);
   }
}

void bitonicSort(vector<int>& A, int size, int start, int dir) {
   bitonicSorter(A, size, start, dir);
}

void bitonicMerger(vector<int>& A, int size, int start, int dir) {
   if (size == 1) {
       return;
       //end condition
   }
   for (int i = 0; i < size / 2; ++i) {
       compareTwo(A, start + i, start + size / 2 + i, dir);
   }
   bitonicMerger(A, size / 2, start, dir);
   bitonicMerger(A, size / 2, start + size / 2, dir);
}

void bitonicSorter(vector<int>& A, int size, int start, int dir) {
   if (size == 1) {
       return;
       //end condition
   }
   // same as the exam, but the first two merger have same functionality as the last merger
   // can be commented out for better performance.
   bitonicSorter(A, size / 2, start, true);
   //bitonicMerger(A, size / 2, start, true);//1

   bitonicSorter(A, size / 2, start + size / 2, false);
   //bitonicMerger(A, size / 2, start + size / 2, false);//2

   bitonicMerger(A, size, start, dir);//3

}
