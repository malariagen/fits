#include <algorithm>
#include "update_sanger.h"
#include "json.hpp"

using namespace std ;
using json = nlohmann::json;

UpdateSanger::UpdateSanger () {
}

void UpdateSanger::updateFromMLWH () {
	updateChangedFlowcellData() ;

	// Update metadata
	addLaneMetrics() ;
	addTaxonID() ;
	fixMissingMetadata() ;
	updateFromSubtrack() ;
}

// For samples with MLWH sample ID but no Sequenscape sample ID, import the latter from MLWH
void UpdateSanger::updateSequenscapeSampleIDfromMLWHsampleID () {
	string note = "Imported from mlwh.sample on " + getCurrentTimestamp() ;
	string sql = "SELECT sample_id,value AS mlwh_sample_id FROM sample2tag s1 WHERE tag_id=1362 AND s1.sample_id NOT IN (SELECT sample_id FROM sample2tag WHERE tag_id=3585)" ;
	SQLresult r ;
	SQLmap datamap ;
	query ( dab.ft , r , sql ) ;
	while ( r.getMap(datamap) ) {
		string fits_sample_id = datamap["sample_id"].asString() ;
		string mlwh_sample_id = datamap["mlwh_sample_id"].asString() ;
		sql = "SELECT * FROM sample WHERE id_sample_tmp=" + mlwh_sample_id + " AND id_lims='SQSCP' AND id_sample_lims!='' AND id_sample_lims IS NOT NULL" ;
		SQLresult r2 ;
		SQLmap datamap2 ;
		query ( mlwh , r2 , sql ) ;
		while ( r2.getMap(datamap2) ) {
			string sequenscape_sample_id = datamap2["id_sample_lims"].asString() ;
			dab.setSampleTag ( fits_sample_id , "sequenscape LIMS ID" , sequenscape_sample_id , note ) ;
		}
	}
}

// Adds file JSON from baton where missing
void UpdateSanger::updateMissingFileJSON () {
	string sql = "SELECT sample2file.*,(SELECT full_path FROM file WHERE id=file_id) AS full_path,(SELECT value FROM sample2tag WHERE sample2tag.sample_id=sample2file.sample_id AND tag_id=1362) AS mlwh_sample_id FROM sample2file WHERE file_id IN (SELECT id FROM file WHERE id NOT IN (SELECT DISTINCT file_id FROM file2tag WHERE tag_id=3575))" ;
	SQLresult r ;
	SQLmap datamap ;
	query ( dab.ft , r , sql ) ;
	while ( r.getMap(datamap) ) {
		string mlwh_sample_id = datamap["mlwh_sample_id"].asString() ;
		cout << datamap["sample_id"].asString() << " : " << mlwh_sample_id << endl ;
		addFilesForSampleFromBaton ( mlwh_sample_id ) ;
	}
}


void UpdateSanger::fixMissingMetadataFromFileAvus ( string missing_tag_id , string avu_key ) {
	string sql = "SELECT * FROM file2tag ft1 WHERE tag_id=3575 AND file_id NOT IN (SELECT file_id FROM file2tag WHERE tag_id=" + missing_tag_id + ") AND `value` LIKE '%\"" + avu_key + "\"%'" ;
	SQLresult r ;
	SQLmap datamap ;
	query ( dab.ft , r , sql ) ;
	while ( r.getMap(datamap) ) {
		string file_id = datamap["file_id"] ;
		json j = json::parse ( datamap["value"].asString() ) ;

		string note = "Imported from BATON on " + getCurrentTimestamp() ;
		for ( auto avu: j["file"]["avus"] ) {
			string attribute = avu["attribute"].get<std::string>() ;
			if ( attribute != avu_key ) continue ;
			string value = avu["value"].get<std::string>() ;
cout << attribute << " : " << value << endl ;
			addFileTagFromAvu ( file_id , attribute , value , note ) ;
		}
	}
}

