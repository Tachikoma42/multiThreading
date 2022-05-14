#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <complex>
#include <thread>
#include <cmath>
using namespace std;

int bitReverse(int x, int bits);

int main() {

    string dataLocation = "C:\\Users\\xkzmx\\Documents\\Odyssey\\School\\Thread\\FFT\\data.txt";
    string dataSaveLocation = "C:\\Users\\xkzmx\\Documents\\Odyssey\\School\\Thread\\FFT\\output_data.txt";
    ifstream inFile;
    inFile.open(dataLocation);
    vector<double> inputData;

    if (inFile.is_open()) {
        try {
            // use try catch to prevent any read in error.
            double temp;
            while (!inFile.eof()) {
                inFile >> temp;
                inputData.emplace_back(temp);
            }
        }
        catch (...) {
            cout << "Read data error" << endl;
            return -1;
        }
    } else {
        cout << "Unable to open file" << endl;
        return -1;
    }
    inFile.close();

    int totalInput = inputData.size();
    int bits = (int)log2(totalInput);
//    while (tempNum!=1){
//        bits++;
//        tempNum/=2;
//    }
    vector<double> butterflyInput;
    for (int i = 0; i < totalInput; ++i) {
        butterflyInput.emplace_back(inputData[bitReverse(i,bits)]);
    }
    ofstream outFile(dataSaveLocation);
    if (outFile.is_open()) {
        int breakNum = 0;
        for (const auto &datum: inputData) {
            breakNum++;
            outFile << datum << " ";
            if (breakNum % 10 == 0) {
                outFile << endl;
            }
        }
        outFile << endl;
    } else {
        cout << "Unable to open file";
        return -1;
    }
    return 0;
}
int bitReverse(int x, int bits) {
    // should have use lookup table
    // x is the number to be reversed, bits is the number of bits. bits should be 6 since
    // 64 = 2^6;
    int ans = 0;
    for (int i = 0; i < bits; ++i) {
        // get last bit of x
        ans = ans << 1; // shift ans left to leave out last bit to be assigned
        ans = ans | (x & 1); // assign last bit to ans
        x = x >> 1; // lose the last bit
    }
    return ans;
}