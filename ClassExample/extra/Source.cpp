#include<iostream>
#include<thread>
#include<chrono>
using namespace std;
using namespace chrono;

void f1() {
	cout << this_thread::get_id() << endl;
}
int main() {
	cout << thread::hardware_concurrency() << endl;

	thread T1(f1);
	//T1(f1);
	T1.join();
	//cout << T1.hardware_concurrency() << endl;
	return 0;
}