void UpdateSanger::updateFromSubtrack () {
	// Find new ones
	string sql = "SELECT id,full_path,\
		(SELECT `value` FROM file2tag WHERE file_id=file.id AND tag_id=3578 LIMIT 1) AS run_id,\
		(SELECT `value` FROM file2tag WHERE file_id=file.id AND tag_id=3571 LIMIT 1) AS lane,\
		(SELECT `value` FROM file2tag WHERE file_id=file.id AND tag_id=3569 LIMIT 1) AS tag_id\
		FROM file \
		WHERE id NOT IN (SELECT DISTINCT file_id FROM file2tag WHERE tag_id=3607) \
		AND id IN (SELECT file_id FROM file2tag WHERE tag_id=3576 AND value IN ('bam','cram'))\
		AND full_path NOT LIKE '%_phix%' AND full_path NOT LIKE '%_human%'\
		HAVING run_id IS NOT NULL AND lane IS NOT NULL" ; // Using absence of 3607 as a marker
	SQLresult r ;
	SQLmap datamap ;
	query ( dab.ft , r , sql ) ;
	while ( r.getMap(datamap) ) {
		string file_id = datamap["id"] ;
		sql = "SELECT * FROM submission WHERE run=" + datamap["run_id"].asString() + " AND lane=" + datamap["lane"].asString() ;
		if ( datamap["tag_id"].asString() != "" ) sql += " AND mux=" + datamap["tag_id"].asString() ;
		updateFromSubtrackTables ( file_id , sql ) ;
	}

	// Update old, blank values
	sql = "SELECT file_id,`value` AS sub_id FROM file2tag ft1 WHERE tag_id=3608 AND NOT EXISTS (SELECT * FROM file2tag ft2 WHERE tag_id=3610 AND ft1.file_id=ft2.file_id)" ;
	query ( dab.ft , r , sql ) ;
	while ( r.getMap(datamap) ) {
		if ( datamap["sub_id"].asString().empty() ) continue ;
		sql = "SELECT * FROM submission WHERE id=" + datamap["sub_id"].asString() ;
		updateFromSubtrackTables ( datamap["file_id"] , sql ) ;
	}
}

void UpdateSanger::updateFromSubtrackTables ( string file_id , string sql_submission ) {
	map <string,string> tag2col = {
		{ "subtrack submission ID" , "id" } ,
		{ "ENA submission accession ID" , "ebi_sub_acc" } ,
//		{ "ENA sample accession ID" , "ebi_sample_acc" } ,
		{ "ENA study accession ID" , "ebi_study_acc" } ,
		{ "ENA experiment accession ID" , "ebi_exp_acc" } ,
		{ "ENA run accession ID" , "ebi_run_acc" }
	} ;
	SubtrackDatabase subtrack ;
	string note = "Imported from subtrack.submission on " + getCurrentTimestamp() ;
	string sql ;
	SQLresult r2 ;
	SQLmap datamap2 ;
	query ( subtrack , r2 , sql_submission ) ;
	if ( !r2.getMap(datamap2) ) return ;
	for ( auto t2c = tag2col.begin() ; t2c != tag2col.end() ; t2c++ ) {
		if ( !datamap2[t2c->second].asString().empty() ) dab.setFileTag ( file_id , t2c->first , datamap2[t2c->second].asString() , note ) ;
	}
	string submission_id = datamap2["id"].asString() ;
	string ena_sample_acc = datamap2["ebi_sample_acc"].asString() ;

	// Sample accession
	sql = "SELECT sample_id FROM sample2file WHERE file_id=" + file_id ;
	query ( dab.ft , r2 , sql ) ;
	while ( r2.getMap(datamap2) ) {
		string sample_id = datamap2["sample_id"].asString() ;
		if ( !ena_sample_acc.empty() ) dab.setSampleTag ( sample_id , "ENA sample accession ID" , ena_sample_acc , note ) ;
	}

	// File size
	sql = "SELECT * FROM files WHERE sub_id=" + submission_id + " LIMIT 1" ;
	query ( subtrack , r2 , sql ) ;
	if ( r2.getMap(datamap2) ) {
		if ( !datamap2["bytes"].asString().empty() ) dab.setFileTag ( file_id , "subtrack file size" , datamap2["bytes"].asString() , note ) ;
		if ( !datamap2["file_name"].asString().empty() ) dab.setFileTag ( file_id , "subtrack file name" , datamap2["file_name"].asString() , note ) ;
	}
}

