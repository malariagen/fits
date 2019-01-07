# TABLES


CREATE TABLE `tag` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(255) NOT NULL DEFAULT '',
  `type` varchar(32) NOT NULL DEFAULT '',
  `note` varchar(250) DEFAULT '',
  `use_for_pivot` int(11) NOT NULL DEFAULT '1' COMMENT 'Use this tag as a column in the vw_pivot_file/vw_pivot_sample views',
  `min_sample` int(11) DEFAULT NULL COMMENT 'Every sample should have at least X of these tags',
  `max_sample` int(11) DEFAULT NULL COMMENT 'Every sample should have no more than X of these tags',
  `min_file` int(11) DEFAULT NULL COMMENT 'Every file should have at least X of these tags',
  `max_file` int(11) DEFAULT NULL COMMENT 'Every file should have no more than X of these tags',
  `avu_field` varchar(64) DEFAULT NULL COMMENT 'Equivalent field in iRODs avu structure',
  `iseq_flowcell_field` varchar(64) DEFAULT NULL COMMENT 'Equivalent field in the MLWH iseq_flowcell table',
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `note` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `source` varchar(128) NOT NULL DEFAULT '',
  `rationale` varchar(128) NOT NULL DEFAULT '',
  `day` varchar(10) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `source` (`source`,`rationale`,`day`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;


CREATE TABLE `file` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `filename` varchar(255) NOT NULL DEFAULT '' COMMENT 'A sub-string of full_path. Useful for some queries.',
  `full_path` varchar(255) NOT NULL DEFAULT '' COMMENT 'The full path of the file, in the respective storage system.',
  `ts_created` varchar(14) NOT NULL DEFAULT '' COMMENT 'Log info for when this row was created.',
  `ts_touched` varchar(14) NOT NULL DEFAULT '' COMMENT 'Log info for when this row was last changed. May be obsolete.',
  `storage` int(11) unsigned NOT NULL DEFAULT '1' COMMENT 'The tag corresponding to the storage system used. Duplicated in file2tag, here for easier querying.',
  `note_id` int(11) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `storage` (`storage`,`full_path`),
  KEY `touched` (`ts_touched`),
  KEY `ts_created` (`ts_created`),
  KEY `filename` (`filename`),
  KEY `full_path` (`full_path`),
  CONSTRAINT `file_note_1` FOREIGN KEY (`note_id`) REFERENCES `note` (`id`),
  CONSTRAINT `file_ibfk_1` FOREIGN KEY (`storage`) REFERENCES `tag` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `sample` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(127) DEFAULT NULL COMMENT 'An arbitary sample name. Not used anywhere programmatically.',
  `note_id` int(11) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  CONSTRAINT `sample_note_1` FOREIGN KEY (`note_id`) REFERENCES `note` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;


CREATE TABLE `file_relation` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `parent` int(11) unsigned NOT NULL,
  `child` int(11) unsigned NOT NULL,
  `relation` int(11) unsigned NOT NULL COMMENT 'tag ID describing the relationship',
  `note_id` int(11) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `parent` (`parent`,`child`),
  KEY `child` (`child`),
  KEY `relation` (`relation`),
  CONSTRAINT `file_relation_note_1` FOREIGN KEY (`note_id`) REFERENCES `note` (`id`),
  CONSTRAINT `file_relation_ibfk_1` FOREIGN KEY (`parent`) REFERENCES `file` (`id`),
  CONSTRAINT `file_relation_ibfk_2` FOREIGN KEY (`child`) REFERENCES `file` (`id`),
  CONSTRAINT `file_relation_ibfk_3` FOREIGN KEY (`relation`) REFERENCES `tag` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `file2tag` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `file_id` int(10) unsigned NOT NULL,
  `tag_id` int(10) unsigned NOT NULL,
  `value` varchar(128) NOT NULL DEFAULT '',
  `note_id` int(11) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `file_id_2` (`file_id`,`tag_id`,`value`(200)),
  KEY `bucket_id` (`tag_id`),
  KEY `file_id` (`file_id`,`tag_id`),
  KEY `value` (`value`(250)),
  CONSTRAINT `file2tag_note_1` FOREIGN KEY (`note_id`) REFERENCES `note` (`id`),
  CONSTRAINT `file2tag_ibfk_1` FOREIGN KEY (`file_id`) REFERENCES `file` (`id`),
  CONSTRAINT `file2tag_ibfk_2` FOREIGN KEY (`tag_id`) REFERENCES `tag` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `id_iseq_flowcell_tmp_no_file` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `missing_file` int(11) NOT NULL DEFAULT '1',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `kv` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `kv_key` varchar(128) NOT NULL DEFAULT '',
  `kv_value` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `kv_key` (`kv_key`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `logs` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `table_name` set('file2tag','sample2tag','sample2file','file_relation','file','sample') NOT NULL DEFAULT '',
  `table_id` int(11) NOT NULL COMMENT 'id of the table_name table row',
  `table_subject` int(11) NOT NULL COMMENT 'such as file_id or sample_id, depending on table_name',
  `table_subject2` int(11) NOT NULL DEFAULT 0 COMMENT 'such as file_relation.child (or 0 if not applicable)',
  `table_tag` int(11) NOT NULL COMMENT 'tag_id (or 0 if not applicable), depending on table_name',
  `action` set('INSERT','UPDATE','DELETE') NOT NULL DEFAULT '',
  `value` varchar(128) NOT NULL DEFAULT '',
  `edit_date` datetime NOT NULL,
  `edit_user` varchar(50) NOT NULL DEFAULT '',
  `note_id` int(11) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  CONSTRAINT `logs_note_1` FOREIGN KEY (`note_id`) REFERENCES `note` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='This table logs UPDATE and DELETE events on file2tag and sample2tag tables since 2018-12-18 15:30:00.\nThe contents of the log is the state of the respective table row BEFORE the UPDATE/DELETE.';

CREATE TABLE `sample2file` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `sample_id` int(10) unsigned NOT NULL,
  `file_id` int(10) unsigned NOT NULL,
  `note_id` int(11) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `sample_file` (`sample_id`,`file_id`),
  KEY `file_id` (`file_id`),
  KEY `note_id` (`note_id`),
  CONSTRAINT `sample2file_note_1` FOREIGN KEY (`note_id`) REFERENCES `note` (`id`),
  CONSTRAINT `sample2file_ibfk_1` FOREIGN KEY (`sample_id`) REFERENCES `sample` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `sample2tag` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `sample_id` int(10) unsigned NOT NULL,
  `tag_id` int(10) unsigned NOT NULL,
  `value` varchar(128) NOT NULL DEFAULT '',
  `note_id` int(11) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `sample_id_2` (`sample_id`,`tag_id`,`value`),
  KEY `bucket_id` (`tag_id`),
  KEY `sample_id` (`sample_id`,`tag_id`),
  KEY `value` (`value`),
  CONSTRAINT `sample2tag_note_1` FOREIGN KEY (`note_id`) REFERENCES `note` (`id`),
  CONSTRAINT `sample2tag_ibfk_1` FOREIGN KEY (`sample_id`) REFERENCES `sample` (`id`),
  CONSTRAINT `sample2tag_ibfk_2` FOREIGN KEY (`tag_id`) REFERENCES `tag` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;



# VIEWS

CREATE ALGORITHM=UNDEFINED DEFINER=`solaris_rw`@`%` SQL SECURITY DEFINER VIEW `vw_file_tag`
AS SELECT
   `file2tag`.`id` AS `id`,
   `file2tag`.`file_id` AS `file_id`,
   `file2tag`.`tag_id` AS `tag_id`,
   `tag`.`name` AS `name`,
   `tag`.`type` AS `type`,
   `file2tag`.`value` AS `value`,
   `note`.`id` AS `note_id`,
   `note`.`source` AS `source`,
   `note`.`rationale` AS `rationale`,
   `note`.`day` AS `day`
FROM (((`file` join `tag`) join `file2tag`) left join `note` on((`file2tag`.`note_id` = `note`.`id`))) where ((`file`.`id` = `file2tag`.`file_id`) and (`tag`.`id` = `file2tag`.`tag_id`));


CREATE ALGORITHM=UNDEFINED DEFINER=`solaris_rw`@`%` SQL SECURITY DEFINER VIEW `vw_sample_file`
AS SELECT
   `sample2file`.`id` AS `id`,
   `sample2file`.`sample_id` AS `sample_id`,
   `sample2file`.`file_id` AS `file_id`,
   `file`.`full_path` AS `full_path`,
   `note`.`id` AS `note_id`,
   `note`.`source` AS `source`,
   `note`.`rationale` AS `rationale`,
   `note`.`day` AS `day`
FROM (((`sample` join `file`) join `sample2file`) left join `note` on((`sample2file`.`note_id` = `note`.`id`))) where ((`sample`.`id` = `sample2file`.`sample_id`) and (`file`.`id` = `sample2file`.`file_id`));

CREATE ALGORITHM=UNDEFINED DEFINER=`solaris_rw`@`%` SQL SECURITY DEFINER VIEW `vw_sample_tag`
AS SELECT
   `sample2tag`.`id` AS `id`,
   `sample2tag`.`sample_id` AS `sample_id`,
   `sample2tag`.`tag_id` AS `tag_id`,
   `tag`.`name` AS `name`,
   `tag`.`type` AS `type`,
   `sample2tag`.`value` AS `value`,
   `note`.`id` AS `note_id`,
   `note`.`source` AS `source`,
   `note`.`rationale` AS `rationale`,
   `note`.`day` AS `day`
FROM (((`sample` join `tag`) join `sample2tag`) left join `note` on((`sample2tag`.`note_id` = `note`.`id`))) where ((`sample`.`id` = `sample2tag`.`sample_id`) and (`tag`.`id` = `sample2tag`.`tag_id`));



# TRIGGERS

## file2tag

delimiter $$
CREATE TRIGGER insert_file2tag AFTER INSERT ON file2tag
FOR EACH ROW 
BEGIN
DECLARE vUser varchar(50);
SELECT USER() INTO vUser;
INSERT INTO logs (table_name,table_id,table_subject,table_tag,action,value,note_id,edit_date,edit_user) 
VALUES ('file2tag',NEW.id,NEW.file_id,NEW.tag_id,'INSERT',NEW.value,NEW.note_id,SYSDATE(),vUser);
END ;$$

delimiter $$
CREATE TRIGGER update_file2tag BEFORE UPDATE ON file2tag
FOR EACH ROW 
BEGIN
DECLARE vUser varchar(50);
SELECT USER() INTO vUser;
INSERT INTO logs (table_name,table_id,table_subject,table_tag,action,value,note_id,edit_date,edit_user) 
VALUES ('file2tag',NEW.id,NEW.file_id,NEW.tag_id,'UPDATE',NEW.value,NEW.note_id,SYSDATE(),vUser);
END ;$$

delimiter $$
CREATE TRIGGER delete_file2tag BEFORE DELETE ON file2tag
FOR EACH ROW 
BEGIN
DECLARE vUser varchar(50);
SELECT USER() INTO vUser;
INSERT INTO logs (table_name,table_id,table_subject,table_tag,action,value,note_id,edit_date,edit_user) 
VALUES ('file2tag',OLD.id,OLD.file_id,OLD.tag_id,'DELETE',OLD.value,OLD.note_id,SYSDATE(),vUser);
END ;$$



## sample2tag

delimiter $$
CREATE TRIGGER insert_sample2tag AFTER INSERT ON sample2tag
FOR EACH ROW 
BEGIN
DECLARE vUser varchar(50);
SELECT USER() INTO vUser;
INSERT INTO logs (table_name,table_id,table_subject,table_tag,action,value,note_id,edit_date,edit_user) 
VALUES ('sample2tag',NEW.id,NEW.sample_id,NEW.tag_id,'INSERT',NEW.value,NEW.note_id,SYSDATE(),vUser);
END ;$$

delimiter $$
CREATE TRIGGER update_sample2tag BEFORE UPDATE ON sample2tag
FOR EACH ROW 
BEGIN
DECLARE vUser varchar(50);
SELECT USER() INTO vUser;
INSERT INTO logs (table_name,table_id,table_subject,table_tag,action,value,note_id,edit_date,edit_user) 
VALUES ('sample2tag',NEW.id,NEW.sample_id,NEW.tag_id,'UPDATE',NEW.value,NEW.note_id,SYSDATE(),vUser);
END ;$$

delimiter $$
CREATE TRIGGER delete_sample2tag BEFORE DELETE ON sample2tag
FOR EACH ROW 
BEGIN
DECLARE vUser varchar(50);
SELECT USER() INTO vUser;
INSERT INTO logs (table_name,table_id,table_subject,table_tag,action,value,note_id,edit_date,edit_user) 
VALUES ('sample2tag',OLD.id,OLD.sample_id,OLD.tag_id,'DELETE',OLD.value,OLD.note_id,SYSDATE(),vUser);
END ;$$


## sample

delimiter $$
CREATE TRIGGER insert_sample AFTER INSERT ON sample
FOR EACH ROW 
BEGIN
DECLARE vUser varchar(50);
SELECT USER() INTO vUser;
INSERT INTO logs (table_name,table_id,table_subject,table_tag,action,value,note_id,edit_date,edit_user) 
VALUES ('sample',NEW.id,0,0,'INSERT',NEW.name,NEW.note_id,SYSDATE(),vUser);
END ;$$

delimiter $$
CREATE TRIGGER update_sample BEFORE UPDATE ON sample
FOR EACH ROW 
BEGIN
DECLARE vUser varchar(50);
SELECT USER() INTO vUser;
INSERT INTO logs (table_name,table_id,table_subject,table_tag,action,value,note_id,edit_date,edit_user) 
VALUES ('sample',NEW.id,0,0,'UPDATE',NEW.name,NEW.note_id,SYSDATE(),vUser);
END ;$$

delimiter $$
CREATE TRIGGER delete_sample BEFORE DELETE ON sample
FOR EACH ROW 
BEGIN
DECLARE vUser varchar(50);
SELECT USER() INTO vUser;
INSERT INTO logs (table_name,table_id,table_subject,table_tag,action,value,note_id,edit_date,edit_user) 
VALUES ('sample',OLD.id,0,0,'DELETE',OLD.name,OLD.note_id,SYSDATE(),vUser);
END ;$$


## file

delimiter $$
CREATE TRIGGER insert_file AFTER INSERT ON file
FOR EACH ROW 
BEGIN
DECLARE vUser varchar(50);
SELECT USER() INTO vUser;
INSERT INTO logs (table_name,table_id,table_subject,table_tag,action,value,note_id,edit_date,edit_user) 
VALUES ('file',NEW.id,0,NEW.storage,'INSERT',NEW.full_path,NEW.note_id,SYSDATE(),vUser);
END ;$$

delimiter $$
CREATE TRIGGER update_file BEFORE UPDATE ON file
FOR EACH ROW 
BEGIN
DECLARE vUser varchar(50);
SELECT USER() INTO vUser;
INSERT INTO logs (table_name,table_id,table_subject,table_tag,action,value,note_id,edit_date,edit_user) 
VALUES ('file',NEW.id,0,NEW.storage,'UPDATE',NEW.full_path,NEW.note_id,SYSDATE(),vUser);
END ;$$

delimiter $$
CREATE TRIGGER delete_file BEFORE DELETE ON file
FOR EACH ROW 
BEGIN
DECLARE vUser varchar(50);
SELECT USER() INTO vUser;
INSERT INTO logs (table_name,table_id,table_subject,table_tag,action,value,note_id,edit_date,edit_user) 
VALUES ('file',OLD.id,0,OLD.storage,'DELETE',OLD.full_path,OLD.note_id,SYSDATE(),vUser);
END ;$$


## file_relation

delimiter $$
CREATE TRIGGER insert_file_relation AFTER INSERT ON file_relation
FOR EACH ROW 
BEGIN
DECLARE vUser varchar(50);
SELECT USER() INTO vUser;
INSERT INTO logs (table_name,table_id,table_subject,table_subject2,table_tag,action,value,note_id,edit_date,edit_user) 
VALUES ('file_relation',NEW.id,NEW.parent,NEW.child,NEW.relation,'INSERT',null,NEW.note_id,SYSDATE(),vUser);
END ;$$

delimiter $$
CREATE TRIGGER update_file_relation BEFORE UPDATE ON file_relation
FOR EACH ROW 
BEGIN
DECLARE vUser varchar(50);
SELECT USER() INTO vUser;
INSERT INTO logs (table_name,table_id,table_subject,table_subject2,table_tag,action,value,note_id,edit_date,edit_user) 
VALUES ('file_relation',NEW.id,NEW.parent,NEW.child,NEW.relation,'UPDATE',null,NEW.note_id,SYSDATE(),vUser);
END ;$$

delimiter $$
CREATE TRIGGER delete_file_relation BEFORE DELETE ON file_relation
FOR EACH ROW 
BEGIN
DECLARE vUser varchar(50);
SELECT USER() INTO vUser;
INSERT INTO logs (table_name,table_id,table_subject,table_subject2,table_tag,action,value,note_id,edit_date,edit_user) 
VALUES ('file_relation',OLD.id,OLD.parent,OLD.child,OLD.relation,'DELETE',null,OLD.note_id,SYSDATE(),vUser);
END ;$$



## sample2file


delimiter $$
CREATE TRIGGER insert_sample2file AFTER INSERT ON sample2file
FOR EACH ROW 
BEGIN
DECLARE vUser varchar(50);
SELECT USER() INTO vUser;
INSERT INTO logs (table_name,table_id,table_subject,table_subject2,table_tag,action,value,note_id,edit_date,edit_user) 
VALUES ('sample2file',NEW.id,NEW.sample_id,NEW.file_id,0,'INSERT',null,NEW.note_id,SYSDATE(),vUser);
END ;$$

delimiter $$
CREATE TRIGGER update_sample2file BEFORE UPDATE ON sample2file
FOR EACH ROW 
BEGIN
DECLARE vUser varchar(50);
SELECT USER() INTO vUser;
INSERT INTO logs (table_name,table_id,table_subject,table_subject2,table_tag,action,value,note_id,edit_date,edit_user) 
VALUES ('sample2file',NEW.id,NEW.sample_id,NEW.file_id,0,'UPDATE',null,NEW.note_id,SYSDATE(),vUser);
END ;$$

delimiter $$
CREATE TRIGGER delete_sample2file BEFORE DELETE ON sample2file
FOR EACH ROW 
BEGIN
DECLARE vUser varchar(50);
SELECT USER() INTO vUser;
INSERT INTO logs (table_name,table_id,table_subject,table_subject2,table_tag,action,value,note_id,edit_date,edit_user) 
VALUES ('sample2file',OLD.id,OLD.sample_id,OLD.file_id,0,'DELETE',null,OLD.note_id,SYSDATE(),vUser);
END ;$$



# TAG DATA


INSERT INTO `tag` (`id`, `name`, `type`, `note`, `use_for_pivot`, `min_sample`, `max_sample`, `min_file`, `max_file`, `avu_field`, `iseq_flowcell_field`)
VALUES
  (1,'iRODS sequencing','storage',NULL,1,NULL,NULL,NULL,NULL,NULL,NULL),
  (2,'Sanger NFS','storage',NULL,1,NULL,NULL,NULL,NULL,NULL,NULL),
  (3,'iRODS Team112','storage',NULL,1,NULL,NULL,NULL,NULL,NULL,NULL),
  (4,'ENA','storage',NULL,1,NULL,NULL,NULL,NULL,NULL,NULL),
  (7,'MD5','file metadata',NULL,1,NULL,NULL,NULL,1,'md5',NULL),
  (8,'file size','file metadata',NULL,1,NULL,NULL,NULL,1,'size',NULL),
  (347,'file creation date','file metadata',NULL,1,NULL,NULL,NULL,1,NULL,NULL),
  (1362,'MLW sample ID','sample metadata',NULL,1,NULL,1,NULL,NULL,NULL,NULL),
  (3560,'MLW study ID','sample metadata',NULL,1,NULL,1,NULL,NULL,NULL,NULL),
  (3561,'oxford sample ID','sample metadata',NULL,1,NULL,1,NULL,NULL,NULL,NULL),
  (3562,'solaris sample ID','sample metadata',NULL,1,NULL,1,NULL,NULL,NULL,NULL),
  (3563,'file derived from','file relation',NULL,1,NULL,NULL,NULL,NULL,NULL,NULL),
  (3564,'requested insert size from','sequencing metadata',NULL,1,NULL,NULL,NULL,1,NULL,'requested_insert_size_from'),
  (3565,'requested insert size to','sequencing metadata',NULL,1,NULL,NULL,NULL,1,NULL,'requested_insert_size_to'),
  (3566,'forward read length','sequencing metadata',NULL,1,NULL,NULL,NULL,1,NULL,'forward_read_length'),
  (3568,'reverse read length','sequencing metadata',NULL,1,NULL,NULL,NULL,1,NULL,'reverse_read_length'),
  (3569,'tag index','sequencing metadata',NULL,1,NULL,NULL,NULL,1,'tag_index','tag_index'),
  (3570,'flowcell id','sequencing metadata','id_flowcell_lims in MLWH.iseq_flowcell',1,NULL,NULL,NULL,1,NULL,'id_flowcell_lims'),
  (3571,'flowcell position','sequencing metadata','aka \"lane\"',1,NULL,NULL,NULL,1,'lane','position'),
  (3572,'mlwarehouse flowcell entry id','sequencing metadata',NULL,1,NULL,NULL,NULL,1,NULL,NULL),
  (3575,'iRODs JSON','file metadata',NULL,0,NULL,NULL,NULL,1,NULL,NULL),
  (3576,'file type','file metadata',NULL,1,NULL,NULL,NULL,1,'type',NULL),
  (3577,'total reads','file metadata',NULL,1,NULL,NULL,NULL,1,'total_reads',NULL),
  (3578,'sequencing run','sequencing metadata',NULL,1,NULL,NULL,NULL,1,'id_run','id_run'),
  (3579,'CRAM reference','file metadata',NULL,1,NULL,NULL,NULL,1,'reference',NULL),
  (3580,'external release','sequencing metadata','Sequenscape external release (1 or 0)',1,NULL,NULL,NULL,NULL,NULL,'external_release'),
  (3581,'manual qc','sequencing metadata','Sequenscape manual QC (1 or 0)',1,NULL,NULL,NULL,1,'manual_qc','manual_qc'),
  (3582,'is r and d','sequencing metadata','Sequenscape \"is R&D\" (1 or 0)',1,NULL,NULL,NULL,1,NULL,'is_r_and_d'),
  (3584,'is paired read','file metadata','Sequenscape \"is_paired_read\" (1 or 0)',1,NULL,NULL,NULL,1,'is_paired_read',NULL),
  (3585,'sequenscape LIMS ID','sample metadata',NULL,1,NULL,1,NULL,NULL,NULL,NULL),
  (3586,'sequenscape sample name','sample metadata',NULL,1,NULL,1,NULL,NULL,NULL,NULL),
  (3587,'ENA sample accession ID','sample metadata','ebi_sample_acc',1,NULL,1,NULL,NULL,NULL,NULL),
  (3588,'alignment filter','file metadata',NULL,1,NULL,NULL,NULL,NULL,'alignment_filter',NULL),
  (3589,'sample supplier name','file and sample metadata',NULL,1,NULL,NULL,NULL,NULL,NULL,NULL),
  (3591,'sample common name','file and sample metadata',NULL,1,NULL,NULL,NULL,NULL,'sample_common_name',NULL),
  (3592,'library type','file metadata',NULL,1,NULL,NULL,NULL,NULL,'library_type',NULL),
  (3593,'sequenscape study ID','sample metadata',NULL,1,NULL,NULL,NULL,NULL,NULL,NULL),
  (3594,'sequenscape study name','sample metadata',NULL,1,NULL,NULL,NULL,NULL,NULL,NULL),
  (3595,'file contains same data','file relation',NULL,1,NULL,NULL,NULL,NULL,NULL,NULL),
  (3596,'instrument name','file metadata','from MLWH iseq_run_lane_metrics.instrument_name',1,NULL,NULL,NULL,1,NULL,NULL),
  (3597,'instrument model','file metadata','from MLWH iseq_run_lane_metrics.instrument_model',1,NULL,NULL,NULL,1,NULL,NULL),
  (3598,'run complete','file metadata','from MLWH iseq_run_lane_metrics.run_complete',1,NULL,NULL,NULL,1,NULL,NULL),
  (3599,'qc complete','file metadata','from MLWH iseq_run_lane_metrics.qc_complete',1,NULL,NULL,NULL,1,NULL,NULL),
  (3600,'MLWH taxon ID','sample metadata','from MLWH sample.taxon_id',1,NULL,NULL,NULL,1,NULL,NULL),
  (3602,'ENA run accession ID','file metadata','ebi_run_acc',1,NULL,NULL,NULL,1,NULL,NULL),
  (3604,'Alfresco study','sample metadata',NULL,1,NULL,NULL,NULL,NULL,NULL,NULL),
  (3605,'do use in data release','file and sample metadata','overrides \"do not use in data release\" tag',1,NULL,NULL,NULL,NULL,NULL,NULL),
  (3606,'do not use in data release','file and sample metadata',NULL,1,NULL,NULL,NULL,NULL,NULL,NULL),
  (3607,'subtrack file name','file metadata','from subtrack.submission',1,NULL,NULL,NULL,1,NULL,NULL),
  (3608,'subtrack submission ID','file metadata','from subtrack.submission',1,NULL,NULL,NULL,1,NULL,NULL),
  (3609,'subtrack file size','file metadata','from subtrack.submission',1,NULL,NULL,NULL,1,NULL,NULL),
  (3610,'ENA submission accession ID','file metadata','ebi_sub_acc',1,NULL,NULL,NULL,1,NULL,NULL),
  (3611,'ENA study accession ID','file metadata','ebi_study_acc',1,NULL,NULL,NULL,1,NULL,NULL),
  (3612,'ENA experiment accession ID','file metadata','ebi_exp_acc',1,NULL,NULL,NULL,1,NULL,NULL),
  (3613,'MLW legacy library ID','file metadata','[Cathrine Baker]: \"any sample or library is termed and asset and is given an asset hence 5890203, this asset is the library generated\"',1,NULL,NULL,NULL,1,'library_id','legacy_library_id'),
  (3614,'sequenscape library lims ID','file metadata','[Cathrine Baker]: \"is the \'Human barcode\', a readable barcode label that is used physically in the lab\"',1,NULL,NULL,NULL,1,NULL,'id_library_lims'),
  (3615,'sequenscape library name','file metadata','[Cathrine Baker]: \"is the library generated- same as [tag 3613]\"',1,NULL,NULL,NULL,1,'library',NULL),
  (3616,'Solaris study group','sample metadata','means \"species\"; imported from solaris.vw_vrpipe',1,NULL,1,NULL,NULL,NULL,NULL),
  (3617,'Solaris project code','sample metadata','the old, pre-Alfresco Solaris project name; imported from solaris.vw_vrpipe',1,NULL,NULL,NULL,NULL,NULL,NULL),
  (3618,'MLWH LIMS ID','sample metadata','mlwh sample.id_lims (a short string)',1,NULL,NULL,NULL,NULL,NULL,NULL),
  (3619,'MLWH reference genome','sample metadata','mlwh sample.reference_genome (a string)',1,NULL,NULL,NULL,NULL,NULL,NULL),
  (3620,'MLWH organism','sample metadata','mlwh sample.organism (a string)',1,NULL,NULL,NULL,NULL,NULL,NULL),
  (3621,'Human percent mapped','file metadata','mlwh.iseq_product_metrics',1,NULL,NULL,NULL,1,NULL,NULL);
