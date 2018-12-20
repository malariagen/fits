# Database design
This document describes the database used in the FITS file tracking system.
It does _not_ describe FITS in total.
See [overview](https://github.com/malariagen/fits/blob/master/documentation/overview.md) and [MVP V1](https://github.com/malariagen/fits/blob/master/documentation/mvp_v1.md) for that.

# Design principles
Tables in the FITS database are _normalized_, that is, they contain many rows but few columns.
Relations between rows in tables are defined through other (also normalized) tables.

That approach increases flexibility, but makes it somewhat more involved to query data.
The solution for this problem are views, which can simulate larger (denormalized) tables with more columns.
These are often easier to query, and more convenient for huamns to read.

Files, stored in the `file` table, are the backbone of the database.

Samples exists as a "adjuncts" to files.
Samples without files would be useless, in the context of this database.
Samples serve as an internal grouping mechanism, derived from the data sources used to populate the FITS database.
In the FITS database, a sample does therefore not necessarily correspond to any concept of a sample in other systems.

Both files and samples can be annotated with tags and values, in the respective tables.
These annotations carry (almost) all metadata for either.


# Database core schema

![](https://github.com/wtsi-team112/fits/raw/master/documentation/FITS_database_schema_v1.png)

# Tables
Individual fields not described here are annotated in the SQL schema of the respective table.

## file
The `file` table contains the file name and full path, a `tag` reference for `storage` (currently: Sanger sequencing iRODs only. It is used to make it faster and easier to differentiate between different storage systems, once FITS uses more than one. This information is also present in `file2tag`).
Its `id` field is the unique identifier (UID) for a file.

## sample
The `sample` table consists of just the id field for internal referencing, and an arbitrary name given on creation (currently, like “Sequenscape sample #1648793”).

## tag
The `tag` table contains a short list of tags (59 as of 17 Dec 2018) to be used as metadata with samples and files.
Its main information are the `id` and `name` fields, with some annotation.
A `type` field indicates the subject area of the tag, but no functionality or technical limitation is derived from that.

Tags are used in conjunction with a `value` and a `file` or `sample` in the respective tables (see below).
Tags are also used directly (without values) in the `file` table to indicate the storage system, and in the `file_relation` table to indicate the relation of two files.

Tags can, and should, be self-documenting via their `note` field.
Detailed description of individual tags is therefore not part of this document.
Use common sense for the `note` documentation.

At the moment, tags can only be added by Magnus.
This might change with more people getting write access to the FITS database, or alternative interfaces (e.g. web app) in the future.

## file2tag/sample2tag
These two tables are identical in structure.
They contain a `file_id` or `sample_id`, respectively, a tag_id to reference a tag, a `value` field for the value of that `tag` in that `file`/`sample`, and an optional `note` field for human-readable annotation.

All data about files and samples in stored in these two tables. For convenience, they come with views, `vw_file_tag` and `vw_sample_tag`, respectively, which present relations and values in a more human-readable form.

Metadata includes data about the file itself (e.g. size, MD5 hashsum), external IDs (e.g. sequenscape sample ID, Oxford code), data in a file (e.g. number of reads, CRAM reference file), sample metadata (e.g. sequenscape study ID&name, taxon ID).

This data is collected from various sources, including Solaris, iRODs (via baton), and the Multi-LIMS Warehouse and subtrack databases.
The source and date of import is often given in the `note` field (this should be done automatically now, but may be missing from early imports).
See the [processes documentation](https://github.com/malariagen/fits/blob/master/documentation/processes.md) for details.

## sample2file
This table models a relation between `sample`s and `file`s, by linking a `sample_id` and a `file_id`.
This expresses the notion that data from a given sample is present in a given file.
An optional `note` can give human-readable details.

## file_relation
This table related two files by their ID, parent and child.
The type of relation is expressed by the `relation` field, referencing a `tag`.
One example is a “file contains same data” relation, used for two files that contain the same data but are not identical, such as a BAM and a CRAM file on iRODs containing the same reads.
Future use might also, for example, incude "data derived from" relations.

## Other tables
There are several other tables in the database, but they are temporary/test ones, and are not an essential part of FITS. They may be altered or removed at any point in the future. Therefore, if a table is not listed here, ignore it.

# Views
Views are read-only, user-friendly representations of (parts of) the tables described above. They are not required for FITS functionality, but serve as easier access to the data for humans.

## vw_file_tag/vw_sample_tag
These two views are an amalgamate of the `file`/`sample` table and the `tag` table. They make it easier to see the meaning of a tag in conjunction with its value.

## vw_pivot_file/vw_pivot_sample
These two files show a "pivot table" view of `tag` values vs. the `file`/`sample` table. Each possible tag is a column, each file/sample is a row. Multiple values for the same tag and file/sample are separated by "|".

When a new tag is added, the command line tool needs to be run with `fits upsate_sanger --pivot_views` to update the views to include the new tag.

Due to a regression in the MySQL version used by Sanger, these views are currently quite slow. This cannot be fixed within FITS.

## vw_sample_file
An amalgamate of `sample` and `file` table, showing the file path in conjunction with the FITS sample ID.

## vw_sample_file_meta
Similar to `vw_sample_file` but including the file size. Obsolete.

## vw_tag_types
A statistical overview of all `tag`s.

## vw_files_with_reads
A basic summary of files that have a read count tag.

Due to a regression in the MySQL version used by Sanger, this view is currently quite slow. This cannot be fixed within FITS.

## Other views
There are several other views in the database, but they are temporary/test ones, and are not an essential part of FITS. They may be altered or removed at any point in the future. Therefore, if a view is not listed here, ignore it.

# Access
For obvious basic operational security reasons, database access details can not, and should not, be part of this documentation, or this repository.
For access to the FITS database, ask Magnus.