void UpdateSanger::fixMissingMetadata () {
	updateSequenscapeSampleIDfromMLWHsampleID() ;
	updateMissingFileJSON() ;
//	fixMissingMetadataFromFileAvus("3588","alignment_filter") ; // Used to "backfill" from cached JSON iRODs data, usually as one-off
	fixMissingMetadataForTag ( "sequenscape_sample_name" , "sanger_sample_id" ) ; // Missing Sequenscape sample name
	fixMissingMetadataForTag ( "MLWH taxon ID" , "taxon_id" ) ; // Missing taxon id

	SQLresult r ;
	SQLmap datamap ;
	string sql ;

	// Update sequenscape study data from MLWH
	sql = "SELECT sample_id,`value` FROM sample2tag s1 WHERE s1.tag_id=1362 AND s1.sample_id NOT IN (select DISTINCT s2.sample_id FROM sample2tag s2 WHERE s2.tag_id=3594)" ;
	query ( dab.ft , r , sql ) ;
	while ( r.getMap(datamap) ) {
		db_id fits_sample_id = datamap["sample_id"].asInt() ;
		SQLresult r2 ;
		SQLmap datamap2 ;
		sql = "SELECT study.* FROM study,iseq_flowcell WHERE study.id_lims='SQSCP' AND id_sample_tmp=" + datamap["value"].asString() + " AND iseq_flowcell.id_study_tmp=study.id_study_tmp LIMIT 1" ; // Only one study, one hopes
		query ( mlwh , r2 , sql ) ;
		while ( r2.getMap(datamap2) ) {
			dab.setSampleTag ( fits_sample_id , "MLW study ID" , datamap2["id_study_tmp"] ) ;
			dab.setSampleTag ( fits_sample_id , "sequenscape study ID" , datamap2["id_study_lims"] ) ;
			dab.setSampleTag ( fits_sample_id , "sequenscape study name" , datamap2["name"] ) ;
		}
	}

	// Add sequenscape study name as Alfresco study where it fits the pattern
	sql = "INSERT IGNORE INTO sample2tag (sample_id,tag_id,`value`,`note`) SELECT s1.sample_id,3604,s1.value,'Inferred from sequenscape study name' FROM sample2tag s1 WHERE s1.tag_id=3594 AND s1.sample_id NOT IN (select DISTINCT s2.sample_id FROM sample2tag s2 WHERE s2.tag_id=3604) AND s1.value REGEXP '^[0-9][0-9][0-9][0-9]-[A-Z][A-Z]-[A-Z][A-Z]'" ;
	dab.ft.exec ( sql ) ;
}

void UpdateSanger::fixMissingMetadataForTag ( string tag_name , string mlwh_column_name ) {
	db_id tag_id = dab.getTagID ( tag_name ) ;
	SQLresult r ;
	SQLmap datamap ;
	string sql ;

	map <db_id,db_id> mlwh_sample2fits_sample ;
	sql = "select sample_id,`value` FROM sample2tag st1 WHERE st1.tag_id=1362 AND st1.sample_id NOT IN (SELECT st2.sample_id FROM sample2tag st2 WHERE st2.tag_id=" + i2s(tag_id) + ")" ;
	query ( dab.ft , r , sql ) ;
	while ( r.getMap(datamap) ) mlwh_sample2fits_sample[datamap["value"].asInt()] = datamap["sample_id"].asInt() ;
	if ( mlwh_sample2fits_sample.empty() ) return ;

	sql = "SELECT * FROM sample WHERE id_sample_tmp IN (" ;
	bool first = true ;
	for ( auto x = mlwh_sample2fits_sample.begin() ; x != mlwh_sample2fits_sample.end() ; x++ ) {
		if ( first ) first = false ;
		else sql += "," ;
		sql += i2s(x->first) ;
	}
	sql += ") AND `" + mlwh_column_name + "` IS NOT NULL" ;
	query ( mlwh , r , sql ) ;
	while ( r.getMap(datamap) ) {
		db_id mlwh_id = datamap["id_sample_tmp"].asInt() ;
		if ( mlwh_sample2fits_sample.find(mlwh_id) == mlwh_sample2fits_sample.end() ) continue ; // Paranoia
		string sample_name = datamap[mlwh_column_name] ;
		if ( sample_name.empty() ) continue ;
		db_id fits_sample_id = mlwh_sample2fits_sample[mlwh_id] ;
		dab.setSampleTag ( fits_sample_id , tag_name , sample_name ) ;
	}
}

