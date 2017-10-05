#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
# This steering file shows how to run the framework
# for different experiment, run and event numbers.
#
# In the example below, basf2 will run on and display
# the following experiment, run and event numbers:
#
# Experiment 71, Run  3, 4 Events
# Experiment 71, Run  4, 6 Events
# Experiment 73, Run 10, 2 Events
# Experiment 73, Run 20, 5 Events
# Experiment 73, Run 30, 3 Events
#
# Example steering file - 2011 Belle II Collaboration
# useage basf2 readDataFile.py -i srootfilename -o outputfilename
######################################################

from basf2 import *

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.ERROR)
# set_log_level(LogLevel.INFO)

use_local_database("localdb/database.txt", "localdb")
# use use_central_database for uploading data to PNNL
# use_central_database("test_bklm")

# input
# input = register_module('RootInput')
input = register_module('SeqRootInput')
conversion = register_module('Convert2RawDet')
# method 1, input one file
# filelist = '/hsm/belle2/bdata/Data/sRaw/e0001/r03978/sub00/cosmic.0001.03978.HLT1.f00000.sroot'
# input.param('inputFileName', filelist)
# method 2, input several files, use a pattern for the filenames
filelist = '/hsm/belle2/bdata/Data/sRaw/e0001/r03978/sub00/cosmic.0001.03978.HLT1.f%05d.sroot'
input.param('fileNameIsPattern', 1)
input.param('inputFileNames', filelist)
# method 3, input several files, name all files
# filelist = ['/hsm/belle2/bdata/Data/sRaw/e0001/r03978/sub00/cosmic.0001.03978.HLT1.f00000.sroot',
#             '/hsm/belle2/bdata/Data/sRaw/e0001/r03978/sub00/cosmic.0001.03978.HLT1.f00001.sroot'
#            ]
# input.param('inputFileNames', filelist)

# EventInfoSetter - generate event meta data
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [20])

gearbox = register_module('Gearbox')
print('Gearbox registered ')
# gearbox.param('backends', ['file:./geometry/data/', 'file:./XMLdata/', 'file:'])

# Geometry builder
geobuilder = register_module('Geometry')
geobuilder.log_level = LogLevel.INFO
geobuilder.param('components', ['BKLM'])
bklmUnpack = register_module('BKLMUnpacker')
# bklmUnpack.param("keepEvenPackages",1)
bklmUnpack.param("useDefaultModuleId", 1)
bklmUnpack.param('loadMapFromDB', 0)
bklmUnpack.param('rawdata', 1)
bklmreco = register_module('BKLMReconstructor')
# bklmreco.log_level = LogLevel.INFO

# efficiencies...
# bklmEff = register_module('BKLMEffnRadio')

# bklm tracking
bklmtrk = register_module('BKLMTracking')

print('backend set ')
# gearbox.param('fileName', 'Belle2_red.xml')
# gearbox.param('InputFileXML','Belle2.xml')

# output
# output = register_module('PrintDataTemplate')

# Create main path
main = create_path()

# Add modules to main path
# main.add_module(eventinfosetter)
main.add_module(input)
main.add_module(conversion)
main.add_module(gearbox)
main.add_module(geobuilder)

main.add_module(bklmUnpack)
main.add_module(bklmreco)
main.add_module(bklmtrk)
# main.add_module(bklmEff)

# output
output = register_module('RootOutput')
output.param('branchNames', ['BKLMDigits', 'BKLMHit2ds'])
# output.param('outputFileName', 'bklm_mdst.root')
main.add_module(output)

# Print progress
progress = register_module('Progress')
main.add_module(progress)

# Process all events
process(main)

# Print statistics
print(statistics)
