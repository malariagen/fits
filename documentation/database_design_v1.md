# Design principles
* Normalized schema (long tables, few fields)
* Files as central component
* Type and location agnostic
* Samples as a grouping mechanism, not as “sample definition”
* Tags and tag-value pairs to carry metadata for both samples and files
* Views for convenience

# Database core schema

![](https://github.com/wtsi-team112/fits/raw/master/documentation/FITS_database_schema_v1.png)

# Tables
## file
The file table contains the file name and full path, a tag reference for storage (currently: Sanger sequencing iRODs only). Its id field is the unique identifier (UID) for a file.

## sample
The sample table consists of just the id field for internal referencing, and an arbitrary name given on creation (currently, like “Sequenscape sample #1648793”).

## tag
The tag table contains a short list of tags to be used as metadata with samples and files. Its main information are the id and name fields, with some annotation. See below for a list at the time of writing.

## file2tag/sample2tag
These two tables are identical in structure. They contain a file_id or sample_id, respectively, a tag_id to reference a tag, a value field for the value of that tag in that file/sample, and an optional note field for human-readable annotation.

All data about files and samples in stored in these two tables. For convenience, they come with views, vw_file_tag and vw_sample_tag, respectively, which present relations and values in a more human-readable form.

Metadata includes data about the file itself (e.g. size, MD5 hashsum), external IDs (e.g. sequenscape sample ID, Oxford code), data in a file (e.g. number of reads, CRAM reference file), sample metadata (e.g. sequenscape study ID&name, taxon ID).
This data is collected from various sources, including Solaris, iRODs (via baton), and the Multi-LIMS Warehouse database.

## sample2file
This table models a relation between samples and files, by linking a sample_id and a file_id. This expresses the notion that data from a given sample is present in a given file. An optional note can give human-readable details.

## file_relation
This table related two files by their ID, parent and child. The type of relation is expressed by the relation field, referencing a tag. One example is a “file contains same data” relation, used for two files that contain the same data but are not identical, such as a BAM and a CRAM file on iRODs containing the same reads.

