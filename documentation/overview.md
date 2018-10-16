# FITS overview

## Rationale
The group requires a single system to track all primary data files, associate them with samples, track relevant metadata, and allow retrieval of that data for specific applications, e.g. to build a manifest of files to go into an analysis. The system to perform this purpose shall be called FITS (FIle Tracking System).

## Requirements
* Track all primary (raw) data files the group has produced, or uses
* Be agnostic to different storage systems (iRODs, various net/cloud systems)
* Associate files with primary metadata, such as a sample identifier
* Store secondary metadata (e.g. "requested insert size") where easily obtainable
* Resolve conflicts in such metadata, where possible
* Present the master repository for such metadata for the group (that is, the agreed-upon truth)

## Detailed documents
* (MVP V1)[https://github.com/wtsi-team112/fits/blob/master/documentation/mvp_v1.md], the first Minimum Viable Product
* (Database design)[https://github.com/wtsi-team112/fits/blob/master/documentation/database_design_v1.md], a description of the databse underlying FITS
* (How to build a manifest)[https://github.com/wtsi-team112/fits/blob/master/documentation/How_to_build_a_manifest.md], a practical approach on how to generate a manifest file from database queries
* (Processes)[https://github.com/wtsi-team112/fits/blob/master/documentation/processes.md], as description on how data flows into FITS
