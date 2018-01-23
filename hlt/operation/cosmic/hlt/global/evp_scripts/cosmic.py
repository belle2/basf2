#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Usage:
# the script running on hlt for cosmic

import basf2
import os
import sys
import ROOT

from reconstruction import add_cosmics_reconstruction
from rawdata import add_unpackers
from softwaretrigger import (
    add_fast_reco_software_trigger,
    add_hlt_software_trigger,
    add_calibration_software_trigger,
)
from softwaretrigger.hltdqm import cosmic_hltdqm
from daqdqm.cosmicdqm import add_cosmic_dqm

argvs = sys.argv
argc = len(argvs)

##########
# Various setups
##########
# Log level
basf2.set_log_level(basf2.LogLevel.ERROR)
# Components to be processed
components = ["CDC", "ECL", "TOP", "BKLM", "TRG"]

# Objects to be sent to storage
saveobjs = ['EventMetaData',
            'RawPXDs', 'RawSVDs', 'RawCDCs', 'RawTOPs',
            'RawECLs', 'RawKLMs',
            'RawCOPPERs', 'RawDataBlocks', 'RawFTSWs', 'RawTRGs',
            'SoftwareTriggerResult',
            'Tracks', 'TrackFitResults', 'CDCHits', 'TOPDigits',
            'ECLClusters', 'BKLMDigits', 'BKLMHit1ds', 'BKLMHit2ds']

# Crash handler switch
enable_graceful_crash_handling = False

##########
# Local DB specification
##########
basf2.reset_database()
basf2.use_local_database(ROOT.Belle2.FileSystem.findFile("hlt/examples/LocalDB/database.txt"))

##########
# Path definitions
##########
# main path
main_path = basf2.create_path()
# crash handling path
crashsafe_path = basf2.create_path()

##########
# Input
##########
# Input from ringbuffer (for raw data)
input = basf2.register_module('Raw2Ds')
input.param("RingBufferName", argvs[1])

# Input from ringbuffer (for simulated raw data in SROOT)
# input = basf2.register_module('Rbuf2Ds')
# input.param("RingBufferName", argvs[1])

# Input from SeqRoot file
# input = basf2.register_module('SeqRootInput')
# input file name should be specified with -i option

main_path.add_module(input)

##########
# Histogram Handling
##########
# HistoManager for real HLT
histoman = basf2.register_module('DqmHistoManager')
histoman.param("Port", int(argvs[3]))
histoman.param("Port", 9991)
histoman.param("DumpInterval", 180)
histoman.param("WriteInterval", 180)

# HistoManageer for offline
# histoman = basf2.register_module('HistoManager')

main_path.add_module(histoman)

##########
# Reconstruction
##########
# Raw data unpackers
add_unpackers(crashsafe_path, components=components)

# cosmic reconstruction
add_cosmics_reconstruction(crashsafe_path, components=components)

##########
# Software Trigger
##########
# hlt trigger modules for test
add_fast_reco_software_trigger(crashsafe_path)
add_hlt_software_trigger(crashsafe_path)
add_calibration_software_trigger(crashsafe_path)

##########
# Crash Handling
##########
if enable_graceful_crash_handling:
    crashhandler = main_path.add_module('CrashHandler', path=crashsafe_path)
    # in case of crashes, save the event and continue normally with the
    # following modules
    save_crashing_events_path = basf2.create_path()
    save_crashing_events_path.add_module('SeqRootOutput', outputFileName='crashing_events.sroot')
    crashhandler.if_false(save_crashing_events_path, basf2.AfterConditionPath.CONTINUE)
    crashhandler.set_log_level(basf2.LogLevel.WARNING)
else:
    main_path.add_path(crashsafe_path)


###########
# DQM
###########
add_cosmic_dqm(main_path, components=components)
cosmic_hltdqm(main_path)

##########
# Output
##########
# Output to RingBuffer
output = basf2.register_module("Ds2Rbuf")
output.param("RingBufferName", argvs[2])

# Output to SeqRoot
# output = basf2.register_module("SeqRootOutput")
# output.param('outputFileName', 'HLTout.sroot')
# output file name should be specified with -o option

# Specification of output objects
output.param("saveObjs", saveobjs)

main_path.add_module(output)

##########
# Other utilities
##########
progress = basf2.register_module('Progress')
main_path.add_module(progress)

etime = basf2.register_module('ElapsedTime')
etime.param('EventInterval', 10000)
etime.logging.log_level = basf2.LogLevel.INFO
main_path.add_module(etime)


##########
# Start basf2 processing
##########
basf2.set_nprocesses(int(argvs[4]))
basf2.set_streamobjs(saveobjs)

basf2.process(main_path)
