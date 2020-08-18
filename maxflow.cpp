#include "maxflow.h"
# include <cassert>

Network::Network(const char *inf)
{
	ifstream infile(inf);
	if ( ! infile )
	{	string msg = "Could not open ";
		msg += inf;
		throw exception( msg.c_str() );
	}
	getStrings(infile, label);
	N = int(label.size());
	sink = N-1;
	map<string,int> index;	// inverted list for labels
	for ( int i = 0; i < N; i++ )
		index[label[i]] = i;
	f = new int[N*N];
	c = new int[N*N];
	e = new int[N];
	h = new int[N];
	if ( f == 0 || c == 0 || e == 0 || h == 0)
		throw exception("Too many nodes");
	for ( int i = 0; i < N*N; i++ )
		f[i] = c[i] = 0;
	for ( int i = 0; i < N; i++ )
		e[i] = h[i] = 0;
	list<int> blank;
	for ( int i = 0; i < N; i++ )
		neighbor.push_back(blank);		
	while ( readEdge(infile, index) )
		;
	infile.close();
	// Complete the initialization
	// See Initialize-Preflow in CLR p. 608
	h[0] = N;
	for ( int i = 0; i < N; i++ )
		next.push_back(neighbor[i].begin());
	list<int>::iterator it = neighbor[0].begin();
	while ( it != neighbor[0].end() )
	{	int u = *it++;
		int cap = capacity(0, u);
		flow(0, u) = cap;
		flow(u, 0) = -cap;
		e[u] = cap;
	}
}

bool
Network::readEdge(std::ifstream &in, std::map<string,int> &index)
{
	// Read an edge from in.  An edge is of the form
	//    initial_vertex terminal_vertex capacity
	// where the initial and terminal vertices are strings
	// in the label list and capacity is a positive integer.
	// An edge with a capacity of 0 is an input error.

	// An edge can appear at most once, but oppositely-oriented
	// edges with the same endpoints are allowed.
	// Loops (self-edges) are not allowed.
	
	// Return true is an edge was read successfully, and false
	// if the input is exhausted.  Throw an exception on any error.

	string init;
	string term;
	int cap = 0;

	in >> init;
	if ( in.eof() )
		return false;
	in >> term;
	in >> cap;
	if ( in.eof() )
		throw exception("Input file exhausted while reading edges.");
	if ( index.count(init) == 0 )
	{	string msg = "Unknown vertex ";
		msg += init + " in input file.";
		throw exception( msg.c_str() );
	}
	if ( index.count(term) == 0 )
	{	string msg = "Unknown vertex ";
		msg += term + " in input file.";
		throw exception( msg.c_str() );
	}
	if ( init == term )
	{	string msg = "Loop (self-edge) not allowed: (";
		msg += init + ", " + term + ").)";
		throw exception( msg.c_str() );
	}	
	if ( cap <= 0 )
	{	string msg = "Invalid capacity for edge (";
		msg += init + ", " + term + ").)";
		throw exception( msg.c_str() );
	}		
	int v1 = index[init];
	int v2 = index[term];
	if ( v2 == 0 )
		throw exception( "Inflow to source" );
	if ( v1 == sink )
		throw exception( "Outflow from sink" );

	//Edge looks good.  Check for duplication

	if ( capacity(v1, v2) > 0 )
	{	string msg = "Duplicate edge (";
		msg += init + ", " + term + ").)";
		throw exception( msg.c_str() );
	}
	
	// Good edge
	capacity(v1, v2) = cap;
	// Update neighbor lists if the oppositely-oriented
    // edge hasn't already been added.
	if ( capacity(v2, v1) == 0 )
	{
		neighbor[v1].push_back(v2);
		neighbor[v2].push_back(v1);
	}	

	return true;
}

void 
Network::push(int u, int v)
{
	// See CLR
	// Preconditions:
	//		1. u is overflowing
	//		2. Residual capacity in edge (u, v) is positive
	//		3. h[u] = h[v] + 1
	// Action: Push min(e[u], c[u][v]-f[u][v]) units from
	//         u to v
	assert(e[u] >0);					// precondition 1
	assert(capacity(u,v) > flow(u,v));	// precondition 2
	assert(h[u] == h[v] + 1);			// precondition 3

	int d = min(e[u], capacity(u, v) - flow(u, v));
	flow(u, v) += d;
	flow(v, u) = -flow(u, v);
	e[u] -= d;
	e[v] += d;
}

