// Solve a network flow problem by the lift-to-front preflow-push
// algorithm.  See "Introduction to Algorithms" by Cormen, Leierson, 
// and Rivest (CLR).

#pragma once
#include <vector>
#include <string>
#include <list>
#include <iterator>
#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <set>
#include <queue>
#include "util.h"
using namespace std;

class Network
{
private:
	vector<string>		// vertex labels for I/O
	   label;           //   first is the source and last is the sink
	vector<list<int>>   // adjacency lists for underlying
	   neighbor;        //   undirected graph
	vector<list<int>::iterator>
	   next;			// iterator for adjacency list
	int N;				// number of nodes in network
	int *f;	            // flows
	int *c;             // capacity
	int *e;				// excess
	int *h;				// height
	int sink;
	bool readEdge(ifstream& in, map<string, int>& index);
	inline int& flow(int init, int term) 
					{return f[N*init + term];}
	inline int& capacity(int init, int term) 
					{return c[N*init + term];}
	void push(int u, int v);
	void lift(int u);
	void discharge(int u);
	
public:
	Network(const char* filename);
	int maxFlow(void);
	void printFlow(const char* filename);
	void printCut(const char* filename);
};

