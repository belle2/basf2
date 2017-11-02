#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Imports
import os
import sys

# command arguments
#   argvs[1] = input ring buffer name
#   argvs[2] = output ring buffer name
#   argvs[3] = port number of hserver
#   argvs[4] = number of cores for parallel processing

argvs = sys.argv
argc = len(argvs)

from basf2 import *

from simulation import add_simulation
import os

from rawdata import add_raw_seqoutput, add_unpackers

from softwaretrigger.path_functions import (
    setup_softwaretrigger_database_access,
    add_softwaretrigger_reconstruction,
    DEFAULT_HLT_COMPONENTS,
)

from ROOT import Belle2

# Log level
set_log_level(LogLevel.ERROR)

# Setup local database
reset_database()
# use_local_database(Belle2.FileSystem.findFile("hlt/examples/LocalDB/database.txt"))
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))

# You could use your own components here or just use the default for the HLT (everything except PXD)
# e.g. without SVD
#     components = ["CDC", "ECL", "TOP", "ARICH", "BKLM", "EKLM"]
# if you leave out the components in all calls, the default will be used
# components = DEFAULT_HLT_COMPONENTS
components = ["CDC", "ECL", "TOP", "ARICH", "BKLM", "EKLM"]


# Create Path
main_path = create_path()

##########
# Input
##########
# Input from a file
# input = register_module('SeqRootInput')
# input.param('inputFileName',argvs[1])

# Input from ringbuffer (Raw2Ds for real raw data)
# input = register_module("Raw2Ds")
# input.param("RingBufferName", argvs[1])

# Input from ringbuffer (Rbuf2Ds for simulated raw data)
input = register_module('Rbuf2Ds')
input.param("RingBufferName", argvs[1])

main_path.add_module(input)

##########
# HistoManager
##########
# HistoManager for offline
# histoman = register_module('HistoManager')
# histoman.param('histoFileName', 'hlt_dqm.root')

# HistoManager for real HLT
histoman = register_module('DqmHistoManager')
histoman.param("Port", 9991)
# histoman.param("Port", int(argvs[3]))
histoman.param("DumpInterval", 10000)

main_path.add_module(histoman)

##########
# Unpacker (may crash)
##########
crashsafe_path = create_path()
add_unpackers(crashsafe_path, components=components)

##########
# Tracking (may crash)
##########
# Reconstruction chain for Software Trigger
add_softwaretrigger_reconstruction(crashsafe_path, store_array_debug_prescale=1, components=components)

##########
# DQM (may crash)
##########
# Tracking DQM
trackdqm = register_module('TrackAna')
crashsafe_path.add_module(trackdqm)

# add_dqm_tracking(crash_path)
# phystrigdqm = register_module ( 'PhysicsTriggerDQM' )
# crash_path.add_module(phystrigdqm)

##########
# Crash Handling
##########
enable_graceful_crash_handling = False
if enable_graceful_crash_handling:
    crashhandler = main_path.add_module('CrashHandler', path=crashsafe_path)

    # in case of crashes, save the event and continue normally with the
    # following modules
    save_crashing_events_path = create_path()
    save_crashing_events_path.add_module('SeqRootOutput', outputFileName='crashing_events.sroot')
    crashhandler.if_false(save_crashing_events_path, AfterConditionPath.CONTINUE)
    crashhandler.set_log_level(LogLevel.WARNING)
else:
    main_path.add_path(crashsafe_path)


# - Output to a file
# output = register_module ('SeqRootOutput')
# output.param('outputFileName', argvs[2] )
# - Output to RingBuffer (Ds2Rbuf)
output = register_module("Ds2Rbuf")
output.param("RingBufferName", argvs[2])

# Output object list
objlist = ['EventMetaData', 'RawPXDs', 'RawSVDs', 'RawCDCs', 'RawTOPs',
           'RawARICHs', 'RawECLs', 'RawKLMs', 'ROIs',
           'SoftwareTriggerResult', 'SoftwareTriggerVariables'
           ]
output.param("saveObjs", objlist)

main_path.add_module(output)

# Progress
progress = register_module('Progress')
main_path.add_module(progress)


# Start processing
set_nprocesses(int(argvs[4]))
process(main_path)
