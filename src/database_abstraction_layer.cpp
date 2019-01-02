#include "database_abstraction_layer.h"
#include <algorithm>

string Note::getID ( FileTrackingDatabase &ft ) {
    if ( id > 0 ) return i2s(id) ; // Cached
    setCurrentDay() ;

    string sql = "SELECT * FROM `note` WHERE `source`="+ft.quote(source)+" AND `rationale`="+ft.quote(rationale)+" AND `day`="+ft.quote(day) ;
    SQLmap datamap ;
    SQLresult r = ft.query ( sql ) ;
    while ( r.getMap(datamap) ) {
        id = datamap["id"].asInt() ;
        return i2s(id) ;
    }

    sql = "INSERT IGNORE INTO `note` (`source`,`rationale`,`day`) VALUES ("+ft.quote(source)+","+ft.quote(rationale)+","+ft.quote(day)+")" ;
    ft.exec ( sql ) ;
    id = ft.getLastInsertID() ;
    return i2s(id) ;
}

void Note::setCurrentDay() {
    if ( !day.empty() ) return ;
    time_t timer;
    char buffer[26];
    struct tm* tm_info;
    time(&timer);
    tm_info = localtime(&timer);
    strftime(buffer, 26, "%Y-%m-%d", tm_info);
    day = buffer ;
}

//______

DatabaseAbstractionLayer::DatabaseAbstractionLayer () {
    loadTags() ;
}

string DatabaseAbstractionLayer::sanitizeTagName ( string tag_name ) {
    std::transform(tag_name.begin(), tag_name.end(), tag_name.begin(), ::tolower);
    for ( size_t p = 0 ; p < tag_name.size() ; p++ ) {
        if ( tag_name[p] == ' ' ) tag_name[p] = '_' ;
    }
    return tag_name ;
}

db_id DatabaseAbstractionLayer::getTagID ( string tag_name ) {
    loadTags() ;
    tag_name = sanitizeTagName ( tag_name ) ;
    if ( isNumeric(tag_name) ) return s2i(tag_name) ;
    for ( auto &tag:tags ) {
        if ( tag.name == tag_name ) return tag.id ;
    }
    return 0 ;
}

void DatabaseAbstractionLayer::loadTags() {
    if ( !tags.empty() ) return ;

    string sql = "SELECT * FROM tag" ;

    SQLresult r = ft.query ( sql ) ;

    SQLmap map ;
    while ( r.getMap(map) ) {
        DALtag tag ;
        tag.id = map["id"].asInt() ;
        tag.name = sanitizeTagName(map["name"]) ;
        tag.type = map["type"] ;
        tag.note = map["note"] ;
        tags.push_back ( tag ) ;
    }
}

bool DatabaseAbstractionLayer::tableHasTag ( string table , string id , string tag_name , string value ) {
    db_id tag_id = getTagID(tag_name) ;
    if ( tag_id == 0 ) return false ;

    string sql = "SELECT * FROM " + table + " WHERE `tag_id`=" + i2s(tag_id) + " AND `" + getMainID4table(table) + "`=" + ft.quote(id) ;
    if ( !value.empty() ) sql += " AND `value`=" + ft.quote(value) ;
    SQLresult r = ft.query ( sql ) ;
    SQLmap map ;
    if ( r.getMap(map) ) return true ;
    return false ;
}

vector <string> DatabaseAbstractionLayer::getIDsForTag ( string table , string tag , string value ) {
    vector <string> ret ;
    db_id tag_id = getTagID ( tag ) ;
    if ( tag_id == 0 ) return ret ;

    string field_name = getMainID4table ( table ) ;
    string sql = "SELECT DISTINCT `" + field_name + "` FROM `"+table+"` WHERE tag_id="+i2s(tag_id) ;
    if ( !value.empty() ) sql += " AND `value`=" + ft.quote(value) ;
    SQLmap map ;
    SQLresult r = ft.query ( sql ) ;
    if ( r.getMap(map) ) {
        ret.push_back ( map[field_name] ) ;
    }
    return ret ;
}

