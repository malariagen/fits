#ifndef __UPDATE_SANGER_H__
#define __UPDATE_SANGER_H__

#include "tools.h"
#include "database_abstraction_layer.h"


class UpdateSanger : public StringTools {
public:
	UpdateSanger() ;
	void updateFromMLWH () ;
	void updatePivotView ( string table ) ;

protected:
	void report ( string s ) ;
	vector <string> getOurMLWHstudies() ;
//	void processFlowcell ( SQLmap &map ) ;
	string getBatonCommandForSequenscapeSample ( string sequenscape_sample_id ) ;
	void addLaneMetrics () ;
	void addTaxonID () ;
	void updateFromSubtrack () ;
	void updateFromSubtrackTables ( string file_id , string sql_submission ) ;
	void fixMissingMetadata () ;
	void fixMissingMetadataForTag ( string tag_name , string mlwh_column_name ) ;
	void fixMissingMetadataFromFileAvus ( string missing_tag_id , string avu_key ) ;
	void addFileTagFromAvu ( string file_id , string attribute , string value , Note note , string fits_sample_id = "" ) ;
	void updateMissingFileJSON () ;
	void updateSequenscapeSampleIDfromMLWHsampleID () ;
	void updateChangedFlowcellData () ;
	void ensureMLWHsamplesExistInFITS ( const map <string,string> &mlwh_sample2mlwh_study ) ;
	void addFilesForSampleFromBaton ( string mlwh_sample_id , vector <SQLmap> *all_flowcells = NULL ) ;
	bool shouldWeAddThisFile ( string filename , string full_path ) ;
	string getRunForFlowcell ( string flowcell_barcode ) ;
	void updateSampleMetadata ( db_id fits_sample_id ) ;
	vector <string> getMLWHSampleIDsWithoutFiles () ;
	bool isFlowcellMissingFile ( string id_iseq_flowcell_tmp ) ;

	vector <string> queryFirstColumn ( MysqlDatabase &db , string sql ) ;
	void query ( MysqlDatabase &db , SQLresult &r , string sql ) ;

	DatabaseAbstractionLayer dab ;
	MultiLimsWarehouseDatabase mlwh ;
} ;


#endif