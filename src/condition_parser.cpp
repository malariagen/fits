#include <iostream>
#include <fstream>
#include <algorithm>
#include "condition_parser.h"


const vector <string> ConditionParser::comparison = { ">=" , "<=" , "=" , "<" , ">" , "!=" } ;
const vector <string> ConditionParser::logical = { "and" , "or" , "in" } ;

void ConditionParserElement::out ( int indent ) {
    cout << std::string(indent*3, ' ') ;
    switch ( type ) {
        case ConditionParserElementType::ROOT: cout << "<R>" ; break ;
        case ConditionParserElementType::COMPARISON: cout << "<C> " << s ; break ;
        case ConditionParserElementType::VARIABLE: cout << "<V> " << s ; break ;
        case ConditionParserElementType::NUMERIC: cout << "<N> " << s ; break ;
        case ConditionParserElementType::LOGICAL: cout << "<L> " << s ; break ;
        case ConditionParserElementType::ENUM: cout << "<E> " << s ; break ;
        case ConditionParserElementType::CONSTANT: cout << "<_> '" << s << "'" ; break ;
        case ConditionParserElementType::SUB: cout << "<S>" ; break ;
    }
    cout << endl ;
    for ( auto &s:sub ) s.out(indent+1) ;
}

void ConditionParserElement::fixup () {

	// Load file
	if ( type == ConditionParserElementType::VARIABLE && s[0] == '@' ) {
		string filename = s.substr(1) ;
		type = ConditionParserElementType::SUB ;
		s.clear() ;
		sub.clear() ;

		std::ifstream is ( filename.c_str() , std::ifstream::in ) ;
		while ( !is.eof() ) {
			string line , col1 ;
			getline ( is , line ) ;
			std::size_t found = line.find_first_of("\t");
			if ( found == std::string::npos ) col1 = line ;
			else col1 = line.substr(0,found) ;
			if ( col1.empty() ) continue ;
			sub.push_back ( ConditionParserElement ( ConditionParserElementType::CONSTANT , col1 ) ) ;
		}
	}

	// Remove comma elements from list
	if ( type == ConditionParserElementType::SUB ) {
		vector <ConditionParserElement> sub2 ;
		for ( auto &e:sub ) {
			if ( e.type == ConditionParserElementType::ENUM ) continue ;
			sub2.push_back ( e ) ;
		}
		sub2.swap ( sub ) ;
	}

	// Process all the sub-elements
	for ( auto &e:sub ) {
		e.fixup() ;
	}

	// Single sub-sub
	while ( type == ConditionParserElementType::SUB && sub.size() == 1 && sub[0].type == ConditionParserElementType::SUB ) {
		sub = sub[0].sub ;
	}

}

bool ConditionParser::isValidNumeric ( char c , const string &numeric ) {
    if ( c >= '0' && c <= '9' ) return true ;
    if ( c == '-' && numeric.empty() ) return true ;
    if ( c == '.' && !numeric.empty() ) return true ;
    return false ;
}

bool ConditionParser::isValidVariableChar ( char c ) {
    if ( c == '_' || c == '@' || c == '/' || c == '.' || c == '-' ) return true ;
    if ( c >= 'a' && c <= 'z' ) return true ;
    if ( c >= '0' && c <= '9' ) return true ;
    if ( c >= 'A' && c <= 'Z' ) return true ;
    return false ;
}

void ConditionParser::addNewVariable ( string &variable_name , ConditionParserElement &e ) {
    if ( variable_name.empty() ) return ;
    e.sub.push_back ( ConditionParserElement ( ConditionParserElementType::VARIABLE , variable_name ) ) ;
    variable_name.clear() ;
}

void ConditionParser::addNewNumeric ( string &numeric , ConditionParserElement &e ) {
    if ( numeric.empty() ) return ;
    e.sub.push_back ( ConditionParserElement ( ConditionParserElementType::NUMERIC , numeric ) ) ;
    numeric.clear() ;
}