void 
Network::lift(int u)
{	
	// See CLR
	// Preconditions:
	//	1. u is overflowing
	//  2. for all vertices v, if (u, v) is a residual edge,
	//     then h[u] <= h[v] 
	// Action: Increase the height of u by the smallest amount
	//     necessary to allow flow to be pushed from u.

	assert(e[u] > 0);		// precondition 1
	
	int low = 2*N*N+1;		// "infinite" height
							// (See CLR Corollary 27.21)
	list<int>::iterator it;
	list<int>::iterator stop = neighbor[u].end();
	for ( it = neighbor[u].begin(); it != stop ; it++ )
	{	int v = *it;
		if ( capacity(u, v) > flow(u, v) ) // residual edge
		{	assert(h[u] <= h[v]);		   // precondition 2
			low = min(low, h[v]);
		}
	}
	h[u] = low + 1;
}

void 
Network::discharge(int u)
{
	// From CLR: An overflowing vertex u is discharged by
	// pushing all of its excess flow through admissible
	// edges to neighboring vertices, lifting u as necessary
	// to cause edges leaving u to become admissible.

	while ( e[u] > 0 ) // while u is overflowing
	{	int v; 
		if ( next[u] == neighbor[u].end() )
		{	lift(u);
			next[u] = neighbor[u].begin();
		}
		else
		{	v = *next[u];
			if ( capacity(u, v) > flow(u, v) && h[u] == h[v] + 1)
				push(u, v);
			else
				next[u]++;
		}
	}
}

int 
Network::maxFlow(void)
{
	// Run the lift-to-front algorithm
	list<int> L;	// list of all nodes but source and sink
	for ( int i = 1; i < sink; i++ )
		L.push_back(i);
	list<int>::iterator iter = L.begin();
	list<int>::iterator stop = L.end();
	while ( iter != stop )
	{	int u = *iter;
		int oldHeight = h[u];
		discharge(u);
		if ( h[u] > oldHeight )
		{	L.erase( iter );
			L.push_front( u );
			iter = L.begin();
		}
		iter++;
	}
	// The value of the flow is the flow out of the source
	int F = 0;

	iter = neighbor[0].begin();
	stop = neighbor[0].end();

	while ( iter != stop )
		F += flow(0, *iter++);
	return F;
}

void 
Network::printFlow(const char* filename)
{
    ofstream fout(filename);
    if ( ! fout )
    {   string msg = "Could not open  ";
		msg += filename;
		msg += " for output.";
		throw exception(msg.c_str());
    }
    fout << "Flows on edges are:" << endl;
    fout << setw(12) << "Initial" << setw(12) << "Terminal" <<
            setw(12) << "Flow" << endl;
    for ( int i = 0; i < N; i++ )
    {   for ( int j = 0; j < N; j++ )
            if ( flow(i, j) > 0 )
                fout << setw(12) << label[i] << setw(12) << label[j]
					 << setw(12) << flow(i, j) << endl;
    }
    fout << "All other flows are 0 " << endl << endl;
    fout.close();
} 

void 
Network::printCut(const char* filename)
{
	// Find and print a minimal cut
	// Run one iteration of the labeling procedure

	set<int> cut;
	queue<int> q;
	vector<bool> visited;

	for ( int i = 0; i < sink; i++ )
		visited.push_back(false);
	q.push(0);
	visited[0] = true;
	cut.insert(0);
	while ( ! q.empty() )
	{	int u = q.front();
		q.pop();
		list<int>::iterator iter = neighbor[u].begin();
		list<int>::iterator stop = neighbor[u].end();
		while ( iter != stop )
		{	int v = *iter++;
			if ( flow(u, v) < capacity(u, v) && ! visited[v] )
			{	q.push(v);
				visited[v] = true;
				cut.insert(v);
			}
		}
	}
	ofstream fout(filename);
	if ( ! fout )
	{	string msg = "Could not open ";
		msg += filename;
		msg += " for output";
		throw exception(msg.c_str());		
	}
	fout << "Minimal Cut:\nP = {" << label[0];
    for ( int k = 1; k < N; k++ )
    {   if ( cut.count(k) > 0 )
			fout << ", " << label[k];
    }
    fout << "}" << endl;
    fout << "Q = {";
    for ( int k = 1; k < sink; k++ )
    {   if ( cut.count(k) == 0 )
            fout << label[k] << ", "; 
    }
    fout << label[sink] << "}" << endl;
    fout.close();
}
