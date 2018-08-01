#include "command_list.h"


CommandLineParameterParser::CommandLineParameterParser (int argc, char *argv[]) {
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

int32_t CommandLineParameterParser::getParameterID ( string key ) {
    for ( uint32_t p = 0 ; p < parameters.size() ; p++ ) {
        if ( parameters[p].key == key ) return (int32_t) p ;
    }
    return -1 ;
}


//________________________________________________________________________________________________________________________

string CommandList::constructSQLconditions ( const ConditionParserElement &e ) {
    string ret ;
    if ( e.type == ConditionParserElementType::ROOT || e.type == ConditionParserElementType::SUB ) {
        if ( e.sub.size() == 3 && e.sub[1].type == ConditionParserElementType::LOGICAL ) {
            ret = "(" ;
            ret += constructSQLconditions ( e.sub[0] ) ;
            ret += ") " + e.sub[1].s + " (" ;
            ret += constructSQLconditions ( e.sub[2] ) ;
            ret += ")" ;
        } else {
            for ( auto &se:e.sub ) {
                if ( !ret.empty() ) ret += "," ;
                ret += constructSQLconditions ( se ) ;
            }
        }
    } else if ( e.type == ConditionParserElementType::COMPARISON ) {
    } else if ( e.type == ConditionParserElementType::VARIABLE ) {
        db_id tag = dab.getTagID ( e.s ) ;
        if ( tag == 0 ) {
            cerr << "Unknown tag " << e.s << endl ;
            exit ( -1 ) ;
        }
        char tmp2[100] ;
        sprintf ( tmp2 , "(tag_id=%d)" , tag ) ;
        ret = tmp2 ;
    } else if ( e.type == ConditionParserElementType::NUMERIC ) {
        ret = e.s ;
//    } else if ( e.type == ConditionParserElementType::LOGICAL ) {
    } else if ( e.type == ConditionParserElementType::ENUM ) {

    } else if ( e.type == ConditionParserElementType::CONSTANT ) {
        ret = "'" + e.s + "'" ; // TODO escape
    } else {
        cerr << "Unknown element type " << (int) e.type << endl ;
        exit ( -1 ) ;
    }
    return ret ;
}

string CommandList::constructSQLfromQuery ( const ConditionParser &cp ) {
    string sql ;
    vector <string> where ;

    where.push_back ( "sample.id=sample2file.sample_id AND file.id=sample2file.file_id" ) ;
    where.push_back ( constructSQLconditions ( cp.root ) ) ;

    sql = "SELECT * FROM sample,file,sample2file WHERE (" ;
    for ( auto &w:where ) {
        sql += ") AND (" + w ;
    }
    sql += ")" ;
    return sql ;
}

bool CommandList::parse ( CommandLineParameterParser &cpl ) {
    string sql ;

    for ( auto &p:cpl.parameters ) {
        if ( p.key == "-q" || p.key == "--query" ) {
            ConditionParser cp ;
            if ( !cp.parse ( p.value ) ) {
                return false ;
            }
            sql = constructSQLfromQuery ( cp ) ;
        }
    }

    if ( sql.empty() ) return throwError ( "No query (-q or --query)" ) ;

    cout << sql << endl ;

    return true ;
}

bool CommandList::run () {
    return true ;
}
