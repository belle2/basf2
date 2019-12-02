#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# KLM data quality monitor test.
#
# DQM analysis example (the next stage of DQM data processing) is
# dqm/analysis/examples/DQMAnalysisKLM.py

import basf2
import sys

raw_data = False
input_files = []
if len(sys.argv) >= 2:
    if sys.argv[1] == 'raw':
        raw_data = True
        input_files = sys.argv[2:]
    else:
        input_files = sys.argv[1:]
else:
    message = 'Usage of the script:\n\n'
    message += '  basf2 TestDQM.py <inputFiles>\n\n'
    message += 'If raw data are used as input, please use "raw" as first argument:\n\n'
    message += '  basf2 TestDQM.py raw <inputFiles>\n\n'
    message += 'Note that it is possible to use the wildcard "*" to pass multiple files in input.\n'
    sys.exit(message)

if not input_files:
    sys.exit('Please provide at least one input file.')

# Check if we are using .sroot files or not
sroot = False
if input_files[0].endswith('.sroot'):
    sroot = True
    # If .sroot files are used, we must disable the GT replay
    basf2.conditions.override_globaltags(['online'])
    basf2.B2INFO('No GT is set in FileMetaData; GT automatically set to "online"')
elif input_files[0].endswith('.root'):
    # Disable the GT replay also if no GT is set in FileMetaData
    metadata = basf2.get_file_metadata(input_files[0])
    if metadata.getDatabaseGlobalTag() == '':
        raw_data = True
        basf2.conditions.override_globaltags(['online'])
        basf2.B2INFO('No GT is set in FileMetaData; GT automatically set to "online"')
else:
    sys.exit('Please provide a valid list of .sroot or .root input files.')

# Set the global log level
basf2.set_log_level(basf2.LogLevel.INFO)

# Create path
main = basf2.Path()

# Input
if sroot:
    main.add_module('SeqRootInput',
                    inputFileNames=input_files)
else:
    main.add_module('RootInput',
                    inputFileNames=input_files)

# Progress
main.add_module('Progress')

# Histogram manager
main.add_module('HistoManager',
                histoFileName='KLMDQM.root')

# Unpacker and reconstruction.
if (sroot or raw_data):
    # Gearbox
    main.add_module('Gearbox')

    # Geometry
    main.add_module('Geometry')

    # Unpacker
    main.add_module('KLMUnpacker')

    # BKLM reconstruction
    main.add_module('BKLMReconstructor')

# DQM
main.add_module('KLMDQM')

# Process events
basf2.process(main)
print(basf2.statistics)
