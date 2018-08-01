#include "database_abstraction_layer.h"
#include <algorithm>


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

bool DatabaseAbstractionLayer::setSampleFile ( db_id sample_id , db_id file_id , string note ) {
    string sql = "INSERT IGNORE INTO sample2file (sample_id,file_id,note) VALUES (" + i2s(sample_id) + "," + i2s(file_id) + "," + ft.quote(note) + ")" ;
    ft.exec ( sql ) ;
    return true ;
}

bool DatabaseAbstractionLayer::setTableTag ( string table , string id , string tag , string value , string note ) {
    db_id tag_id = getTagID ( tag ) ;
    if ( tag_id == 0 ) return false ;

    string field_name = getMainID4table ( table ) ;
    string sql = "INSERT IGNORE INTO `" + table + "` (" + field_name + ",tag_id,value" ;
    if ( !note.empty() ) sql += ",note" ;
    sql += ") VALUES (" + ft.quote(id) + "," + ft.quote(tag_id) + "," + ft.quote(value) ;
    if ( !note.empty() ) sql += "," + ft.quote(note) ;
    sql += ")" ;

//if ( table == "file2tag" ) { /* cout << sql << endl ; */ return true ; } // TESTING FIXME

    ft.exec ( sql ) ;
    return true ;
}

string DatabaseAbstractionLayer::getMainID4table ( const string &table ) {
    return (table=="file2tag")?"file_id":"sample_id" ;
}

db_id DatabaseAbstractionLayer::doGetFileID ( string full_path , string filename , db_id storage , bool create_if_missing ) {
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
    sql = "INSERT IGNORE INTO file (filename,full_path,ts_created,ts_touched,storage) VALUES (" +
        ft.quote(filename) + "," +
        ft.quote(full_path) + "," +
        ft.quote(timestamp) + "," +
        ft.quote(timestamp) + "," +
        ft.quote(storage) + ")" ;
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
