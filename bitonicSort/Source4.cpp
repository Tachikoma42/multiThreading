#include <iostream>
#include <vector>
#include <thread>
#include <fstream>
#include <chrono>

using namespace std;
using namespace chrono;

class Timer {
public:
	system_clock::time_point start;
	Timer() {
		start = system_clock::now();
	}

	~Timer() {
		cout << "runtime: " << duration_cast<milliseconds>(system_clock::now() - start).count() << " ms" << endl;
	}
};

// I removed all the int end in the function since they are not needed.
//void bitonicSort(vector<int> &A, int size, int start, int dir);
//void compareTwo(vector<int>&A, int n1, int n2, bool dir);// 0->1 true 1->0 false
void bitonicSorter(vector<int>& A, int size, int start, int dir);
void bitonicMerger(vector<int>& A, int size, int start, int dir);

int main() {
	int n{ 16777216 }; // n should be bigger than 2 to parallel.

	vector<int>A;
	for (int i = 0; i < n; ++i) {
		A.emplace_back(rand() % 10000);
	}
	{
		Timer tm2;
		//divided into four thread means end two steps earlier.
		thread t1{ bitonicSorter, ref(A), n / 4, 0, true };
		thread t2{ bitonicSorter, ref(A), n / 4, 0 + n / 4, false };
		thread t3{ bitonicSorter, ref(A), n / 4,0 + n / 4 * 2,true };
		bitonicSorter(A, n / 4, 0 + n / 4 * 3, false);
		t1.join();
		t2.join();
		t3.join();

		// two bitonicMerger merge into the last step
		thread t4{ bitonicMerger, ref(A), n / 2, 0, true }; // use thread to speed up
		//bitonicMerger(A, n / 2, 0, true);
		bitonicMerger(A, n / 2, 0 + n / 2, false);
		t4.join();
		//merge into the final result.
		bitonicMerger(A, n, 0, true);
	}


	ofstream outFile;
	outFile.open("output4.txt");
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

//void compareTwo(vector<int> &A, int n1, int n2, bool dir) {
//    //0->1 true 1->0 false
//    //this function is same as exam, but can be integered into Merger
//    if ((A[n1] > A[n2] )== dir){
//        swap(A[n1],A[n2]);
//    }
//}

//void bitonicSort(vector<int> &A, int size, int start, int dir) {
//    bitonicSorter(A,size,start, dir);
//}

void bitonicMerger(vector<int>& A, int size, int start, int dir) {
	if (size == 1) {
		return;
		//end condition
	}
	for (int i = 0; i < size / 2; ++i) {
		if ((A[start + i] > A[start + size / 2 + i]) == dir) {
			swap(A[start + i], A[start + size / 2 + i]);
		}
		//compareTwo(A, start+i, start+size/2+i, dir);
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
	//bitonicMerger(A,size/2, start, true);//1

	bitonicSorter(A, size / 2, start + size / 2, false);
	//bitonicMerger(A,size/2, start+size/2, false);//2

	bitonicMerger(A, size, start, dir);//3

}
