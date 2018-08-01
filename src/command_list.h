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
    CommandLineParameterParser (int argc, char *argv[]) ;
    int32_t getParameterID ( string key ) ;
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