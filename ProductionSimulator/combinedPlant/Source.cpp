#include <iostream>
#include <thread>
#include <random>
#include <vector>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <numeric>
#include <fstream>

using namespace std;

mutex m1, m2, m3, m4; // m4 is for debug
condition_variable cv1, cv2;

typedef chrono::duration<long long int, ratio<1, 1000000>> durationTimeVariable;
typedef chrono::time_point<chrono::steady_clock> timePointVariable;

// plant parameters
const int MaxTimePart{ 18000 }, MaxTimeProduct{ 20000 };
vector<int> bufferState(5);
int iteration = 5;
int maxPartSize = 5;
int maxTypeCount = 3;
int totalCompleteProduct = 0;
vector<int> maxBufferState{ 5, 5, 4, 3, 3 };
vector<int> produceTime{ 500, 500, 600, 600, 700 };
vector<int> assemblyTime{ 600, 600, 700, 700, 800 };
vector<int> moveTime{ 200, 200, 300, 300, 400 };
string logLocation = "log.txt";
string logLocationP = "log.txt";
vector<string> statusVector{ "New Load Order", "Wakeup-Notified", "Wakeup-Timeout", "New Pickup Order" };
timePointVariable globalStart;


/// part&product function

void PartWorker(int id);

void ProductWorker(int id);

/// support class & function


struct productWorkerPrinter {
	timePointVariable bufferTimer, returnedTimer;
	int Status;
	int iteration;
	durationTimeVariable accWaitTime;
	vector<int> beforeBuffer, beforePickUp, beforeLocal, beforeCart;
	vector<int> updateBuffer, updatePickUp, updateLocal, updateCart;
	int totalCount;
	int printMode; //0->wake-up notify, 1
	vector<int> endLocal, endCart;

};
class partWork {
private:
	int id;
	string fileLocation;
	int waitTimeLength;
	int partSize;
	vector<int> oldOrder;
	durationTimeVariable accumulatedWaitTime;
	timePointVariable start, end;
	int debugUpdateCount;
	vector<int> debugUpdateTime;

public:


	timePointVariable before, after;

	vector<int> loadOrder;
	durationTimeVariable waitTime;

	partWork(int workerId, int waitLength, int partNumber, string logLocation);

	void freshStart();

	/// function about order
	void partOrderGenerator();

	void transferOrder();

	bool isEmptyOrder();

	/// function about time
	void updateWaitTime(durationTimeVariable waitedTime);

	int buildPartTime();

	int movePartTime();

	/// function about load to buffer
	bool canLoadToBuffer(vector<int>& bufferStates);

	void loadToBuffer(vector<int>& bufferStates);

	/// function about output
	void printBeforeStatus(int iteration, int status, const vector<int>& bufferState);

	void printAfterState(const vector<int>& bufferState);


	/// function about thread wait
	bool partThreadWait(unique_lock<mutex>& lck, condition_variable& cv, vector<int>& bufferStates);

	/// function about debug
	void debugPrintLoadOrder();
};

class productWork {
private:
	int id;
	string fileLocation;
	int waitTimeLength;
	int partSize;

public:
	vector<int> localState;
	vector<int> pickUpOrder;
	vector<int> cartState;
	vector<int> tmpLoad;
	timePointVariable start, end;

	durationTimeVariable waitTime;
	durationTimeVariable accumulatedWaitTime;

	productWork(int workerId, int waitLength, int partNumber, string logLocation);

	void freshStart();

	/// function about order
	void assemblyOrderGenerator();

	bool finishedLoad();

	void buildNewParts(productWorkerPrinter& tobePrinted);

	void waitForParts();

	/// function about time
	void updateWaitTime(durationTimeVariable waitedTime);

	int movePartTime();

	int assemblePartTime();

	/// function about get from buffer
	bool canLoadToCart(vector<int>& bufferStates);

	void loadToCart(vector<int>& bufferStates);

	/// function about output
	void printBeforeStatus(int iteration, int status, const vector<int>& bufferState);
	void printAfterState(const vector<int>& bufferState);
	void printTotalFinished(const int CompleteProductCount);
	void printFinished(const int CompleteProductCount);
	void structPrinter(const productWorkerPrinter& tobePrinted);


