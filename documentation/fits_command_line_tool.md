# The FITS command line utility

## Introduction
This document describes the FITS command line utility. The binary executable is names `fits`.

## Implementation
The FITS command line utility is written in C++. You can find the code in [the src path](https://github.com/malariagen/fits/tree/master/src).

### Components
* `fits` contains the `main()` function for the utility.
* `database` implements a MySQL interface.
* `database_abstraction_layer` inherits from `database`, and implements special functionality for the FITS database.
* `tools` implements various convenience functions.
* `update_sanger` performs data imports from MLWH and iRODs/baton.
* `sanity_checks` implemets the sanity_checks functionality.
* `json.hpp` is a third-party JSON parser. It is used to interface with iRODs/baton.
* `command_list` is part of the (partially implemented) query functionality.
* `condition_parser` is part of the (partially implemented) query functionality.

### Configuration files
Each database that is accessed by the utility needs to have a configuration file, in the same directory as the `fits` binary.
At the moment, these are:
* `fits.conf`
* `mlwh.conf`
* `subtrack.conf`
A config file consists of key-value-pairs, one per line:
```
host = HOST_NAME
schema = DATABASE_NAME
port = PORT_NUMBER
user = YOUR_USER_NAME
password = YOUR_PASSWORD_HERE
```

## Current functionality
At the moment, the utility can perform two functions:
* Update the FITS database from Sanger MLWH and iRODs (via baton)
* Perform some basic sanity checks on the data in the FITS database
This is considered to implement [MVP V1](https://github.com/malariagen/fits/blob/master/documentation/mvp_v1.md).

## Future functionality
The utility contains a partially implemented query function, to allow a user to generate a manifest.
This is _not_ considered to be in scope for MVP V1. For now, manifests can be generated [from the database](https://github.com/malariagen/fits/blob/master/documentation/How_to_build_a_manifest.md).

The utility will also be able to import data from other sources. SIMS, and possibly ROMA, come to mind.
This is _not_ considered to be in scope for MVP V1.

## Usage
_Note:_ For some of these functions to work, write-access configuration files are necessary.
* `fits update_sanger` performs the update from MLWH/iRODs. This can take a while. As this is currently run daily, manual runs are unnecessary.
* `fits update_sanger --pivot_views` updates the `vw_pivot_file` and `vw_pivot_sample` view definitions, based on the `tag` table. This has to be done manually after adding/renaming/deleting a tag, though the views are not used by FITS itself, but serve as a convenience to FITS database users.
* `fits sanity_checks` performs some sanity checks on the data.