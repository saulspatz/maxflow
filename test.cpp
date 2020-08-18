#include "maxflow.h"

int main(int argc, char* argv[])
{
	if ( argc != 3 )
	{	cout << "Usage: test infile outfile"  << endl;
		return 1;
	}
	try
	{	Network ntw(argv[1]);
		cout << ntw.maxFlow();
		//ntw.printFlow(argv[2]);
		ntw.printCut(argv[2]);
	}
	catch ( exception xcpt )
	{	cout << xcpt.what(); 
		return 1;
	}
	return 0;
}