bool ConditionParser::parse ( string query ) {
    cout << "QUERY: " << query << endl ;
    int32_t end_pos = parseFromPosition ( query , 0 , root ) ;
    if ( end_pos == -1 || end_pos != query.length() ) return false ;
    root.fixup() ;
//    root.out() ;
    return true ;
}

int32_t ConditionParser::parseFromPosition ( const string &query , int32_t pos , ConditionParserElement &e ) {
    string variable_name ;
    string constant ;
    string numeric ;
    char quote_char ;
    bool in_quote = false ;
    string query_lc = query ;
    std::transform(query_lc.begin(), query_lc.end(), query_lc.begin(), ::tolower);
    while ( pos < query.size() ) {
        char c = query[pos] ;
        if ( c == ' ' || c == '\n' || c == '\t' ) {
            addNewVariable(variable_name,e) ;
            addNewNumeric(numeric,e) ;
            pos++ ;
            continue ;
        }

        if ( in_quote ) {
            if ( c == quote_char ) {
                in_quote = false ;
                e.sub.push_back ( ConditionParserElement ( ConditionParserElementType::CONSTANT , constant ) ) ;
                pos++ ;
                continue ;
            }
            // TODO backslash-escaped quote
            constant += c ;
            pos++ ;
            continue ;
        }

        if ( c == 39 || c == '"' ) {
            constant.clear() ;
            in_quote = true ;
            quote_char = c ;
            pos++ ;
            continue ;
        }

        for ( auto &what:logical ) {
            if ( pos == 0 ) continue ;
            if ( isValidVariableChar(query_lc[pos-1]) ) continue ; // Don't start in the middle of a variable...
            if ( query_lc.substr(pos,what.size()) != what ) continue ;
            addNewVariable(variable_name,e) ;
            addNewNumeric(numeric,e) ;
            if ( e.sub.size() > 1 || ( e.sub.size() == 1 && e.sub[0].type != ConditionParserElementType::SUB ) ) { // Sub-group first part, if necessary
                ConditionParserElement ne ( ConditionParserElementType::SUB ) ;
                for ( auto &s:e.sub ) ne.sub.push_back ( s ) ;
                e.sub.clear() ;
                e.sub.push_back ( ne ) ;
            }
            e.sub.push_back ( ConditionParserElement ( ConditionParserElementType::LOGICAL , what ) ) ;
            e.sub.push_back ( ConditionParserElement ( ConditionParserElementType::SUB ) ) ;
            return parseFromPosition ( query , pos+what.size() , e.sub[e.sub.size()-1] ) ;
        }

        if ( !numeric.empty() && isValidNumeric(c,numeric) ) {
            numeric += c ;
            pos++ ;
            continue ;
        } else addNewNumeric(numeric,e) ;

        if ( isValidVariableChar(c) ) {
            variable_name += c ;
            pos++ ;
            continue ;
        } else addNewVariable(variable_name,e) ;

        if ( isValidNumeric(c,numeric) ) {
            numeric += c ;
            pos++ ;
            continue ;
        } else addNewNumeric(numeric,e) ;

        if ( c == '(' ) {
            e.sub.push_back ( ConditionParserElement ( ConditionParserElementType::SUB ) ) ;
            pos = parseFromPosition ( query , pos+1 , e.sub[e.sub.size()-1] ) ;
            if ( pos == -1 ) return pos ; // Error
            continue ;
        }

        if ( c == ')' ) {
            return pos+1 ;
        }

        if ( c == ',' ) {
            e.sub.push_back ( ConditionParserElement ( ConditionParserElementType::ENUM , "," ) ) ;
        	pos++ ;
        	continue ;
        }

        bool found = false ;
        for ( auto &what:comparison ) {
            if ( query.substr(pos,what.size()) != what ) continue ;
            e.sub.push_back ( ConditionParserElement ( ConditionParserElementType::COMPARISON , what ) ) ;
            pos += what.size() ;
            found = true ;
        }
        if ( found ) continue ;

        return pos ;
    }
    addNewVariable(variable_name,e) ;
    addNewNumeric(numeric,e) ;
    if ( in_quote ) {
        cout << "ERROR: Unclosed quote in " << query << endl ;
        return -1 ;
    }
    return pos ;
}