	/// function about thread wait
	bool productThreadWait(unique_lock<mutex>& lck, condition_variable& cv, vector<int>& bufferStates);

	/// function about debug
	void debugPrintPickUpOrder();

};

durationTimeVariable calTimePassed(timePointVariable start, timePointVariable end);
void printStates(ostream& log, const vector<int>& orderStates);

void debugPrintBuffer(int id, int type);

int random(int range_from, int range_to); 

int main() {
	const int m = 20, n = 16;   //m: number of Part Workers
	//n: number of Product Workers 20 16
	//Different numbers might be used during grading.

	// clean out the log file
	ofstream log;
	log.open(logLocation);
	log.close();
	/*log.open(logLocationP);
	log.close();*/
	vector<thread> PartW, ProductW;
	globalStart = chrono::high_resolution_clock::now();
	for (int i = 0; i < m; ++i) {
		PartW.emplace_back(PartWorker, i + 1);
	}
	for (int i = 0; i < n; ++i) {
		ProductW.emplace_back(ProductWorker, i + 1);
	}
	for (auto& i : PartW) i.join();
	for (auto& i : ProductW) i.join();
	cout << "Finish!" << endl;
	//cout << totalCompleteProduct << endl;
	return 0;
}

durationTimeVariable calTimePassed(timePointVariable start, timePointVariable end) {
	return chrono::duration_cast<chrono::microseconds>(end - start);
}



void PartWorker(int id) {
	partWork worker(id, MaxTimePart, maxPartSize, logLocation);

	for (int i = 0; i < iteration; ++i) {
		worker.freshStart();/// start timer
		//worker.partOrderGenerator();
		/// sleep for generate part time
		this_thread::sleep_for(chrono::microseconds((worker.buildPartTime())));
		/// sleep for move part time
		this_thread::sleep_for(chrono::microseconds((worker.movePartTime())));

		//unique_lock<mutex> lck(m1);
		//while (!worker.canLoadToBuffer(bufferState))
		//{
		//    cv1.wait(lck);
		//}    

		////cv1.wait(lck, canLoadToBuffer);
		//worker.printBeforeStatus(i + 1, 0, bufferState);
		//worker.loadToBuffer(bufferState);

		//worker.printAfterState(bufferState);
		///// new load order // TODO: print output
		//cv2.notify_all();// TODO: notify producer
		bool newOrder = true;
		while (!worker.isEmptyOrder()) {
			unique_lock<mutex> lck(m1);
			/*auto before = chrono::high_resolution_clock::now();*/
			bool result = worker.partThreadWait(lck, cv1, bufferState);
			/* auto after = chrono::high_resolution_clock::now();*/
  /*           if (!result)
			 {
				 lock_guard<mutex> LG1(m4);
				 cout << endl;
				 cout << "accu Time:" << calTimePassed(before, after).count() << endl;
				 cout << "wait:" << worker.waitTime.count() << endl;
				 cout << endl;

			 }*/
			worker.updateWaitTime(calTimePassed(worker.before, worker.after));
			if (result) {
				///wakeup do something // TODO: print output
				worker.printBeforeStatus(i + 1, newOrder ? 0 : 1, bufferState);
				worker.loadToBuffer(bufferState);
				worker.printAfterState(bufferState);
				newOrder = false;
				cv2.notify_all();// TODO: notify producer
			}
			else {
				//timeout
				///wakeup due to timeout
				//worker.debugPrintLoadOrder();

				break;
			}
		}
		if (worker.isEmptyOrder()) {
			/// order is empty

			worker.transferOrder();
		}
		else {
			this_thread::sleep_for(chrono::microseconds((worker.movePartTime())));
			/// wakeup due to timeout // TODO: print output
			worker.printBeforeStatus(i + 1, 2, bufferState);
			worker.printAfterState(bufferState);
			worker.transferOrder();
		}

	}

}

