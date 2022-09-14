#include<iostream>
#include<atomic>
#include<future>
#include<chrono>
#include <thread>
#include<vector>
using namespace std;
using namespace literals::chrono_literals;


atomic_flag Flag{ATOMIC_FLAG_INIT};


class Timer{
    private:
        std::chrono::system_clock::time_point start,end;
    public:
    Timer(){
        start = std::chrono::system_clock::now();
    }
    ~Timer(){
        end = std::chrono::system_clock::now();
        std::chrono::system_clock::duration Runtime = end-start;
        cout << "Time has passed: " << std::chrono::duration_cast<std::chrono::milliseconds>(Runtime).count() << endl;
    }
};
struct test{
    int i1;
    int i2;
   // int i3;

};
bool Is_Prime(int n) {
	for (int i = 2; i < n; ++i) {
		if (n % i == 0) return false;
	}
	return true;
}

class myClass {
public:

	bool Is_Prime(int n)  const {
		for (int i = 2; i < n; ++i) {
			if (n % i == 0) return false;
		}
		return true;
	}

} f1;

void IsPrime(future<int>& F, bool& B) {
	int a = F.get();
	for (int i = 2; i < a; ++i) {
		if (a % i == 0) { B = false;  return; }
	}
	B = true;
	return;

}

int main(){

	promise<int> Prom2;
	future<int> Fut6 = Prom2.get_future();
    
	bool B{ false };

	thread T1{ IsPrime, ref(Fut6), ref(B) };
	//....
	Prom2.set_value(4348398);

	cout << "B " << B << endl;

	T1.join();

}