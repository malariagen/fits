# Process documentation

## Rationale
This document describes the processes and data sources underlying FITS.

## General process
The FITS command line tool (_see below_) is adding new data to the FITS database daily. It is never overweiting existing information. This will preserve established data, but may lead to multiple entries where only one should exist. These conflicts will have to be resolved manually, but many can be detected automatically.

Manual imports and updates have taken place directly in the database. Manual changes will continue to resolve conflicts between different data sources, and establish a "truth set".

## FITS command line tool
All automated data import into FITS currently flows through the FITS command line tool ((source)[https://github.com/wtsi-team112/fits/tree/master/src]), especially the `update_sanger` sub-command. For implementation details, please see the relevant code.

The `sanity_checks` sub-command can perform some sanity checks (e.g. metadata that should have only one value but has more, such as "requested insert size").

## Data sources
### MLWH (Multi-LIMS Warehouse)
* The `study` table is the primary source of data; only samples in certain studies are considered, mainly based on the "Kwiatkowski" keyword in the `faculty_sponsor` field.
* Detailed data comes from the `iseq_flowcell` field for above studies. This information is used to populate sample metadata in FITS.

### iRODs/baton
Files and file-specific metadata are retrieved from iRODs, the Sanger file storage system for primary (=raw sequencing) data files.
Based on MLWH-derived data in FITS, a Sanger-supplied tool called `baton` is used to retrieve file names, locations, and various metadata.
Upon creation of file information in FITS, these files are also associated with the respective samples.

### Subtrack
Subtrack is another Sanger database. From here, mostly EBI-related IDs are imported into FITS as file/sample metadata.

### Solaris
A lot of metadata has been imported from Sanger, mainly sample-related information such as Oxford codes and Alfresco study associations. Much of that information came from the `vw_vrpipe` view, which is considered the "truth set" for many samples and file-to-sample associations. As this view is, in essence, underlying all previous Pv and Pf release builds, its information has replaced metadata from other sources, in case of conflict.
