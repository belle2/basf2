#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
import os
import sys
import ROOT
from rawdata import add_unpackers
from tracking import add_cdc_cr_track_finding, add_prune_tracks
from reconstruction import add_cosmics_reconstruction, add_posttracking_reconstruction

# command arguments
#   argvs[1] = input ring buffer name
#   argvs[2] = output ring buffer name
#   argvs[3] = port number of hserver
#   argvs[4] = number of cores for parallel processing

argvs = sys.argv
argc = len(argvs)

##########
# Various setups
##########
basf2.set_log_level(basf2.LogLevel.ERROR)

# components used in reconstruction
# components = ["CDC", "ECL", "TOP", "ARICH", "BKLM", "EKLM"]
components = ["CDC", "ECL", "TOP", "BKLM"]

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
# Input from ringbuffer
input = basf2.register_module('Raw2Ds')
input.param("RingBufferName", argvs[1])

# Input from ringbuffer (Rbuf2Ds for simulated raw data)
# input = basf2.register_module('Rbuf2Ds')
# input.param("RingBufferName", argvs[1])
# main_path.add_module(input)

# Input from SeqRoot file
# input = basf2.register_module('SeqRootInput')
# input file name should be specified with -i option

main_path.add_module(input)

##########
# Histogram Handling
##########
# HistoManager for real HLT
histoman = basf2.register_module('DqmHistoManager')
# histoman.param("Port", int(argvs[3]))
histoman.param("Port", 9991)
histoman.param("DumpInterval", 1000)

# HistoManageer for offline
# histoman = basf2.regsiter_module('HistoManager')

main_path.add_module(histoman)


##########
# Reconstruction
##########
# Raw data unpackers
add_unpackers(crashsafe_path, components=components)

# Add cdc tracking reconstruction modules
# track finding
add_cdc_cr_track_finding(crashsafe_path)

# track fitting
crashsafe_path.add_module('SetupGenfitExtrapolation', energyLossBrems=False, noiseBrems=False)

# Time seed
crashsafe_path.add_module("PlaneTriggerTrackTimeEstimator",
                          pdgCodeToUseForEstimation=13,
                          triggerPlanePosition=[0, 0, 0],
                          triggerPlaneDirection=[0, 1, 0],
                          useFittedInformation=False)

# Initial track fitting
crashsafe_path.add_module("DAFRecoFitter",
                          probCut=0.00001,
                          pdgCodesToUseForFitting=13,
                          )

# Correct time seed
crashsafe_path.add_module("PlaneTriggerTrackTimeEstimator",
                          pdgCodeToUseForEstimation=13,
                          triggerPlanePosition=[0, 0, 0],
                          triggerPlaneDirection=[0, 1, 0],
                          useFittedInformation=True
                          )

# Track fitting
crashsafe_path.add_module("DAFRecoFitter",
                          # probCut=0.00001,
                          pdgCodesToUseForFitting=13,
                          )

# Create Belle2 Tracks from the genfit Tracks
crashsafe_path.add_module('TrackCreator',
                          pdgCodes=[13],
                          useClosestHitToIP=True
                          )

# Add further reconstruction modules
add_posttracking_reconstruction(crashsafe_path, components=components, pruneTracks=True,
                                addClusterExpertModules=True,
                                trigger_mode="all")

##########
# Crash Handling
##########
enable_graceful_crash_handling = False
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
# DQM modules here
###########
# monitor the result of reconstruction of CDC, ECL, and KLM
crydqmhlt = basf2.register_module('CosmicRayHLTDQM')
main_path.add_module(crydqmhlt)

# CDCDQM
cdcdqm = basf2.register_module('cdcDQM7')
main_path.add_module(cdcdqm)

##########
# Output
##########
# Objects to be sent to storage
objlist = ['EventMetaData',
           'RawPXDs', 'RawSVDs', 'RawCDCs', 'RawTOPs',
           'RawECLs', 'RawKLMs',
           'RawCOPPERs', 'RawDataBlocks', 'RawFTSWs', 'RawTRGs',
           'Tracks', 'TrackFitResults', 'CDCHits', 'TOPDigits',
           'ECLClusters', 'BKLMDigits', 'BKLMHit1ds', 'BKLMHit2ds']

# Output to RingBuffer
output = basf2.register_module("Ds2Rbuf")
output.param("RingBufferName", argvs[2])

# Output to SeqRoot
# output = basf2.register_modules("SeqRootOutput")
# output file name should be specified with -o option

# Specification of output objects
output.param("saveObjs", objlist)

main_path.add_module(output)

##########
# Select streaming objects (turn on only when streamng bottleneck is observed)
##########
# Limit streaming objects to minimal, add objects requred to be streamed
# set_streamobjs(objlist+['RecoTracks'])

##########
# Start basf2 processing
##########
basf2.set_nprocesses(int(argvs[4]))
basf2.process(main_path)