void ProductWorker(int id) {
	productWork worker(id, MaxTimePart, maxPartSize, logLocationP);
	for (int i = 0; i < iteration; ++i) {

		worker.freshStart();
		//debugPrintBuffer(id, 1);
	   // unique_lock<mutex> lck(m1);
	   // while (!worker.canLoadToCart(bufferState)) {
	   //     cv2.wait(lck);
	   // }

	   // //cv2.wait(lck, worker.canLoadToCart(bufferState));
	   // //worker.debugPrintPickUpOrder();

	   //// worker.printBeforeStatus(i + 1, 3, bufferState);
	   // worker.loadToCart(bufferState);
	   // //worker.printAfterState(bufferState);
	   // //worker.printTotalFinished();
	   // this_thread::sleep_for(chrono::microseconds((worker.movePartTime())));
	   // cv1.notify_all();// TODO: notify builder
		//debugPrintBuffer(id,1);
		//cout << totalCompleteProduct << endl;
		bool newOrder = true;
		productWorkerPrinter printer;
		while (!worker.finishedLoad()) {
			unique_lock<mutex> lck(m1);
			auto before = chrono::high_resolution_clock::now();
			printer.bufferTimer = before;
			printer.iteration = i + 1;
			bool result = worker.productThreadWait(lck, cv2, bufferState);
			auto after = chrono::high_resolution_clock::now();
			worker.updateWaitTime(calTimePassed(before, after));
			printer.accWaitTime = worker.accumulatedWaitTime;


			printer.beforeBuffer = bufferState;
			printer.beforePickUp = worker.pickUpOrder;
			printer.beforeLocal = worker.localState;
			printer.beforeCart = worker.cartState;

			if (result) {

				printer.Status = newOrder ? 3 : 1;
				newOrder = false;


				worker.loadToCart(bufferState);

				printer.updateBuffer = bufferState;
				printer.updatePickUp = worker.pickUpOrder;
				printer.updateLocal = worker.localState;
				printer.updateCart = worker.cartState;
				/*worker.printAfterState(bufferState);
				worker.printTotalFinished(totalCompleteProduct);*/
				cv1.notify_all();// TODO: notify builder
				//this_thread::sleep_for(chrono::microseconds(worker.movePartTime()));
				if (!worker.finishedLoad())
				{
					printer.totalCount = totalCompleteProduct;
					printer.printMode = 0;
					worker.structPrinter(printer);
					printer = {};
				}

			}
			else {
				/// timeout
				printer.Status = 2;
				printer.updateBuffer = bufferState;
				printer.updatePickUp = worker.pickUpOrder;
				printer.updateLocal = worker.localState;
				printer.updateCart = worker.cartState;
				printer.printMode = 1;
				//printer.totalCount = totalCompleteProduct;
				break;
			}
		}
		if (worker.finishedLoad()) {
			this_thread::sleep_for(chrono::microseconds(worker.movePartTime()));
			this_thread::sleep_for(chrono::microseconds(worker.assemblePartTime()));
			printer.returnedTimer = chrono::high_resolution_clock::now();
			worker.buildNewParts(printer);

			printer.endCart = worker.cartState;
			printer.endLocal = worker.localState;
			// it is possible two product worker increase the total number of product taht will result in a jump in number when print to log.txt
			printer.totalCount = totalCompleteProduct;
			//cout << totalCompleteProduct << endl;

			printer.printMode = 1;
			worker.structPrinter(printer);
			printer = {};

			// whenever complete product print
		}
		else {
			/// timeout
			this_thread::sleep_for(chrono::microseconds(worker.movePartTime()));
			printer.returnedTimer = chrono::high_resolution_clock::now();
			worker.waitForParts();

			printer.endCart = worker.cartState;
			printer.endLocal = worker.localState;
			printer.totalCount = totalCompleteProduct;
			printer.printMode = 1;
			worker.structPrinter(printer);
			printer = {};
		}
		
	}
	//cout << totalCompleteProduct << endl;
}

void printStates(ostream& log, const vector<int>& orderStates) {

	//for (const auto& item : orderStates) {
	//    log << item << ",";
	//}
	for (int i = 0; i < 5; i++)
	{
		log << orderStates[i];
		if (i != 4)
		{
			log << ",";
		}
	}
	log << ")" << endl;
}

