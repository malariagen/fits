#ifndef __DATABASE_ABSTRACTION_LAYER_H__
#define __DATABASE_ABSTRACTION_LAYER_H__

#include "database.h"
#include "tools.h"

typedef int db_id ;

class DALtag {
public:
    db_id id ;
    string name , type , note ;
} ;

class Note : public StringTools {
public:
    Note () {} ;
    Note ( string source , string rationale = "" , string day = "" ) : source(source),rationale(rationale),day(day) {} ;
    string getID ( FileTrackingDatabase &ft ) ;
protected:
    void setCurrentDay() ;
    string source , rationale , day ;
    db_id id = 0 ;
} ;

class DatabaseAbstractionLayer : public StringTools {
public:
    DatabaseAbstractionLayer () ;

    db_id getTagID ( string tag_name ) ;
    bool setSampleTag ( string sample_id , string tag , string value , Note note ) { return setTableTag ( "sample2tag" , sample_id , tag , value , note ) ; }
    bool setFileTag ( string file_id , string tag , string value , Note note ) { return setTableTag ( "file2tag" , file_id , tag , value , note ) ; }
    bool setSampleTag ( db_id sample_id , string tag , string value , Note note ) { return setSampleTag(i2s(sample_id),tag,value,note) ; }
    bool setFileTag ( db_id file_id , string tag , string value , Note note ) { return setFileTag(i2s(file_id),tag,value,note) ; }
    bool fileHasTag ( db_id file_id , string tag , string value = "" ) { return tableHasTag ( "file2tag" , i2s(file_id) , tag , value ) ; }
    bool sampleHasTag ( db_id sample_id , string tag , string value = "" ) { return tableHasTag ( "sample2tag" , i2s(sample_id) , tag , value ) ; }
    bool fileHasTag ( string file_id , string tag , string value = "" ) { return tableHasTag ( "file2tag" , file_id , tag , value ) ; }
    bool sampleHasTag ( string sample_id , string tag , string value = "" ) { return tableHasTag ( "sample2tag" , sample_id , tag , value ) ; }
    bool fileHasJSON ( string file_id ) ;
    bool fileHasJSON ( db_id file_id ) { return fileHasJSON ( i2s(file_id) ) ; }
    void setFileJSON ( string file_id , string json , Note note ) ;
    void setFileJSON ( db_id file_id , string json , Note note ) { setFileJSON ( i2s(file_id) , json , note ) ; }
    vector <string> getSamplesForTag ( string tag , string value = "" ) { return getIDsForTag ( "sample2tag" , tag , value ) ; }
    vector <string> getFilesForTag ( string tag , string value = "" ) { return getIDsForTag ( "file2tag" , tag , value ) ; }
    bool setSampleFile ( db_id sample_id , db_id file_id , Note note ) ;
    string createNewSample ( string name , Note &note ) ;

    bool doesFileExist ( string full_path , db_id storage ) { return (getFileID(full_path,storage)) != 0 ; }
    db_id getFileID ( string full_path , db_id storage ) { Note note ; return doGetFileID ( full_path , "" , storage , note , false ) ; }
    db_id getOrCreateFileID ( string full_path , string filename , db_id storage , Note note ) { return doGetFileID ( full_path , filename , storage , note , true ) ; }
    vector <string> getSamplesForFile ( string file_id ) ;

    string getKV ( string key , string default_value = "" ) ;
    void setKV ( string key , string value ) ;

    FileTrackingDatabase ft ;

private:
    void loadTags() ;
    inline string getMainID4table ( const string &table ) ;
    vector <string> getIDsForTag ( string table , string tag , string value ) ;
    bool tableHasTag ( string table , string id , string tag_name , string value ) ;
    string sanitizeTagName ( string tag_name ) ;
    bool setTableTag ( string table , string id , string tag , string value , Note note ) ;
    db_id doGetFileID ( string full_path , string filename , db_id storage , Note note , bool create_if_missing = false ) ;

    vector <DALtag> tags ;
} ;

#endif