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
#
# Contributors: Jonas Wagner, Felix Metzner
#####################################################################


from basf2 import *
from setup_modules import setup_RTCtoSPTCConverters


# ---------------------------------------------------------------------------------------
# Settings

# Logging and Debug Level
# TODO: Remove logLevel, as it can be set via basf2 option -l
set_log_level(LogLevel.ERROR)
log_to_file('logVXDTF2Preparation.log', append=False)


# ---------------------------------------------------------------------------------------
# Create paths
path = create_path()

# Input Module
rootInputM = register_module('RootInput')
path.add_module(rootInputM)

# Event Info Module
eventinfoprinter = register_module('EventInfoPrinter')
path.add_module(eventinfoprinter)

# Gearbox
gearbox = register_module('Gearbox')
path.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
geometry.param('components', ['BeamPipe',
                              'MagneticFieldConstant4LimitedRSVD',
                              'PXD',
                              'SVD'])
path.add_module(geometry)

# Event counter
eventCounter = register_module('EventCounter')
path.add_module(eventCounter)


# PXD not used, yet, as it is not supported.
usePXD = False
if usePXD:
    spCreatorPXD = register_module('SpacePointCreatorPXD')
    spCreatorPXD.param('NameOfInstance', 'SpacePoints')
    spCreatorPXD.param('SpacePoints', 'SpacePoints')
    path.add_module(spCreatorPXD)

spCreatorSVD = register_module('SpacePointCreatorSVD')
spCreatorSVD.param('OnlySingleClusterSpacePoints', False)
spCreatorSVD.param('NameOfInstance', 'SpacePoints')
spCreatorSVD.param('SpacePoints', 'SpacePoints')
path.add_module(spCreatorSVD)


# Converts GenFit track candidates and checks them, with respect to the SecMap settings
# Produces SpacePoint TrackCand which is used in VXDTFTrainingDataCollector.
setup_RTCtoSPTCConverters(path=path,
                          pxdSPs='SpacePoints',
                          svdSPs='SpacePoints',
                          RTCinput='mcTracks',
                          sptcOutput='checkedSPTCs',
                          usePXD=usePXD,
                          logLevel=LogLevel.WARNING)


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


process(path)
print(statistics)