void debugPrintBuffer(int id, int type)
{
	lock_guard<mutex> LG1(m4);
	cout << "total Complete Product " << totalCompleteProduct << endl;
	if (type == 1) {
		cout << "ProductWorker " << id;
	}
	else {
		cout << "PartWorker " << id;
	}
	cout << " Buffer State" << ": ";
	for (auto& item : bufferState)
	{
		cout << item << " ";
	}
	cout << endl;
}

int random(int range_from, int range_to) {
	// srand(system_clock::now().time_since_epoch().count());
	// return rand() % 5; // this method will generate duplicate order

	// random number generator from cppreference.com
	// https://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> distrib(range_from, range_to);
	return distrib(gen);
}

partWork::partWork(int workerId, int waitLength, int partNumber, string logLocation) {
	this->id = workerId;
	this->fileLocation = logLocation;
	this->partSize = partNumber;
	this->oldOrder = { 0, 0, 0, 0, 0 };
	this->loadOrder = { 0, 0, 0, 0, 0 };
	this->accumulatedWaitTime = chrono::microseconds(0);
	this->waitTimeLength = waitLength;
	this->waitTime = chrono::microseconds(waitTimeLength);
}

void partWork::freshStart() {
	this->loadOrder.clear();
	this->loadOrder = { 0, 0, 0, 0, 0 };
	this->accumulatedWaitTime = chrono::microseconds(0);
	this->start = globalStart;
	this->waitTime = chrono::microseconds(waitTimeLength);
	partOrderGenerator();
	//debug
	debugUpdateCount = 0;
	debugUpdateTime.clear();
}

void partWork::partOrderGenerator() {
	int currentHave = std::accumulate(this->oldOrder.begin(), this->oldOrder.end(), 0);
	loadOrder.clear();
	loadOrder = oldOrder;
	//this->loadOrder.assign(this->oldOrder.begin(), this->oldOrder.end());
	for (int i = 0; i < this->partSize - currentHave; ++i) {
		this->loadOrder[random(0, 4)]++;
	}
}

int partWork::buildPartTime() {
	int totalTime = 0;
	for (int i = 0; i < partSize; ++i) {
		totalTime += produceTime[i] * (loadOrder[i] - oldOrder[i]);
	}
	return totalTime;
}

int partWork::movePartTime() {
	int totalTime = 0;
	for (int i = 0; i < partSize; ++i) {
		totalTime += moveTime[i] * loadOrder[i];
	}
	return totalTime;
}

bool partWork::canLoadToBuffer(vector<int>& bufferStates) {
	for (int i = 0; i < partSize; ++i) {
		if (bufferStates[i] < maxBufferState[i] && loadOrder[i] != 0) {
			return true;
		}
		else {
			continue;
		}
	}
	return false;
}


void partWork::loadToBuffer(vector<int>& bufferStates) {
	for (int i = 0; i < partSize; ++i) {
		if (bufferStates[i] < maxBufferState[i] && loadOrder[i] != 0) {
			int itemToLoad = min(loadOrder[i], maxBufferState[i] - bufferStates[i]);
			loadOrder[i] -= itemToLoad;
			bufferStates[i] += itemToLoad;
		}
	}

}

bool partWork::isEmptyOrder() {
	return std::all_of(this->loadOrder.begin(), this->loadOrder.end(), [](int item) { return item == 0; });
}

//void partWork::emptyOldOrder() {
//    this->oldOrder.clear();
//    this->oldOrder = {0,0,0,0,0};
//}

void partWork::transferOrder() {
	oldOrder.clear();
	oldOrder = loadOrder;
	/*for (int i = 0; i < partSize; ++i) {
		oldOrder[i] = loadOrder[i];
	}*/
}


void partWork::updateWaitTime(durationTimeVariable waitedTime) {
	//if (waitedTime.count()<0||waitTime.count()<0)
	//{
	//    cout << "Wrong" << endl;
	//}
	this->accumulatedWaitTime += waitedTime;
	//debugUpdateTime.push_back(waitedTime.count());
	this->waitTime -= waitedTime;
	//debugUpdateCount++;
	if (waitTime.count() < 0)
	{
		waitTime = chrono::microseconds(0);
	}
}

