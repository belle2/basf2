#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
# This steering file process efficiency studies for bklm
# useage: basf2 -n 10000 bklmeffi.py
########################################################

from basf2 import *

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.ERROR)
# set_log_level(LogLevel.INFO)
input = register_module('RootInput')
filelist = '/ghi/fs01/belle2/bdata/group/detector/BKLM/GCR1/fullRec/dst.cosmic.0001.03978.*.root'
input.param('inputFileNames', filelist)
# filelist = ['/ghi/fs01/belle2/bdata/group/detector/BKLM/GCR1/fullRec/dst.cosmic.0001.03978.f00000.root',
#             '/ghi/fs01/belle2/bdata/group/detector/BKLM/GCR1/fullRec/dst.cosmic.0001.03978.f00001.root'
#            ]
# input.param('inputFileNames', filelist)

gearbox = register_module('Gearbox')
print('Gearbox registered')

# Geometry builder
geobuilder = register_module('Geometry')

# Create main path
main = create_path()
# Add modules to main path
main.add_module(input)
main.add_module(gearbox)
main.add_module(geobuilder)

# re-process bklmReconstructor and Muid
bklmreco = register_module('BKLMReconstructor')
bklmreco.param('Prompt window (ns)', 2000)
# main.add_module(bklmreco)

muid = register_module('Muid')
muid.param('MaxDt', 2000)
# main.add_module(muid)

# efficiency based on bklm-stand-alone tracks
# bklmTrack =  register_module('BKLMTracking')
# bklmTrack.param('StudyEffiMode', 1)
# bklmTrack.param('outputName',"bklmEffi_run3173.root")

# efficiency based on extHit
bklmAna = register_module('BKLMAna')
bklmAna.param('filename', 'extEffi_run3173.root')
# main.add_module(bklmTrack)
main.add_module(bklmAna)
main.add_module('Progress')

# Process all events
process(main)
print(statistics)
