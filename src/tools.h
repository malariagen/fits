#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <string>
#include <sstream>
#include <iostream>
#include <vector>

using namespace std ;

class StringTools {
public:
	string implode ( const vector <string> &vs , bool quote = true ) ;
	string i2s ( int i ) ;
	int s2i ( string s ) ;
	bool isNumeric ( string s ) ;
	string exec(string cmd) ;
	string getCurrentTimestamp () ;
	vector <string> split ( string main , char sep ) {
		vector <string> ret ;
		istringstream f(main);
		string s;
		while (getline(f, s, sep)) ret.push_back ( s ) ;
		return ret ;
	}
} ;

#endif
