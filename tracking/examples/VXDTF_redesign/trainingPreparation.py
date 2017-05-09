#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#####################################################################
# VXDTF2 Example Scripts - Step 1 - Training Data Preparation
#
# This script prepares a dataset for the training of the
# Sector Map from Monte Carlo data.
#
# The input file can be provided with the basf2 commandline
# argument '-i <inputfilename>'.
#
# NOTE: if you want to use PXD you set usePXD in this script!
#
# Contributors: Jonas Wagner, Felix Metzner
#####################################################################


from basf2 import *
from setup_modules import setup_RTCtoSPTCConverters
from setup_modules import setup_Geometry


# ---------------------------------------------------------------------------------------
# Settings

# Logging and Debug Level
# TODO: Remove logLevel, as it can be set via basf2 option -l
set_log_level(LogLevel.ERROR)
log_to_file('logVXDTF2Preparation.log', append=False)
# if false PXD hits will be ignored in the trainings data collection
usePXD = False

# ---------------------------------------------------------------------------------------
# Create paths
path = create_path()

# Input Module
rootInputM = register_module('RootInput')
path.add_module(rootInputM)

# Event Info Module
eventinfoprinter = register_module('EventInfoPrinter')
path.add_module(eventinfoprinter)

# puts the geometry and gearbox in the path
setup_Geometry(path)

# Event counter
eventCounter = register_module('EventCounter')
path.add_module(eventCounter)


# put PXD and SVD SpacePoints into the same StoreArray
if usePXD:
    spCreatorPXD = register_module('SpacePointCreatorPXD')
    spCreatorPXD.param('NameOfInstance', 'PXDSpacePoints')
    spCreatorPXD.param('SpacePoints', 'SpacePoints')
    path.add_module(spCreatorPXD)

spCreatorSVD = register_module('SpacePointCreatorSVD')
spCreatorSVD.param('OnlySingleClusterSpacePoints', False)
spCreatorSVD.param('NameOfInstance', 'SVDSpacePoints')
spCreatorSVD.param('SpacePoints', 'SpacePoints')
path.add_module(spCreatorSVD)


# Converts GenFit track candidates and checks them, with respect to the SecMap settings
# Produces SpacePoint TrackCand which is used in VXDTFTrainingDataCollector.
setup_RTCtoSPTCConverters(path=path,
                          SPscollection='SpacePoints',
                          RTCinput='MCRecoTracks',
                          sptcOutput='checkedSPTCs',
                          usePXD=usePXD,
                          logLevel=LogLevel.ERROR)


# SecMap BootStrap
# Module to fetch SecMap Config and store or load SecMap Training.
# Config is defined in /tracking/modules/vxdtfRedesing/src/SectorMapBootstrapModule.cc
# and must be available for the training of the SecMap
# Double False only fetches config.
secMapBootStrap = register_module('SectorMapBootstrap')
secMapBootStrap.param('ReadSectorMap', False)
secMapBootStrap.param('WriteSectorMap', False)
path.add_module(secMapBootStrap)


# Module for generation of train sample for SecMap Training
SecMapTrainerBase = register_module('VXDTFTrainingDataCollector')
SecMapTrainerBase.param('SpacePointTrackCandsName', 'checkedSPTCs')
path.add_module(SecMapTrainerBase)

# this can take quite long so it is good to know if it still running
path.add_module('Progress')

process(path)

# to show the settings of all modules (only those differing from default)
print_path(path)

print(statistics)