void UpdateSanger::updatePivotView ( string table ) {
	cout << "UPDATING " << table << endl ;

	string view = "vw_pivot_" + table ;

	SQLresult r ;
	SQLmap datamap ;
	string sql ;

	vector <string> parts ;
	sql = "SELECT * FROM tag WHERE id IN (SELECT DISTINCT tag_id FROM "+table+"2tag)" ;
	query ( dab.ft , r , sql ) ;
	while ( r.getMap(datamap) ) {
		if ( !datamap["use_for_pivot"].asInt() ) continue ;
		string tag_id = datamap["id"].asString() ;
		string tag_name = datamap["name"].asString() ;
		for ( uint32_t p = 0 ; p < tag_name.size() ; p++ ) {
			if ( tag_name[p] == ' ' ) tag_name[p] = '_' ; // Space => underscore
			else if ( tag_name[p] >= 'A' && tag_name[p] <= 'Z' ) tag_name[p] = tag_name[p]-'A'+'a' ; // Lowercase
		}
		string part = "(SELECT group_concat(`value` separator '|') FROM "+table+"2tag WHERE "+table+"_id="+table+".id AND tag_id="+tag_id+" GROUP BY `tag_id`) AS " + tag_name ;
		parts.push_back ( "\n" + part ) ;
	}

	sql = "CREATE OR REPLACE VIEW `"+view+"` AS SELECT "+table+".id AS fits_"+table+"_id," ;
	if ( table == "file" ) sql += "file.full_path AS full_path," ;
	sql += implode(parts,false) + " \nFROM " + table ;
//cout << sql << endl ;
	dab.ft.exec ( sql ) ;
}

vector <string> UpdateSanger::getMLWHSampleIDsWithoutFiles () {
	vector <string> mlwh_samples_todo ;
	SQLresult r ;
	SQLmap datamap ;
	string sql = "SELECT DISTINCT value FROM vw_sample_tag WHERE tag_id=1362 AND sample_id IN (select id FROM sample WHERE NOT EXISTS (select * FROM sample2file WHERE sample_id=sample.id))" ;
	query ( dab.ft , r , sql ) ;
	while ( r.getMap(datamap) ) {
		mlwh_samples_todo.push_back ( datamap["value"] ) ;
	}
	return mlwh_samples_todo ;
}

bool UpdateSanger::isFlowcellMissingFile ( string id_iseq_flowcell_tmp ) {
	bool ret = true ; // Missing until know otherwise
	SQLresult r ;
	SQLmap datamap ;
	string sql = "SELECT * FROM id_iseq_flowcell_tmp_no_file WHERE missing_file=0 AND id=" + id_iseq_flowcell_tmp ;
	query ( dab.ft , r , sql ) ;
	if ( r.getMap(datamap) ) ret = false ; // Actually an entry with missing=0
	return ret ;
}

void UpdateSanger::updateChangedFlowcellData () {
	vector <string> id_study_tmp = getOurMLWHstudies() ;
	vector <string> last_update_timestamp = queryFirstColumn ( dab.ft , "SELECT kv_value FROM kv WHERE kv_key='mlwh_flowcell_last_update'" ) ;

	SQLresult r ;
	SQLmap datamap ;
	string sql ;

	// MLWH samples that don't have files in FITS
	vector <string> mlwh_samples_todo = getMLWHSampleIDsWithoutFiles() ;

	// Flowcells without files
	vector <string> flowcells_without_files ;
	sql = "SELECT id FROM id_iseq_flowcell_tmp_no_file WHERE missing_file=1" ;
	query ( dab.ft , r , sql ) ;
	while ( r.getMap(datamap) ) {
		flowcells_without_files.push_back ( datamap["id"] ) ;
	}

	// Get all samples that had an updated flowcell entry (since last timestamp, if available)
	map <string,string> flowcell2run ;
	map <string,string> mlwh_sample2mlwh_study ;
	vector <SQLmap> all_flowcells ;
	int sql_parts = 0 ;
	sql = "SELECT * FROM iseq_flowcell WHERE " ;
	if ( !id_study_tmp.empty() ) {
		if ( sql_parts++ > 0 ) sql += " OR " ;
		sql += " ( id_study_tmp IN (" + implode(id_study_tmp,false) + ")" ;
		sql += " AND id_lims='SQSCP'" ; // Sequenscape only, for now
		if ( !last_update_timestamp.empty() ) sql += " AND last_updated>='" + last_update_timestamp[0] + "'" ;
		sql += ")" ;
	}
	if ( mlwh_samples_todo.size() > 0 ) {
		if ( sql_parts++ > 0 ) sql += " OR " ;
		sql += " (id_sample_tmp IN (" + implode(mlwh_samples_todo,false) + "))" ;
	}
	if ( flowcells_without_files.size() > 0 ) {
		if ( sql_parts++ > 0 ) sql += " OR " ;
		sql += " (id_iseq_flowcell_tmp IN (" + implode(flowcells_without_files,false) + "))" ;
	}
	if ( sql_parts == 0 ) return ; // Nothing to do
	sql += " ORDER BY last_updated" ;
//cout << sql << endl ; exit ( 0 ) ;

	// Get flowcells to check
	query ( mlwh , r , sql ) ;
	while ( r.getMap(datamap) ) {
		if ( !isFlowcellMissingFile(datamap["id_iseq_flowcell_tmp"].asString()) ) continue ;
		mlwh_sample2mlwh_study[datamap["id_sample_tmp"]] = datamap["id_study_tmp"] ;

		string flowcell_barcode = datamap["flowcell_barcode"] ;
		if ( flowcell2run.find(flowcell_barcode) == flowcell2run.end() ) flowcell2run[flowcell_barcode] = getRunForFlowcell ( flowcell_barcode ) ;
		datamap["run"] = flowcell2run[flowcell_barcode] ;
		all_flowcells.push_back ( datamap ) ;

		// Store flowcell ID as "missing file"
		sql = "INSERT IGNORE INTO id_iseq_flowcell_tmp_no_file (id) VALUES (" + datamap["id_iseq_flowcell_tmp"].asString() + ")" ;
		dab.ft.exec ( sql ) ;
	}
	ensureMLWHsamplesExistInFITS ( mlwh_sample2mlwh_study ) ;

	cout << "Samples : " << mlwh_sample2mlwh_study.size() << endl ;

	for ( auto s2s: mlwh_sample2mlwh_study ) {
		addFilesForSampleFromBaton ( s2s.first , &all_flowcells ) ;
	}

	string last_update_done ;
	for ( auto fc: all_flowcells ) {
		if ( last_update_done < fc["last_updated"].asString() ) last_update_done = fc["last_updated"].asString() ;
	}

	if ( !last_update_done.empty() ) {
		sql = "REPLACE INTO kv (kv_key,kv_value) VALUES ('mlwh_flowcell_last_update','" + last_update_done + "')" ;
	}

	// TODO add metadata like sequenscape sample ID to samples with MLWH ID in FITS
}

