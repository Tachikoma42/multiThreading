#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <complex>
#include <thread>
#include <cmath>
using namespace std;

int bitReverse(int x, int bits);
void localFFT(int totalInput, int stage, int innerStart, int innerEnd, vector<complex<double>>& tempResult, const vector<complex<double>>& butterflyInput);
int main() {
    string inputLocation = "data.txt";
    string outputLocation = "output_data1024.txt";
    vector<double> inputData;
    // read in data

    ifstream inFile;
    inFile.open(inputLocation);
    if (inFile.is_open()) {
        double temp;
        while (!inFile.eof())
        {
            inFile >> temp;
            inputData.emplace_back(temp);
        }
    }
    else {
        cout << "Unable to open data file" << endl;
        return 1;
    }

    inFile.close();
    // finish read in data
    int totalInput = inputData.size();
    cout << totalInput << endl;

    // check if input is not power of 2
    /*if (totalInput%2!=0) {
        cout << "Input is not a power of 2" << endl;
        if (inputData[totalInput - 1] == inputData[totalInput-2]) {
            inputData.pop_back();
            totalInput = inputData.size();
        }
        if ((totalInput & (totalInput - 1)) != 0) {
            cout << "Something wrong, please check input" << endl;
            return 1;
        }
    }*/
    int bits = (int)log2(totalInput);

    vector<complex<double>> butterflyInput;
    for (int i = 0; i < totalInput; ++i) {
        butterflyInput.emplace_back(inputData[bitReverse(i,bits)],0);
    }
    auto tempResult = butterflyInput;
    for (int i = 1; i <= bits; ++i) {
        thread T1{ localFFT,totalInput,i, 0, totalInput/4,ref(tempResult),ref(butterflyInput) };
        thread T2{ localFFT,totalInput,i,totalInput/4, totalInput/2, ref(tempResult),ref(butterflyInput)};
        thread T3{ localFFT,totalInput,i,totalInput/2, 3*totalInput/4,ref(tempResult),ref(butterflyInput) };
        localFFT(totalInput,i,3*totalInput/4,totalInput,tempResult, butterflyInput);
        T1.join();
        T2.join();
        T3.join();
        butterflyInput = tempResult;
    }
    ofstream outFile;
    outFile.open(outputLocation);
    if (outFile.is_open()) {
        for (const auto &result: butterflyInput) {
            outFile << result << "\n";
        }
    }
    else {
        cout << "Unable to write to file" << "\n";
        return 1;
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

void localFFT(int totalInput,int stage, int innerStart, int innerEnd, vector<complex<double>> &tempResult, const vector<complex<double>>& butterflyInput) {
    double pi = 3.14159;
    const complex<double> J(0,1);
    complex<double> W= exp(-J*(pi/(totalInput>>1)) );

    for (int j = innerStart; j < innerEnd; ++j) {
        if((j/((int)pow(2,stage-1)) )%2 == 0){
            tempResult[j] = butterflyInput[j]+butterflyInput[j+(int)pow(2,stage-1)];
        }else{
            tempResult[j] = -1.0 * butterflyInput[j]+butterflyInput[j-(int)pow(2,stage-1)];
        }
        if((j/(1<<stage))%2!=0){
            int powerOfW = j%(1<<stage)*totalInput/((int)pow(2,stage+1));
            //cout << "stage: " << stage << "power: " << powerOfW << endl;
            //tempResult[j]*=time * totalInput/((int)pow(2,i+1));
            tempResult[j]*=pow(W, powerOfW);
            //tempResult[j] *=  pow(W,time);
        }
    }
}