void partWork::printBeforeStatus(int iteration, int status, const vector<int>& bufferStates) {
	/// this function need protection
	lock_guard<mutex> LG1(m2);
	this->end = chrono::high_resolution_clock::now();
	ofstream log;
	log.open(fileLocation, std::ios_base::app);
	log << "Current Time: " << calTimePassed(this->start, this->end).count() << " us" << endl;
	log << "Iteration: " << iteration << endl;
	log << "Part Worker ID: " << this->id << endl;
	log << "Status: " << statusVector[status] << endl;
	log << "Accumulated Wait Time: " << this->accumulatedWaitTime.count() << " us" << endl;
	log << "Buffer State: (" << bufferStates[0] << "," << bufferStates[1] << ",";
	log << bufferStates[2] << "," << bufferStates[3] << "," << bufferStates[4] << ")" << endl;
	log << "Load Order: (" << this->loadOrder[0] << "," << this->loadOrder[1] << ",";
	log << this->loadOrder[2] << "," << this->loadOrder[3] << "," << this->loadOrder[4] << ")" << endl;

	// debug 
	/*log << "Accumulated counter: " << debugUpdateCount << endl;
	for (auto& item : debugUpdateTime) {
		log << item << endl;
	}*/
	log.close();
}

void partWork::printAfterState(const vector<int>& bufferStates) {
	lock_guard<mutex> LG1(m2);
	ofstream log;
	log.open(fileLocation, std::ios_base::app);
	log << "Updated Buffer State: (" << bufferStates[0] << "," << bufferStates[1] << ",";
	log << bufferStates[2] << "," << bufferStates[3] << "," << bufferStates[4] << ")" << endl;
	log << "Updated Load Order: (" << loadOrder[0] << "," << loadOrder[1] << ",";
	log << loadOrder[2] << "," << loadOrder[3] << "," << loadOrder[4] << ")" << endl;
	log << endl;
	log.close();
}

//bool partWork::partThreadWait(unique_lock<mutex>& lck, condition_variable& cv, vector<int>& bufferStates) {
//	before = chrono::high_resolution_clock::now();
//    while (!canLoadToBuffer(bufferStates)) {
//        if (cv.wait_for(lck, waitTime) == cv_status::timeout)
//			after = chrono::high_resolution_clock::now();
//            return canLoadToBuffer(bufferStates);
//    }
//    after = chrono::high_resolution_clock::now();
//    return true;
//}

bool partWork::partThreadWait(unique_lock<mutex>& lck, condition_variable& cv, vector<int>& bufferStates) {
	before = chrono::high_resolution_clock::now();
	while (!canLoadToBuffer(bufferStates)) {
		if (cv.wait_until(lck, before + waitTime) == cv_status::timeout) {
			after = chrono::high_resolution_clock::now();
			return canLoadToBuffer(bufferStates);
		}
	}
	after = chrono::high_resolution_clock::now();
	return true;
}

void partWork::debugPrintLoadOrder()
{
	lock_guard<mutex> LG1(m4);
	cout << "PartWorker " << isEmptyOrder();
	cout << " Load Order" << ": ";
	for (auto& item : loadOrder)
	{
		cout << item << " ";
	}
	cout << endl;
}


productWork::productWork(int workerId, int waitLength, int partNumber, string logLocation) {
	this->id = workerId;
	this->fileLocation = logLocation;
	this->partSize = partNumber;
	this->localState = { 0, 0, 0, 0, 0 };
	this->pickUpOrder = { 0, 0, 0, 0, 0 };
	this->cartState = { 0, 0, 0, 0, 0 };
	this->tmpLoad = { 0, 0, 0, 0, 0 };
	this->accumulatedWaitTime = chrono::microseconds(0);
	this->waitTimeLength = waitLength;
	this->waitTime = chrono::microseconds(waitTimeLength);
}

void productWork::freshStart() {
	this->accumulatedWaitTime = chrono::microseconds(0);
	this->start = globalStart;
	this->waitTime = chrono::microseconds(waitTimeLength);
	this->cartState.clear();
	this->cartState = { 0,0,0,0,0 }; //TODO: need check if works.
	this->pickUpOrder.clear();
	this->pickUpOrder = { 0, 0, 0, 0, 0 };
	this->tmpLoad = { 0, 0, 0, 0, 0 };
	assemblyOrderGenerator();
}

