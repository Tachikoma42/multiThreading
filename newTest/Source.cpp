
#include <iostream>
#include <vector>
#include <thread>
#include <fstream>
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

void sort(vector<int>& A, int l, int r, int flag);
void process(vector<int>& A, int l, int r, int N, int flag);
void process2(vector<int>& A, int l, int r, int flag);
void swap(vector<int>& A, int i, int j);

int main() {

	int n{ 16777216 };//2^17
	vector<int> A;
	for (int i = 0; i < n; i++) A.emplace_back(rand() % 100);

	{
		Timer t1;
		vector<thread> T;
		T.emplace_back(sort, ref(A), 0, n / 4 - 1, 1);
		T.emplace_back(sort, ref(A), n / 4, n / 2 - 1, 0);
		T.emplace_back(sort, ref(A), n / 2, 3 * n / 4 - 1, 1);
		sort(A, 3 * n / 4, n - 1, 0);
		for (int i = 0; i < 3; i++) T[i].join();
		T.clear();

		process2(A, 0, n / 2 - 1, 1);
		process2(A, n / 2, n - 1, 0);

		T.emplace_back(process, ref(A), 0, (n - 1) / 4, n / 4, 1);
		T.emplace_back(process, ref(A), n / 4, (n - 1) / 2, n / 4, 1);
		T.emplace_back(process, ref(A), n / 2, (n - 1) * 3 / 4, n / 4, 0);
		process(A, 3 * n / 4, n - 1, n / 4, 0);
		for (int i = 0; i < 3; i++) T[i].join();
		T.clear();
		
		process2(A, 0, n - 1, 1);
		process2(A, 0, n / 2 - 1, 1);
		process2(A, n / 2, n - 1, 1);

		T.emplace_back(process, ref(A), 0, (n - 1) / 4, n / 4, 1);
		T.emplace_back(process, ref(A), n / 4, (n - 1) / 2, n / 4, 1);
		T.emplace_back(process, ref(A), n / 2, (n - 1) * 3 / 4, n / 4, 1);
		process(A, 3 * n / 4, n - 1, n / 4, 1);
		for (int i = 0; i < 3; i++) T[i].join();
	}

	ofstream Out("output4.txt");
	for (auto& i : A) Out << i << endl;
	Out.close();

	return 0;
}

void sort(vector<int>& A, int l, int r, int flag) {
	if (l >= r) return;
	int m = l + (r - l) / 2;
	sort(A, l, m, 1);
	sort(A, m + 1, r, 0);
	int N = r - l + 1;

	for (int i = N / 2; i >= 1; i /= 2) {
		for (int j = l; j <= r; j += 2 * i) {
			for (int k = j; k < j + i; k++) {
				if (flag) {
					if (A[k] > A[k + i]) swap(A, k, k + i);
				}
				else {
					if (A[k] < A[k + i]) swap(A, k, k + i);
				}
			}
		}
	}
};

//perform comparison within threads
void process(vector<int>& A, int l, int r, int N, int flag) {
	for (int i = N / 2; i >= 1; i /= 2) {
		for (int j = l; j <= r; j += 2 * i) {
			for (int k = j; k < j + i; k++) {
				if (flag) {
					if (A[k] > A[k + i]) swap(A, k, k + i);
				}
				else {
					if (A[k] < A[k + i]) swap(A, k, k + i);
				}
			}
		}
	}
}

//perform comparison across threads
void process2(vector<int>& A, int l, int r, int flag) {
	int N = r - l + 1;
	for (int i = l; i < l + N / 2; i++) {
		if (flag) {
			if (A[i] > A[i + N / 2]) swap(A, i, i + N / 2);
		}
		else {
			if (A[i] < A[i + N / 2]) swap(A, i, i + N / 2);
		}
	}
}


void swap(vector<int>& A, int i, int j) {
	int tmp = A[i];
	A[i] = A[j];
	A[j] = tmp;
};


