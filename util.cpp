#include "util.h"

bool getStrings(istream& infile, vector<string>& strings, char sep )
{	// Get a line from infile and break it into strings,
	// adding them to strings. Returns true all input has
	// been exhausted, and false oftherwise
	
	string input;	
	stringstream ss (stringstream::in | stringstream::out);

	strings.clear();
	getline(infile, input, sep);
	
	// erase trailing white space
	int k = static_cast<int>(input.size()) - 1;
	while ( k >= 0 && isspace( input[k] ) )
		k--;
	input.erase(k+1, static_cast<int>(input.size()) - k);

	if ( ! input.empty() )
	{	string str;

		ss.str(input);
		ss >> str;
		strings.push_back(str);
		do
		{	ss >> str;
			strings.push_back(str);
		} while ( ! ss.eof() );
	}
	return ( input.empty() && infile.eof() );
}


