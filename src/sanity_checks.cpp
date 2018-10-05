#include "sanity_checks.h"

void SanityChecks::checkTagCounts ( string table , string tag_id , string tag_name , string min , string max ) {
    if ( min.empty() && max.empty() ) return ;

    string sql = "SELECT `"+table+"_id`,count(*) AS cnt FROM `"+table+"2tag` WHERE `tag_id`="+tag_id+" GROUP BY `"+table+"_id` HAVING" ;
    if ( !min.empty() ) {
        sql += " cnt<"+min ;
    }
    if ( !max.empty() ) {
        if ( !min.empty() ) sql += " OR" ;
        sql += " cnt>"+max ;
    }

    vector <string> ids ;
    SQLresult r ;
    SQLmap datamap ;
    r = dab.ft.query ( sql ) ;
    while ( r.getMap(datamap) ) {
        ids.push_back ( datamap[table+"_id"].asString() ) ;
    }
    if ( ids.empty() ) return ; // All good

    cout << "There are " << ids.size() << " " << table << "s that have tags for " << tag_id << " [" << tag_name << "] and violate " << min << "<X<" << max << endl ;
    cout << "Query:" << endl << sql << endl << endl ;
}

SanityChecks::SanityChecks () {
    string sql ;
    SQLresult r ;
    SQLmap datamap ;

    vector <SQLmap> tags ;
    sql = "SELECT * FROM `tag`" ;
    r = dab.ft.query ( sql ) ;
    while ( r.getMap(datamap) ) {
        tags.push_back ( datamap ) ;
    }

    for ( auto tag = tags.begin() ; tag != tags.end() ; tag++ ) {
        checkTagCounts ( "sample" , (*tag)["id"].asString() , (*tag)["name"].asString() , (*tag)["min_sample"].asString() , (*tag)["max_sample"].asString() ) ;
        checkTagCounts ( "file" , (*tag)["id"].asString() , (*tag)["name"].asString() , (*tag)["min_file"].asString() , (*tag)["max_file"].asString() ) ;
    }
}