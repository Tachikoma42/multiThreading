//HW1 by Sihao Ren
//SU Net ID: sren03 SUID: 947105531

/*

Since in the loop, the i,j,k of the Next matrix in each loop is the same as the index of the Dist matrix,
we can just focus on the data dependency of the Dist matrix.

The outer loop k can't be paralleled,
since the future loop k may modify data used in the past loop k. and the past loop k may modify data used in the future loop.
for example in the loop k = 0, when i = 1 j = 2, we may have to modify Dist(1,2), but when k = 1 i = 0, j = 2, we need Dist(1,2) as the augend.
if the second compare runs before the first compare, the result may be wrong.

Before analysis the dependency in the inner loop, we can skip some compare operations:
when one of the three indexes equal to another index, we can skip them,
since the graph does not allow negative loop, and the distance of self is 0.
for example (when k = 2, i = 2, j = 2 D(2,2) ? D(2,2)+D(2,2) or k = 2 i = 1 j =2 D(1,2)?D(1,2)+D(2,2))
after skip them, we actually skip all the data dependency in the inner loop.

D(i, j) ? D(i, k)+D(k,j) In the same kth inner loop, only when (j == k or i == k or i == j) previous output will be used as input for later, but since these scenarios can be skipped as analyzed before. 
There are no more data dependency in inner loop. We can safely use thread to calculate them.


*/
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