bool DatabaseAbstractionLayer::setSampleFile ( db_id sample_id , db_id file_id , Note note ) {
    string sql = "INSERT IGNORE INTO sample2file (sample_id,file_id,note_id) VALUES (" + i2s(sample_id) + "," + i2s(file_id) + "," + note.getID(ft) + ")" ;
    ft.exec ( sql ) ;
    return true ;
}

bool DatabaseAbstractionLayer::setTableTag ( string table , string id , string tag , string value , Note note ) {
    db_id tag_id = getTagID ( tag ) ;
    if ( tag_id == 0 ) return false ;

    string field_name = getMainID4table ( table ) ;
    string sql = "INSERT IGNORE INTO `" + table + "` (" + field_name + ",tag_id,value,note_id" ;
    sql += ") VALUES (" + ft.quote(id) + "," + ft.quote(tag_id) + "," + ft.quote(value) ;
    sql += "," + note.getID(ft) ;
    sql += ")" ;

//if ( table == "file2tag" ) { /* cout << sql << endl ; */ return true ; } // TESTING FIXME

    ft.exec ( sql ) ;
    return true ;
}

string DatabaseAbstractionLayer::getMainID4table ( const string &table ) {
    return (table=="file2tag")?"file_id":"sample_id" ;
}

vector <string> DatabaseAbstractionLayer::getSamplesForFile ( string file_id ) {
    vector <string> ret ;
    string sql = "SELECT sample_id FROM sample2file WHERE file_id=" + file_id ;
    SQLresult r ;
    SQLmap datamap ;
    r = ft.query ( sql ) ;
    while ( r.getMap(datamap) ) {
        string sample_id = datamap["sample_id"].asString() ;
        ret.push_back ( sample_id ) ;
    }
    return ret ;
}

db_id DatabaseAbstractionLayer::doGetFileID ( string full_path , string filename , db_id storage , Note note , bool create_if_missing ) {
    string sql = "SELECT * FROM file WHERE `storage`=" + i2s(storage) + " AND `full_path`=" + ft.quote(full_path) + " LIMIT 1" ;
    SQLresult r ;
    r = ft.query ( sql ) ;

    SQLmap map ;
    if ( r.getMap(map) ) {
//        cout << "FOUND FILE " <<  map["id"] << endl ;
        return map["id"].asInt() ;
    }
    if ( !create_if_missing ) return 0 ;

    string timestamp = getCurrentTimestamp() ;
    sql = "INSERT IGNORE INTO file (filename,full_path,ts_created,ts_touched,storage,note_id) VALUES (" +
        ft.quote(filename) + "," +
        ft.quote(full_path) + "," +
        ft.quote(timestamp) + "," +
        ft.quote(timestamp) + "," +
        ft.quote(storage) + "," +
        note.getID(ft) + ")" ;
//cout << sql << endl ;
    try {
        ft.exec ( sql ) ;
    } catch ( ... ) {
        cerr << "SQL query\n\t" << sql << "\n failed" << endl ;
        cerr << ft.getErrorString() << endl ;
        return 0 ;
    }

    db_id ret = ft.getLastInsertID() ;
//cout << "=> " << ret << endl << endl ;
    return ret ;
}

string DatabaseAbstractionLayer::getKV ( string key , string default_value ) {
    string sql = "SELECT `kv_value` FROM `kv` WHERE `kv_key`=" + ft.quote(key) ;
    SQLresult r ;
    r = ft.query ( sql ) ;
    SQLmap map ;
    if ( r.getMap(map) ) return map["kv_value"].asString() ;
    return default_value ;
}

void DatabaseAbstractionLayer::setKV ( string key , string value ) {
    if ( key.empty() ) return ; // Paranoia
    string sql = "REPLACE INTO kv (kv_key,kv_value) VALUES (" + ft.quote(key) + "," + ft.quote(value) + ")" ;
    ft.exec ( sql ) ;
}

string DatabaseAbstractionLayer::createNewSample ( string name , Note &note ) {
    string sql = "INSERT INTO sample (name,note_id) VALUES (" + ft.quote(name) + "," + note.getID(ft) + ")" ;
    ft.exec ( sql ) ;
    return i2s ( ft.getLastInsertID() ) ;
}
