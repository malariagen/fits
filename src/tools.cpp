#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include "tools.h"


string StringTools::implode ( const vector <string> &vs , bool quote ) {
	string ret ;
	if ( vs.empty() ) return ret ;
	for ( auto &s:vs ) {
		if ( !ret.empty() ) ret += "," ;
		ret += quote ? "\"" + s + "\"" : s ;
	}
	return ret ;
}

string StringTools::i2s ( int i ) {
    char s[100] ;
    sprintf ( s , "%d" , i ) ;
    return string(s) ;
}

int StringTools::s2i ( string s ) {
	return atoi ( s.c_str() ) ;
}

bool StringTools::isNumeric ( string s ) {
    for ( auto c:s ) {
        if ( c<'0' || c>'9' ) return false ;
    }
    return true ;
}

string StringTools::exec(string cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}

string StringTools::getCurrentTimestamp () {
    time_t timer;
    char buffer[26];
    struct tm* tm_info;

    time(&timer);
    tm_info = localtime(&timer);

    strftime(buffer, 26, "%Y%m%d%H%M%S", tm_info);
    return string ( buffer ) ;
}