// This will run baton for a specific sequenscape sample ID, and try to add the files plus metadata to FITS. Existing files will be ignored.
void UpdateSanger::addFilesForSampleFromBaton ( string mlwh_sample_id , vector <SQLmap> *all_flowcells ) {
	string sql ;
//if(mlwh_sample_id!="3700389") return ; // TESTING
	sql = "SELECT id_sample_lims FROM sample WHERE id_sample_tmp=" + mlwh_sample_id ;
	string sequenscape_sample_id = queryFirstColumn ( mlwh , sql ) [0] ;

	db_id tag_mlwh_sample_id = dab.getTagID ( "MLW sample ID" ) ;
	sql = "SELECT sample_id FROM sample2tag WHERE tag_id=" + dab.ft.quote(tag_mlwh_sample_id) + " AND value=" + dab.ft.quote(mlwh_sample_id) ;
	db_id fits_sample_id = s2i ( queryFirstColumn ( dab.ft , sql ) [0] ) ;
//cout << mlwh_sample_id << " : " << fits_sample_id << endl ;

	string cmd = getBatonCommandForSequenscapeSample ( sequenscape_sample_id ) ;
//cout << cmd << endl ; // TESTING
	string json_text ;
	try {
		json_text = exec ( cmd ) ;
    } catch ( ... ) {
    	report ( "Baton failed for Sequenscape sample " + mlwh_sample_id + " in command " + cmd ) ;
    	exit ( -1 ) ;
    }

    json json_data ;
	try {
		json_data = json::parse ( json_text ) ;
    } catch ( ... ) {
    	report ( "Baton produced broken JSON for Sequenscape sample " + mlwh_sample_id + " in command " + cmd ) ;
    	exit ( -1 ) ;
    }

	vector <string> avu_keys = {"md5","tag_index","total_reads","manual_qc","is_paired_read","library_type"} ;
	vector <string> flowcell_keys = {"requested_insert_size_from","requested_insert_size_to","forward_read_length","reverse_read_length","flowcell_position","is_r_and_d","external_release"} ;

// done by addLaneMetric: "instrument_name" , "instrument_model" , "run_complete" , "qc_complete"

    string note = "Imported from BATON on " + getCurrentTimestamp() ;

	for ( auto entry: json_data ) {

		string filename = entry["data_object"].get<std::string>() ;
		string full_path = entry["collection"].get<std::string>() + "/" + entry["data_object"].get<std::string>() ;
		db_id file_id ;
		bool file_has_id = false ;
		bool flowcell_found = false ;
		SQLmap flowcell ;

		if ( shouldWeAddThisFile ( filename , full_path ) ) {
			file_id = dab.getOrCreateFileID ( full_path , filename , 1 ) ;
			if ( file_id == 0 ) {
				cout << "FAILED TO CREATE FILE ENTRY FOR " << full_path << " of MLWH sample " << mlwh_sample_id << endl ;
				continue ;
			}
			file_has_id = true ;
			dab.setSampleFile ( fits_sample_id , file_id , note ) ;

			dab.setFileTag ( file_id , "iRODS sequencing" ) ;
			dab.setFileTag ( file_id , "file size" , entry["size"].get<std::string>() ) ;

			string run , lane , tag ;

			for ( auto avu: entry["avus"] ) {
				string attribute = avu["attribute"].get<std::string>() ;
				string value = avu["value"].get<std::string>() ;

				if ( attribute == "id_run" ) run = value ;
				if ( attribute == "lane" ) lane = value ;
				if ( attribute == "tag_index" ) tag = value ;

				for ( auto key: avu_keys ) {
					if ( attribute == key ) dab.setFileTag ( file_id , key , value , note ) ;
				}

				addFileTagFromAvu ( i2s(file_id) , attribute , value , note , i2s(fits_sample_id) ) ;

			}

			if ( all_flowcells and !run.empty() and !lane.empty() ) {
	//			cout << "Checking flowcells for " << run << "/" << lane << "/" << tag << endl ;
				for ( auto fc: *all_flowcells ) {
					if ( run != fc["run"].asString() ) continue ;
					if ( lane != fc["position"].asString() ) continue ;
					if ( tag != fc["tag_index"].asString() ) continue ;
					flowcell = fc ;
					flowcell_found = true ;
					fc["flowcell_position"] = lane ;
					for ( auto key:flowcell_keys ) dab.setFileTag ( file_id , key , fc[key] , note ) ;
					dab.setFileTag ( file_id , "MLW legacy library ID" , fc["legacy_library_id"] , note ) ;
					dab.setFileTag ( file_id , "sequenscape library lims ID" , fc["id_library_lims"] , note ) ;

					sql = "UPDATE id_iseq_flowcell_tmp_no_file SET missing_file=0 WHERE id=" + fc["id_iseq_flowcell_tmp"].asString() ;
					dab.ft.exec ( sql ) ;
					break ;
				}
			}
		} else {
			if ( dab.doesFileExist ( full_path , 1 ) ) {
				file_has_id = true ;
				file_id = dab.getFileID ( full_path , 1 ) ;
			}
		}

		if ( file_has_id && !dab.fileHasTag ( file_id , "iRODs JSON") ) {
			string json_info = "{\"file\":"+entry.dump() ;
			if ( flowcell_found ) {
				json run_json ;
				for(auto e : flowcell) run_json[e.first] = e.second.asString() ;
				json_info += ",\"run\":"+run_json.dump() ;
			}
			json_info += "}" ;
			dab.setFileTag ( file_id , "iRODs JSON" , json_info , note ) ;
		}

	}
}

