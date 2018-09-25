#include "database.h"
#include <iostream>
#include <fstream>
#include <sstream>

bool operator ==(const string s, SQLfield& field) {
    return field.asString() == s ;
}

bool operator !=(const string s, SQLfield& field) {
    return field.asString() != s ;
}

ostream& operator<<(ostream& os, const SQLfield& field) {
    if ( field.is_null ) os << "NULL" ;
    else os << field.s ;
    return os ;
}

long int SQLfield::asInt() {
    if ( is_null ) return 0 ;
    return atoi ( s.c_str() ) ;
}

// ________________________________________________________________________________________________________________________________________________________________________________________________

MysqlDatabase::MysqlDatabase () {
    db = make_shared <MYSQL> () ;
    cerr << "MysqlDatabase constructor called without parameters\n" ;
    exit(-1) ;
}

MysqlDatabase::MysqlDatabase(string config_file) {
    db = make_shared <MYSQL> () ;
    loadConfigFile ( config_file ) ;
    mysql_init(db.get());
    connect2DB() ;
}

MysqlDatabase::~MysqlDatabase () {
    mysql_close(db.get());
}

void MysqlDatabase::connect2DB () {
    long int port = atol(options["port"].c_str()) ;
    mysql_real_connect(db.get(),
        options["host"].c_str(),
        options["user"].c_str(),
        options["password"].c_str(),
        options["schema"].c_str(),
        port,
        NULL,
        CLIENT_REMEMBER_OPTIONS
    );
}

string MysqlDatabase::getErrorString() {
    return string ( mysql_error ( db.get() ) ) ;
};

void MysqlDatabase::loadConfigFile ( string path ) {
    ifstream ifs ( path , std::ifstream::in ) ;
    parseConfigFile ( ifs ) ;
}

void MysqlDatabase::parseConfigFile(std::istream & cfgfile) {
    for (std::string line; std::getline(cfgfile, line); ) {
        std::istringstream iss(line);
        std::string id, eq, val;
        bool error = false;

        if (!(iss >> id)) error = true;
        else if (id[0] == '#') continue;
        else if (!(iss >> eq >> val >> std::ws) || eq != "=" || iss.get() != EOF) error = true;

        if (error) throw "Bad config file" ;
        else options[id] = val;
    }
}

SQLresult MysqlDatabase::query ( string sql , bool allow_fail ) {
    MYSQL_RES *result ;
    try {
        while ( 1 ) {
            int error = mysql_query ( db.get() , sql.c_str() ) ;
            if ( error == 0 ) break ; // OK
            if ( error == CR_SERVER_GONE_ERROR || error == CR_SERVER_LOST ) {
                cerr << "Database connection lost, reconnecting..." << endl ;
                connect2DB() ;
            } else throw "MySQL error #"+quote(error)+" for: "+sql+string(" [")+string(mysql_error(db.get()))+"]" ;
        }

        result = mysql_store_result(db.get()) ;
    } catch ( ... ) {
        if ( !allow_fail ) {
            cerr << "SQL query\n\t" << sql << "\n failed" << endl ;
            cerr << getErrorString() << endl ;
            exit(0) ;
        }
    }
    return SQLresult ( db , result ) ;
}

void MysqlDatabase::exec ( string sql , bool allow_fail ) {
    try {
        while ( 1 ) {
            int error = mysql_query ( db.get() , sql.c_str() ) ;
            if ( error == 0 ) break ; // OK
            if ( error == CR_SERVER_GONE_ERROR || error == CR_SERVER_LOST ) {
                cerr << "Database connection lost, reconnecting..." << endl ;
                connect2DB() ;
            } else throw "MySQL error #"+quote(error)+" for: "+sql+string(" [")+string(mysql_error(db.get()))+"]" ;
        }
    } catch ( ... ) {
        if ( !allow_fail ) {
            cerr << "SQL query\n\t" << sql << "\n failed" << endl ;
            cerr << getErrorString() << endl ;
            exit(0) ;
        }
    }
}

string MysqlDatabase::quote ( string s ) {
    char *tmp = new char [s.length()*2+1] ;
    mysql_real_escape_string_quote ( db.get() , tmp , s.c_str() , s.length() , '\'' ) ;
    string ret = "'" ;
    ret += tmp ;
    ret += "'" ;
    delete tmp ;
    return ret ;
}

string MysqlDatabase::quote ( int32_t i ) {
    char s[100] ;
    sprintf ( s , "%d" , i ) ;
    return string(s) ;
}

my_ulonglong MysqlDatabase::getLastInsertID () {
    // Alternative: "SELECT LAST_INSERT_ID() AS id"
   return mysql_insert_id ( db.get() ) ;
}

// ________________________________________________________________________________________________________________________________________________________________________________________________

struct MySQLResultDeleter { 
    void operator()(MYSQL_RES *p) const {
        mysql_free_result(p) ;
    }
};

SQLresult::SQLresult ( const std::shared_ptr <MYSQL> &db , MYSQL_RES *raw_result ) : db(db),result(raw_result,MySQLResultDeleter()) {
    if ( !result ) {
        num_fields = mysql_field_count( db.get() ) ;
        if(num_fields == 0) {
            num_rows = mysql_affected_rows( db.get() );
        } else {
            throw "" ;
        }
    } else {
        num_fields = mysql_field_count(db.get()) ;
        num_rows = mysql_num_rows(result.get()) ;
    }
}

bool SQLresult::getRow ( SQLrow &row ) {
    if ( !result ) return false ; // Nothing to get
    if ( num_fields == 0 ) return false ; // No fields
    MYSQL_ROW tmp;
    if ( !(tmp = mysql_fetch_row(result.get())) ) { // No more rows
        return false ;
    }
    row.clear() ;
    row.reserve ( num_fields ) ;
    for ( my_ulonglong i = 0 ; i < num_fields ; i++ ) {
        row.push_back ( tmp[i] ? SQLfield(tmp[i]) : SQLfield() ) ;
    }
    return true ;
}

bool SQLresult::getMap ( SQLmap &map ) {
    vector <SQLfield> row ;
    if ( !getRow(row) ) return false ;
    readFieldNames() ;
    if ( row.size() != field_names.size() ) throw "Row/field disparity\n" ;
    map.clear() ;
    for ( uint32_t i = 0 ; i < field_names.size() ; i++ ) map[field_names[i]] = row[i] ;
    return true ;
}

void SQLresult::readFieldNames() {
    if ( !result ) return ; // No result, no fields
    if ( !field_names.empty() ) return ; // Already done

    MYSQL_FIELD *field;
    while((field = mysql_fetch_field(result.get()))) {
        field_names.push_back ( field->name ) ;
    }
}
