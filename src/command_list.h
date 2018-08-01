#ifndef __COMMAND_LIST_H__
#define __COMMAND_LIST_H__

#include "condition_parser.h"
#include "database_abstraction_layer.h"


class CommandLineParameter {
public:
    CommandLineParameter ( string key , string value = "" ) : key(key),value(value) {}
    string key , value ;
} ;

class CommandLineParameterParser {
public:
    CommandLineParameterParser (int argc, char *argv[]) {
        if ( argc == 1 ) return ; // No parameters
        command = argv[1] ;
        string last_key ;
        for ( int pos = 2 ; pos < argc ; pos++ ) {
            if ( *(argv[pos]) == '-' ) {
                if ( !last_key.empty() ) parameters.push_back ( CommandLineParameter ( last_key ) ) ;
                last_key = argv[pos] ;
                continue ;
            }
            parameters.push_back ( CommandLineParameter ( last_key , argv[pos] ) ) ;
            last_key.clear() ;
        }
        if ( !last_key.empty() ) parameters.push_back ( CommandLineParameter ( last_key ) ) ;
    }
    string command ;
    vector <CommandLineParameter> parameters ;
} ;



class Command {
public:
    bool throwError ( string msg ) {
        error_message = msg ;
        return false ;
    }
    string error_message ;

protected:
    DatabaseAbstractionLayer dab ;
} ;

class CommandList : public Command {
public:
    CommandList () {}
    bool parse ( CommandLineParameterParser &cpl ) ;
    bool run () ;

private:
    string constructSQLfromQuery ( const ConditionParser &cp ) ;
    string constructSQLconditions ( const ConditionParserElement &e ) ;
} ;

#endif