void productWork::assemblyOrderGenerator() {
	int curHave = std::accumulate(localState.begin(), localState.end(), 0);
	int typeCount = 0;
	if (curHave != 0) {
		for (const auto& part : localState) {
			if (part != 0) {
				typeCount++;
			}
		}
	}
	/// generate new order
	vector<int> tmpOrder;
	while (true) {
		tmpOrder.clear();
		tmpOrder = localState;

		int tmpTypeCount = typeCount;
		for (int i = 0; i < partSize - curHave;) {
			int newPart = random(0, 4);
			if (tmpOrder[newPart] == 0 && tmpTypeCount < maxTypeCount) {
				tmpTypeCount++;
				tmpOrder[newPart]++;
				++i;
			}
			else if (tmpOrder[newPart] != 0) {
				tmpOrder[newPart]++;
				++i;
			}
			else {
				continue;
			}
		}
		if (tmpTypeCount != 1) {
			break;
		}
	}
	this->pickUpOrder.clear();
	pickUpOrder = { 0,0,0,0,0 };
	for (int i = 0; i < partSize; ++i) {
		pickUpOrder[i] = tmpOrder[i] - localState[i];
	}
	//this->pickUpOrder.assign(tmpOrder.begin(), tmpOrder.end());
}



bool productWork::canLoadToCart(vector<int>& bufferStates) {
	for (int i = 0; i < partSize; ++i) {
		if (pickUpOrder[i] > 0 && bufferStates[i] > 0) {
			return true;
		}
		else {
			continue;
		}
	}
	return false;

}

bool productWork::finishedLoad() {
	//    return (cartState == pickUpOrder);
		//return pickUpOrder == localState;
	return std::all_of(this->pickUpOrder.begin(), this->pickUpOrder.end(), [](int item) { return item == 0; });

}

void productWork::loadToCart(vector<int>& bufferStates) {
	tmpLoad = { 0,0,0,0,0 };
	for (int i = 0; i < partSize; ++i) {
		if (pickUpOrder[i] > 0 && bufferStates[i] > 0) {
			tmpLoad[i] = min(pickUpOrder[i], bufferStates[i]);
			bufferStates[i] -= tmpLoad[i];
			cartState[i] += tmpLoad[i];
			pickUpOrder[i] -= tmpLoad[i];
		}
	}
	//for (int i = 0; i < partSize; ++i) {
	//    cartState[i] += tmpLoad[i];
	//    pickUpOrder[i] -= tmpLoad[i];
	//}
}

void productWork::updateWaitTime(durationTimeVariable waitedTime) {
	this->accumulatedWaitTime += waitedTime;
	this->waitTime -= waitedTime;
}

int productWork::movePartTime() {
	int totalTime = 0;
	for (int i = 0; i < partSize; ++i) {
		totalTime += moveTime[i] * cartState[i];
	}
	tmpLoad.clear(); // TODO: empty tmpLoad
	return totalTime;
}

int productWork::assemblePartTime() {
	int totalTime = 0;
	for (int i = 0; i < partSize; ++i) {
		totalTime += assemblyTime[i] * (cartState[i] + localState[i]);
	}

	return totalTime;
}

void productWork::buildNewParts(productWorkerPrinter& tobePrinted) {
	lock_guard<mutex> LG1(m3);

	++totalCompleteProduct; // TODO: may need protection
	//cout << totalCompleteProduct << endl;
	cartState = { 0,0,0,0,0 };
	localState = { 0,0,0,0,0 };
	//printFinished(*completeProductCount);
}

void productWork::waitForParts() {
	for (int i = 0; i < partSize; ++i) {
		localState[i] += cartState[i];
	}
	cartState = { 0,0,0,0,0 };
	//localState = cartState; // TODO: may not work
}

void productWork::printBeforeStatus(int iteration, int status, const vector<int>& bufferStates) {
	/// this function need protection
	lock_guard<mutex> LG1(m2);
	this->end = chrono::high_resolution_clock::now();
	ofstream log;
	log.open(this->fileLocation, std::ios_base::app);
	log << "Current Time: " << calTimePassed(this->start, this->end).count() << " us" << endl;
	log << "Iteration: " << iteration << endl;
	log << "Part Worker ID: " << this->id << endl;
	log << "Status: " << statusVector[status] << endl;
	log << "Accumulated Wait Time: " << this->accumulatedWaitTime.count() << " us" << endl;
	log << "Buffer State: (";
	printStates(log, bufferStates);
	log << "Pickup Order: (";
	printStates(log, pickUpOrder);
	log << "Local State: (";
	printStates(log, localState);
	log << "Cart State: (";
	printStates(log, cartState);
	log.close();
}