void UpdateSanger::addFileTagFromAvu ( string file_id , string attribute , string value , string note , string fits_sample_id ) {
	if ( attribute == "type" ) dab.setFileTag ( file_id , "file type" , value , note ) ;
	else if ( attribute == "reference" ) dab.setFileTag ( file_id , "CRAM reference" , value , note ) ;
	else if ( attribute == "id_run" ) dab.setFileTag ( file_id , "sequencing run" , value , note ) ;
	else if ( attribute == "lane" ) dab.setFileTag ( file_id , "flowcell position" , value , note ) ;
	else if ( attribute == "ebi_run_acc" ) dab.setFileTag ( file_id , "ENA run accession ID" , value , note ) ;
	else if ( attribute == "library" ) dab.setFileTag ( file_id , "sequenscape library name" , value , note ) ;
	else if ( attribute == "alignment_filter" ) dab.setFileTag ( file_id , "alignment filter" , value , note ) ;
	else if ( attribute == "library_id" ) dab.setFileTag ( file_id , "MLW legacy library ID" , value , note ) ;

	else if ( attribute == "sample_common_name" || attribute == "sample_supplier_name" ) {
		dab.setFileTag ( file_id , attribute , value , note ) ;
		dab.setSampleTag ( fits_sample_id , attribute , value , note ) ;

		if ( fits_sample_id.empty() ) {
			auto samples = dab.getSamplesForFile ( file_id ) ;
			for ( auto fsid : samples ) {
				dab.setSampleTag ( fsid , attribute , value , note ) ;
			}
		} else {
			dab.setSampleTag ( fits_sample_id , attribute , value , note ) ;
		}
	}
}

