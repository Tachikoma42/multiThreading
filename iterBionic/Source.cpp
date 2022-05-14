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
void impBitonicSort(vector<int> &A, int N);
int main() {
    int n{ 1048576 }; // n should be bigger than 2 to parallel.

    vector<int>A;
    for (int i = 0; i < n; ++i) {
        A.emplace_back(rand()% 10000);
    }
    {
        Timer tm2;
        impBitonicSort(A, A.size());
    }

    ofstream outFile;
    outFile.open("output4.txt");
    if (outFile.is_open()) {
        for (const auto &result: A) {
            outFile << result << "\n";
        }
    }
    else {
        cout << "Unable to write to file" << "\n";
        return 1;
    }
    return 0;
}


void impBitonicSort(vector<int> &A, int N) {

    int i, j, k;

    for (k = 2; k <= N; k = 2 * k) {
        for (j = k >> 1; j > 0; j = j >> 1) {
            for (i = 0; i < N; i++) {
                int ij = i ^ j;
                if ((ij) > i) {
                    if ((i & k) == 0 && A[i] > A[ij])
                        swap(A[i], A[ij]);
                    if ((i & k) != 0 && A[i] < A[ij])
                        swap(A[i], A[ij]);
                }
            }
        }
    }
}