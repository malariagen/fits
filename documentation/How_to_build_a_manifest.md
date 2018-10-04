# How to build a manifest
This document describes how to build a manifest file from FITS.
It uses the database directly.
Typically, the resulting manifest file will consist of one file per row, with file name, sample name, and other metadata in tab-separated columns.
This tutorial assumes a basic knowledge of MySQL syntax.

## Specify what you want, then filter
The general strategy is to specify a _superset_ of the data you want (to make sure you get all of it), then filter out the parts you don't want.


## Query on tags for samples and files
Both samples and files can have tags with values associated with them.
These associations are stored in the `sample2tag` and `file2tag` tables, respectively.
Each row in these tables has a
* `tag_id` (for a list of available tags, see the `tag` table)
* `sample_id` or `file_id`, respectively
* `value` storing the value for this tag, for this sample/file
* `note` can contain a free-text note, explaining where this data comes from

### Notes
* For any tag and file/sample combination, there may exist any number of distinct values, one per row (say, a sample in multiple Alfresco studies)
* There can only ever be _one_ row for a tag/(sample/file)/value combination
* The same tag/value combination may exist for mulitpe samples/files (e.g multiple FITS sample IDs may be associated with the same Oxford code)
* For a better overview of the data, check out the `vw_sample_tag`/`wv_file_tag` views; they contain the same information, but more nicely annotated with tag information, for human consumption.

### Examples
To get all FITS samples that have a _P falciparum_ taxon ID (`tag_id` 3600=NCBI numbers, source: Multi-LIMS Warehouse):
```sql
SELECT DISTINCT sample_id FROM sample2tag WHERE tag_id=3600 AND value IN (5833,36329,5847,57267,137071);
```
Likewise, to get all FITS samples that have one of your favourite Oxford codes (`tag_id`=3561):
```sql
SELECT DISTINCT sample_id FROM sample2tag WHERE tag_id=3561 AND value IN ("AA0023-C","PF0480-C","WT3332-C");
```

## Relate samples to files
Samples are stored in the `sample` table, files in the `file` table.
Their `id` fields corresponds to `sample_id` and `file_id` from the tag association tables, respectively.
The `sample` table is otherwise quite sparse; the `file` table also contains the file name, full file path, and the storage system.
The `sample2file` table relates those two tables.
It can be used as the _backbone_ for a manifest-generating query.

### Example
To get all files for samples associated with your favourite Oxford code (see example above):
```sql
SELECT # The main SELECT statement
	# List of columns to return for each file
	(SELECT full_path FROM file WHERE file.id=sample2file.file_id) AS file_full_path, # The full path of the file
	(SELECT value FROM sample2tag WHERE sample2tag.sample_id=sample2file.sample_id AND tag_id=3561 LIMIT 1) AS oxford_code # The Oxford code of the sample; note the "LIMIT 1" to ensure only one value!
FROM sample2file # The "backbone table" of the query
WHERE
	# Now follow the conditions for files and samples to include
	sample_id IN (SELECT DISTINCT sample_id FROM sample2tag WHERE tag_id=3561 AND value IN ("AA0023-C","PF0480-C","WT3332-C")) # This subquery is the same as in the tag example!
;
```
This _clean_ way to generate a manifest has
* a subquery for each cloumn you want in your manifest
* a subquery for each condition of your data

## Filter your results
The FITS database contains a lot of data, likely more than you need, or want, at a given time.
The above example will therefore likely return more results than you expect, or require.
Therefore, you need to filter the results.

One way is to _include_ only specific samples/files. To restrict the returned files to BAM and CRAM, add the following condition:
```sql
AND file_id IN (SELECT file_id FROM file2tag WHERE tag_id=3576 AND value IN ('bam','cram')) # Only BAM or CRAM
```

Another way is to _exclude_ specific samples/files. To remove files that have "\_phix" or "\_human" in their filename, add the following condition:
```sql
AND NOT EXISTS (SELECT * FROM file WHERE file.id=file_id AND (full_path LIKE "%\_phix%" OR full_path LIKE "%\_human%") ) # Exclude phiX/human
```
### Examples
Other conditions that are handy for creating a manifest file include:
```sql
AND NOT EXISTS (SELECT * FROM file2tag WHERE sample2file.file_id=file2tag.file_id AND tag_id=8 AND `value`='0') # Exclude zero-size files, where file size is set
AND file_id NOT IN (SELECT parent FROM file_relation WHERE relation=3595) # Only one of multiple, equivalent data files (e.g. BAM and CRAM with identical data)
AND file_id NOT IN (SELECT DISTINCT file_id FROM file2tag WHERE tag_id=3592 AND `value`='GBS') # Exclude genotyping-by-sequencing files
AND file_id NOT IN (SELECT DISTINCT file_id FROM file2tag WHERE tag_id=3577 AND `value`='0') # Exclude files with zero reads
```
