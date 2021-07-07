#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

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
#####################################################################


import basf2 as b2
from setup_modules import setup_RTCtoSPTCConverters
import argparse
import os

# ---------------------------------------------------------------------------------------
# Argument parser to enable training sample selection via comandline option.
parser = argparse.ArgumentParser(description='Training sample preparation:\
                                     Prepare a data sample to train the sector map.\n\
                                     Usage: basf2 traininPreparation.py -i dataSample.root -- --enable_selection boolean')
parser.add_argument(
    '--enable_selection',
    dest='use_NoKick',
    action='store_const',
    const=True,
    default=False,
    help='enable the selection of training sample based on track parameters')

parser.add_argument(
    '--disable_checkFit',
    dest='checkFit',
    action='store_const',
    const=False,
    default=True,
    help="By default only RecoTracks with valid fit are taken for training. Using this option will disable that. ")

arguments = parser.parse_args()
use_noKick = arguments.use_NoKick


# ---------------------------------------------------------------------------------------
# Settings

# Logging and Debug Level
# TODO: Remove logLevel, as it can be set via basf2 option -l
b2.set_log_level(b2.LogLevel.ERROR)
b2.log_to_file('logVXDTF2Preparation.log', append=False)
# if false PXD hits will be ignored in the trainings data collection
# Currently we dont do PXD tracking with vxdtf2 (as of 15.02.2018)
usePXD = False

# ---------------------------------------------------------------------------------------
# Create paths
path = b2.create_path()


# Input Module
rootInputM = b2.register_module('RootInput')
path.add_module(rootInputM)

# Event Info Module
eventinfoprinter = b2.register_module('EventInfoPrinter')
path.add_module(eventinfoprinter)

path.add_module("PrintCollections", printForEvent=1)


# puts the geometry and gearbox in the path
gearbox = b2.register_module('Gearbox')
path.add_module(gearbox)
# the geometry is loaded from the DB by default now! The correct geometry
# should be pickked according to exp number for the generated events.
geometry = b2.register_module('Geometry')
path.add_module(geometry)

# Event counter
# eventCounter = register_module('EventCounter')
# path.add_module(eventCounter)


# put PXD and SVD SpacePoints into the same StoreArray
if usePXD:
    spCreatorPXD = b2.register_module('PXDSpacePointCreator')
    spCreatorPXD.param('NameOfInstance', 'PXDSpacePointCreator')
    spCreatorPXD.param('SpacePoints', 'PXDSpacePoints')
    path.add_module(spCreatorPXD)

spCreatorSVD = b2.register_module('SVDSpacePointCreator')
spCreatorSVD.param('OnlySingleClusterSpacePoints', False)
spCreatorSVD.param('NameOfInstance', 'SVDSpacePointCreator')
spCreatorSVD.param('SpacePoints', 'SVDSpacePoints')
path.add_module(spCreatorSVD)


# Converts GenFit track candidates and checks them, with respect to the SecMap settings
# Produces SpacePoint TrackCand which is used in VXDTFTrainingDataCollector.
setup_RTCtoSPTCConverters(path=path,
                          SVDSPscollection='SVDSpacePoints',
                          PXDSPscollection='PXDSpacePoints',
                          RTCinput='MCRecoTracks',
                          sptcOutput='checkedSPTCs',
                          usePXD=usePXD,
                          logLevel=b2.LogLevel.ERROR,
                          useNoKick=use_noKick,
                          useOnlyFittedTracks=True)  # train on fitted tracks only


# SecMap BootStrap
# Module to fetch SecMap Config and store or load SecMap Training.
# Config is defined in /tracking/modules/vxdtfRedesing/src/SectorMapBootstrapModule.cc
# and must be available for the training of the SecMap
# Double False only fetches config.
secMapBootStrap = b2.register_module('SectorMapBootstrap')
secMapBootStrap.param('ReadSectorMap', False)
secMapBootStrap.param('WriteSectorMap', False)
path.add_module(secMapBootStrap)


# Module for generation of train sample for SecMap Training
nameTag = 'Belle2'
if os.environ.get('USE_BEAST2_GEOMETRY'):
    nameTag = 'Beast2'

if usePXD:
    nameTag += '_VXD'
else:
    nameTag += '_SVDOnly'

#
SecMapTrainerBase = b2.register_module('VXDTFTrainingDataCollector')
SecMapTrainerBase.param('NameTag', nameTag)
SecMapTrainerBase.param('SpacePointTrackCandsName', 'checkedSPTCs')
# SecMapTrainerBase.logging.log_level = LogLevel.DEBUG
path.add_module(SecMapTrainerBase)

# this can take quite long so it is good to know if it is still running
path.add_module('Progress')

path.add_module("PrintCollections", printForEvent=1)

b2.process(path)

# to show the settings of all modules (only those differing from default)
b2.print_path(path)

print(b2.statistics)
