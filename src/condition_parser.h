#ifndef __CONDITION_PARSER_H__
#define __CONDITION_PARSER_H__

#include <string>
#include <vector>

using namespace std ;

enum class ConditionParserElementType {ROOT,COMPARISON,VARIABLE,CONSTANT,NUMERIC,LOGICAL,SUB,ENUM} ;

class ConditionParserElement {
public:
    ConditionParserElement ( ConditionParserElementType type = ConditionParserElementType::ROOT , string s = "" ) : type(type) , s(s) {}
    void out ( int indent = 0 ) ;
    void fixup () ;
    vector <ConditionParserElement> sub ;
    ConditionParserElementType type ;
    string s ;
} ;

class ConditionParser {
public :
    bool parse ( string query ) ;
    ConditionParserElement root ;

private:
    static const vector <string> comparison , logical ;
    int32_t parseFromPosition ( const string &query , int32_t pos , ConditionParserElement &e ) ;
    bool isValidVariableChar ( char c ) ;
    bool isValidNumeric ( char c , const string &numeric ) ;
    void addNewVariable ( string &variable_name , ConditionParserElement &e ) ;
    void addNewNumeric ( string &numeric , ConditionParserElement &e ) ;
} ;


#endif