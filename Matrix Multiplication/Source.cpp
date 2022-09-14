#include <iostream>
#include <vector>
#include <thread>
#include <list>
#include <fstream>

using namespace std;

void innerLoop(vector<vector<int>> &Dist, vector<vector<int>> &Next, int outerLoop, int start, int end, int Nodes);

int main() {
    /// Read in graph
	//Read in number of nodes and number of edges
	ifstream In(R"(C:\Users\xkzmx\Documents\Odyssey\School\Thread\HW1\graph.txt)");

    if (!In.is_open()){
        // check if file can be find
        cout << "Can't find file." << endl;
        return 0;
    }

	int Nodes, Edges;
	In >> Nodes >> Edges;
	int a, b, c;

    vector < list<pair<int, int>>> Graph(Nodes);//read in from graph.txt

    for (int i = 0; i < Edges; i++) {
        In >> a >> b >> c;
        bool exist = false;
        for(auto i = Graph[a].begin();i != Graph[a].end();i++){
            if (i->first == b){
                exist = true;
                if(i->second > c){
                    /// current cost is larger, replace
                    Graph[a].erase(i);
                    Graph[a].push_back(make_pair(b, c));
                }
            }
        }
        if(exist == false){
            Graph[a].push_back(make_pair(b, c));
        }
    }

	In.close();

    // initialize distance & cost matrix
    vector<vector<int>> Dist(Nodes, vector<int>(Nodes, 9999));
    vector<vector<int>> Next(Nodes, vector<int>(Nodes, -1));

    for (int i = 0; i < Nodes; ++i) {
        Dist[i][i] = 0;
    }
    for (int i = 0; i < Nodes; ++i) {
        Next[i][i] = i;
    }

    for (int i = 0; i < Nodes; ++i) {
        for (const auto &item: Graph[i]) {
            Dist[i][item.first] = item.second;
            Next[i][item.first] = item.first;
        }

    }


    //Implement threaded programming
    for (int i = 0; i < Nodes; ++i) {

        thread T1{ innerLoop,ref(Dist), ref(Next),i,0, Nodes/4,Nodes };
        thread T2{ innerLoop,ref(Dist), ref(Next),i,Nodes/4, Nodes/2,Nodes };
        thread T3{ innerLoop,ref(Dist), ref(Next),i,Nodes/2, 3*Nodes/4,Nodes };
        innerLoop(Dist, Next,i,3*Nodes/4,Nodes,Nodes);
        T1.join();
        T2.join();
        T3.join();
    }

    //Print results
	cout << "Dist Matrix" << endl;
	for (auto& i : Dist) {
		for (auto& j : i) {
			cout << j << " ";
		}
		cout << endl;
	}
	cout << endl;
	cout << "Next Matrix" << endl;

	for (auto& i : Next) {
		for (auto& j : i) {
			cout << j << " ";
		}
		cout << endl;
	}


	return 0;
}

void innerLoop(vector<vector<int>> &Dist, vector<vector<int>> &Next, int outerLoop, int start, int end, int Nodes) {
    for (int i = start; i < end; ++i) {
        for (int j = 0; j < Nodes; ++j) {
            if (i != j && i != outerLoop && j != outerLoop)
            if (Dist[i][j] > (Dist[i][outerLoop]+Dist[outerLoop][j])){
                Dist[i][j] = Dist[i][outerLoop]+Dist[outerLoop][j];
                Next[i][j] = Next[i][outerLoop];
            }
        }

    }
}