void UpdateSanger::updateSampleMetadata ( db_id fits_sample_id ) {
	// TODO
}

// This will create new samples where we don't have the MLWH sample ID, with MLWH sample/study IDs in key/value, respectively
void UpdateSanger::ensureMLWHsamplesExistInFITS ( const map <string,string> &mlwh_sample2mlwh_study ) {
	SQLresult r ;
	SQLmap datamap ;
	string sql ;

	// Check which MLWH samples exist in FITS
	map <string,bool> sample_exists ;
	vector <string> mlwh_sample_ids ;
	for ( auto s2s: mlwh_sample2mlwh_study ) mlwh_sample_ids.push_back ( s2s.first ) ;
	if ( mlwh_sample_ids.empty() ) return ;
	sql = "SELECT DISTINCT `value` FROM vw_sample_tag WHERE tag_id=" + i2s(dab.getTagID("MLW sample ID")) + " AND `value` IN (" + implode(mlwh_sample_ids,true) + ")" ;
	query ( dab.ft , r , sql ) ;
	while ( r.getMap(datamap) ) sample_exists[datamap["value"]] = true ;

	// Now create new sample entries for the new ones
	uint32_t number_of_samples_created = 0 ;
	string note = "imported " + getCurrentTimestamp() ;
	for ( auto s2s: mlwh_sample2mlwh_study ) { // s2s.first: MLWH sample id ; s2s.second: MLWH study ID
		if ( sample_exists.find(s2s.first) != sample_exists.end() ) continue ; // No need to create this sample
		vector <string> sample_ids = dab.getSamplesForTag ( "MLW sample ID" , s2s.first ) ;
		if ( !sample_ids.empty() ) continue ; // Some times, this triggers. Should not, as sample_exists should already contain IDs. Huh.
		sql = "INSERT INTO sample (name) VALUES (" + dab.ft.quote("MLWH sample #"+s2s.first) + ")" ;
		query ( dab.ft , r , sql ) ;

		string fits_sample_id = i2s ( dab.ft.getLastInsertID() ) ;
		dab.setSampleTag ( fits_sample_id , "MLW sample ID" , s2s.first , note ) ;
		dab.setSampleTag ( fits_sample_id , "MLW study ID" , s2s.second , note ) ;
		number_of_samples_created++ ;
	}
	report ( i2s(number_of_samples_created) + " samples created in database" ) ;
}

void UpdateSanger::addTaxonID () {
	db_id tag_mlwh_sample_id = dab.getTagID ( "MLW sample ID" ) ;
	db_id tag_mlwh_taxon_id = dab.getTagID ( "MLWH taxon ID" ) ;

	map <string,int> mlwh2sample ;
	SQLresult r ;
	SQLmap map ;
	string sql = "SELECT DISTINCT id,(SELECT DISTINCT value FROM vw_sample_tag WHERE sample.id=sample_id AND tag_id="+dab.ft.quote(tag_mlwh_sample_id)+
		") AS mlwh_sample_id FROM sample WHERE NOT EXISTS (SELECT * FROM vw_sample_tag WHERE sample_id=sample.id AND tag_id="+dab.ft.quote(tag_mlwh_taxon_id)+")" ;
	query ( dab.ft , r , sql ) ;
	while ( r.getMap(map) ) {
		mlwh2sample[map["mlwh_sample_id"].asString()] = map["id"].asInt() ;
	}

	vector <string> mlwh_ids ;
	for ( auto &x:mlwh2sample ) {
		mlwh_ids.push_back ( x.first ) ;
	}
	if ( mlwh_ids.empty() ) return ; // Nothing to do

	sql = "SELECT id_sample_tmp,taxon_id FROM sample WHERE taxon_id IS NOT NULL AND id_sample_tmp IN (" + implode ( mlwh_ids , false ) + ")" ;
	query ( mlwh , r , sql ) ;
	while ( r.getMap(map) ) {
		string mlwh_sample_id = map["id_sample_tmp"].asString() ;
		if ( mlwh2sample.find(mlwh_sample_id) == mlwh2sample.end() ) continue ; // Paranoia
		string taxon_id = map["taxon_id"].asString() ;
		sql = "INSERT IGNORE INTO sample2tag (`sample_id`,`tag_id`,`value`) VALUES ("+dab.ft.quote(mlwh2sample[mlwh_sample_id])+","+dab.ft.quote(tag_mlwh_taxon_id)+","+dab.ft.quote(taxon_id)+")" ;
		dab.ft.exec ( sql ) ;
	}
}

