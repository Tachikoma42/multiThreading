#include<iostream>
#include<thread>
#include<vector>
#include<chrono>

using namespace std;
using namespace chrono;
class Timer {
public:
	chrono::system_clock::time_point Begin, End;
	system_clock::duration Runtime;
	Timer() {
		Begin = system_clock::now();
	}
	~Timer() {
		End = system_clock::now();
		Runtime = End - Begin;
		cout << "Runtime = " << chrono::duration_cast<chrono::microseconds>(Runtime).count() << endl;
	}
};

void walk(int w1, int w2, int n, vector<float>& V) {
	for (int i = w1; i <= w2; ++i)
	{
        int sum = 0;
		for (int j = 0; j < n; j++)
		{
			int x = 0, y = 0;
			for (int k = 0; k < i; k++)
			{
				int p = rand() % 4;
				switch (p)
				{
				case 0: ++x; break;
				case 1: ++y; break;
				case 2: --x; break;
				case 3: --y; break;
				}
			}
            sum += (abs(x) + abs(y));
		}
        V[i] = static_cast<float> (sum) / n;
	}
}
int main() {
	vector<float> V(30);

	int n = 10000;
//    walk(10, 29, n, V);
    {
        Timer t;
        thread t1(walk, 10,14,n,ref(V));
        thread t2(walk, 15,19,n,ref(V));
        thread t3(walk, 20,24,n,ref(V));
        walk( 25,29,n,V);
        t1.join();
        t2.join();
        t3.join();
    }


    for (int i = 10; i < 30; i++)
        cout << V[i] << endl;
}