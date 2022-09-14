#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;
int main(){
	vector<int> loadOrder ={0,0,0,0,0};
cout <<  std::all_of(loadOrder.begin(), loadOrder.end(), [](int item) { return item == 0; });
}