void productWork::printAfterState(const vector<int>& bufferStates) {
	lock_guard<mutex> LG1(m2);
	ofstream log;
	log.open(this->fileLocation, std::ios_base::app);
	log << "Updated Buffer State: (";
	printStates(log, bufferStates);
	log << "Updated Pickup Order: (";
	printStates(log, pickUpOrder);
	log << "Updated Local State: (";
	printStates(log, localState);
	log << "Updated Cart State: (";
	printStates(log, cartState);
	log.close();
}


void productWork::printTotalFinished(const int CompleteProductCount) {
	lock_guard<mutex> LG1(m2);
	ofstream log;
	log.open(this->fileLocation, std::ios_base::app);
	log << "Total Completed Products: " << CompleteProductCount << endl << endl;
	log.close();
}

void productWork::printFinished(const int CompleteProductCount) {
	lock_guard<mutex> LG1(m2);
	ofstream log;
	log.open(this->fileLocation, std::ios_base::app);
	this->end = chrono::high_resolution_clock::now();
	log << "Current Time: " << calTimePassed(this->start, this->end).count() << " us" << endl;
	log << "Updated Local State: (";
	printStates(log, localState);
	log << "Updated Cart State: (";
	printStates(log, cartState);
	log << "Total Completed Products: " << CompleteProductCount << endl << endl;
	log.close();
	//printTotalFinished();
}

void productWork::structPrinter(const productWorkerPrinter& tobePrinted)
{
	lock_guard<mutex> LG1(m2);
	ofstream log;
	log.open(this->fileLocation, std::ios_base::app);
	log << "Current Time: " << calTimePassed(start, tobePrinted.bufferTimer).count() << " us" << endl;
	log << "Iteration: " << tobePrinted.iteration << endl;
	log << "Product Worker ID: " << id << endl;
	log << "Status: " << statusVector[tobePrinted.Status] << endl;
	log << "Accumulated Wait Time: " << tobePrinted.accWaitTime.count() << " us" << endl;
	log << "Buffer State: (";
	printStates(log, tobePrinted.beforeBuffer);
	log << "Pickup Order: (";
	printStates(log, tobePrinted.beforePickUp);
	log << "Local State: (";
	printStates(log, tobePrinted.beforeLocal);
	log << "Cart State: (";
	printStates(log, tobePrinted.beforeCart);

	log << "Updated Buffer State: (";
	printStates(log, tobePrinted.updateBuffer);
	log << "Updated Pickup Order: (";
	printStates(log, tobePrinted.updatePickUp);
	log << "Updated Local State: (";
	printStates(log, tobePrinted.updateLocal);
	log << "Updated Cart State: (";
	printStates(log, tobePrinted.updateCart);
	if (tobePrinted.printMode == 1)
	{
		log << "Current Time: " << calTimePassed(start, tobePrinted.returnedTimer).count() << " us" << endl;
		log << "Updated Local State: (";
		printStates(log, tobePrinted.endLocal);
		log << "Updated Cart State: (";
		printStates(log, tobePrinted.endCart);
	}
	log << "Total Completed Products: " << tobePrinted.totalCount << endl << endl;
	log.close();
}



void productWork::debugPrintPickUpOrder()
{
	lock_guard<mutex> LG1(m4);
	cout << "ProductWorker " << id;
	cout << " PickUp Order" << ": ";
	for (auto& item : pickUpOrder)
	{
		cout << item << " ";
	}
	cout << endl;
}

bool productWork::productThreadWait(unique_lock<mutex>& lck, condition_variable& cv, vector<int>& bufferStates) {
	while (!canLoadToCart(bufferStates))
		if (cv.wait_for(lck, waitTime) == cv_status::timeout)
			return canLoadToCart(bufferStates);
	return true;
}