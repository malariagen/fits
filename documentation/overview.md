# FITS overview

## Rationale

The group requires a single system to track all primary data files, associate them with samples, track relevant metadata, and allow retrieval of that data for specific applications, e.g. to build a manifest of files to go into an analysis.

## Requirements
* Track all primary (raw) data files the group has produced, or uses
* Be agnostic to different storage systems (iRODs, various net/cloud systems)
* Associate files with primary metadata, such as a sample identifier
* Store secondary metadata (e.g. "requested insert size") where easily obtainable
* Resolve conflicts in such metadata, where possible
* Present the master repository for such metadata for the group (that is, the agreed-upon truth)
