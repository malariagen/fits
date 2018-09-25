#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <mysql.h>
#include <errmsg.h>

#include <memory>
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

using namespace std;

class SQLfield {
public:
	SQLfield () {}
	SQLfield ( const string &s ) : s(s) { is_null = false ; }
	bool isNull() { return is_null ; }
	long int asInt() ;
    string asString () { return s ; }

    operator string() const { return s; }

	friend ostream& operator<<(ostream& os, const SQLfield& field);  
    operator string () { return asString() ; }

private:
	string s ;
	bool is_null = true ;
} ;

typedef vector <SQLfield> SQLrow ;
typedef map <std::string,SQLfield> SQLmap ;

class SQLresult {
public:
    SQLresult () {} ;
    SQLresult ( const std::shared_ptr <MYSQL> &db , MYSQL_RES *raw_result ) ;
    my_ulonglong getNumRows() { return num_rows ; }
    my_ulonglong getNumFields() { return num_fields ; }
    bool getRow ( SQLrow &row ) ;
    bool getMap ( SQLmap &map ) ;

protected:
	void readFieldNames() ;

    std::shared_ptr <MYSQL> db ;
    std::shared_ptr <MYSQL_RES> result ;
    my_ulonglong num_rows , num_fields ;
    vector <string> field_names ;
} ;

class MysqlDatabase {
public:
    MysqlDatabase() ;
    MysqlDatabase(string config_file) ;
    virtual ~MysqlDatabase() ;

    string quote ( string s ) ;
    string quote ( int32_t i ) ;
    SQLresult query ( string sql , bool allow_fail = false ) ;
    void exec ( string sql , bool allow_fail = false ) ;
    string getErrorString() ;
    my_ulonglong getLastInsertID () ;

protected:
    void connect2DB() ;
    void loadConfigFile ( string path ) ;
    void parseConfigFile(std::istream & cfgfile) ;

    std::shared_ptr <MYSQL> db ;
    map <std::string,std::string> options ;
} ;

class FileTrackingDatabase : public MysqlDatabase {
public:
    FileTrackingDatabase() : MysqlDatabase ( "fits.conf" ) {} ;
} ;

class MultiLimsWarehouseDatabase : public MysqlDatabase {
public:
    MultiLimsWarehouseDatabase() : MysqlDatabase ( "mlwh.conf" ) {} ;
} ;

class SubtrackDatabase : public MysqlDatabase {
public:
    SubtrackDatabase() : MysqlDatabase ( "subtrack.conf" ) {} ;
} ;

#endif
