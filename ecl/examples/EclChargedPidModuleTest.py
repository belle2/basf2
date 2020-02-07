#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""

This steering file tests the ECLChargedPIDModule.

It must run on DST files, or reduced DSTs containing
ECLShowers, Tracks and all relevant relational containers:

'Tracks',
'TrackFitResults',
'ECLClusters',
'ECLShowers',
'TracksToECLClusters',
'TracksToECLShowers',
'ECLClustersToECLShowers',
'ECLConnectedRegions',
'ECLPidLikelihoods',
'PIDLikelihoods',
'TracksToECLPidLikelihoods',
'TracksToPIDLikelihoods',

Author: Marco Milesi (marco.milesi@unimelb.edu.au)
Year: 2018

Usage:

basf2 -n N -i /path/to/input/DST/file.root EclChargedPidModuleTest.py

"""

import basf2 as b2

# Register necessary modules to this path.
main_path = b2.create_path()

# Add module to read input *DST file.
simpleinput = b2.register_module('RootInput')
main_path.add_module(simpleinput)

# Add the module to the path.
eclid = b2.register_module('ECLChargedPID')
main_path.add_module(eclid)
# Set debug options for this module.
eclid.logging.log_level = b2.LogLevel.DEBUG
eclid.logging.debug_level = 20

# Print the data model objects in the store.
printcolls = b2.register_module('PrintCollections')
main_path.add_module(printcolls)

# Start processing events.
b2.process(main_path)

# Get some statistics about the booked modules.
print(b2.statistics)