void UpdateSanger::addLaneMetrics () {
// SUBQUERY RETURNS MULTIPLE RESULTS; TODO FIXME
	auto fields = { "instrument_name" , "instrument_model" , "run_complete" , "qc_complete" } ;
	SQLresult r ;
	SQLmap map ;
	string sql = "SELECT"
				" (SELECT `value` FROM vw_file_tag WHERE file_id=file.id AND tag_id=3578) AS run,"
				" (SELECT `value` FROM vw_file_tag WHERE file_id=file.id AND tag_id=3571) AS lane,"
				" group_concat(file.id) AS file_ids"
				" FROM file"
				" WHERE file.id NOT IN"
				" (SELECT file_id FROM vw_file_tag WHERE tag_id=3596)"
				" GROUP BY file.id" ;
	query ( dab.ft , r , sql ) ;
	while ( r.getMap(map) ) {
		vector <string> file_ids = split ( map["file_ids"].asString() , ',' ) ;
		int run = map["run"].asInt() ;
		int lane = map["lane"].asInt() ;
		if ( run <= 0 || lane <= 0 ) continue ; // Paranoia

		for ( auto& field_name: fields ) {
			int tag_id = dab.getTagID ( field_name ) ;
			sql = "SELECT " + string(field_name) + " FROM iseq_run_lane_metrics WHERE id_run=" + map["run"].asString() + " AND position=" + map["lane"].asString() + " LIMIT 1" ;
			vector <string> data = queryFirstColumn ( mlwh , sql ) ;
			if ( data.size() != 1 ) continue ;
			for ( auto& file_id: file_ids ) dab.setFileTag ( file_id , field_name , data[0] ) ;
		}
	}
}

vector <string> UpdateSanger::queryFirstColumn ( MysqlDatabase &db , string sql ) {
	vector <string> ret ;
    SQLresult r ;
    query ( db , r , sql ) ;
    SQLrow row ;
    while ( r.getRow(row) ) ret.push_back ( row[0] ) ;
    return ret ;
}

void UpdateSanger::query ( MysqlDatabase &db , SQLresult &r , string sql ) {
    try {
        r = db.query ( sql ) ;
    } catch ( ... ) {
        cerr << "SQL query\n\t" << sql << "\n failed" << endl ;
        cerr << db.getErrorString() << endl ;
        exit ( -1 ) ;
    }
}

string UpdateSanger::getBatonCommandForSequenscapeSample ( string sequenscape_sample_id ) {
	string cmd ( "jq -n '{collection:\"/seq\", avus: [{\"attribute\": \"sample_id\", \"value\": \"") ;
	cmd += sequenscape_sample_id ;
	cmd += "\"}]}' | baton-metaquery --silent --size --avu" ;
	return cmd ;
}

vector <string> UpdateSanger::getOurMLWHstudies() {
	vector <string> ret ;
	ret = queryFirstColumn ( mlwh , "SELECT DISTINCT id_study_tmp FROM study WHERE `faculty_sponsor` like '%Kwiatkowski%'" );
	ret.push_back ( "2104" ) ; // HARDCODED SEQCAP_WGS_Low_coverage_sequencing_of_the_Woloff_from_Gambia, homo sapiens, Richard Durbin

	std::sort ( ret.begin() , ret.end() ) ;
	std::unique ( ret.begin() , ret.end() ) ;
	return ret ;
}

void UpdateSanger::report ( string s ) {
	cout << s << endl ;
}

bool UpdateSanger::shouldWeAddThisFile ( string filename , string full_path ) {
	bool ret = true ;
	if ( full_path.find("/no_cal/") != std::string::npos ) return false ; // Hard exception
	for ( auto p = 2 ; p < filename.size() ; p++ ) {
		if ( filename[p-2]=='#' && filename[p-1]=='0' && (filename[p]=='_'||filename[p]=='.') ) ret = false ; // "#0" file, do not add
	}
	return ret ;
}

string UpdateSanger::getRunForFlowcell ( string flowcell_barcode ) {
	string sql = "SELECT id_run FROM iseq_run_lane_metrics WHERE flowcell_barcode=" + dab.ft.quote(flowcell_barcode) ;
	vector <string> results = queryFirstColumn ( mlwh , sql ) ;
	if ( !results.empty() ) return results[0] ;
//cout << "Can't find a run for " << flowcell_barcode << endl ;
	return "" ;
}
