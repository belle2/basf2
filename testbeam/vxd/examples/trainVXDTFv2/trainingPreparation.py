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
from testbeam.utils import *


import argparse
parser = argparse.ArgumentParser(description="Training VXDTFv2 for DESY VXD Testbeam 2017")
parser.add_argument('--local-db', dest='local_db', action='store', default=None, type=str, help='Location of local db')
parser.add_argument(
    '--global-tag',
    dest='global_tag',
    action='store',
    default=None,
    type=str,
    help='Global tag to use at central DB in PNNL')
parser.add_argument(
    '--magnet-off',
    dest='magnet_off',
    action='store_const',
    const=True,
    default=False,
    help='Turn off magnetic field')
parser.add_argument(
    '--newGeo',
    dest='newGeo',
    action='store_const',
    const=True,
    default=False,
    help='Use if you want the updated geometry for after March 1st 2017')


parser.add_argument('--display', dest='display', action='store_const', const=True, default=False, help='Show Event Display window')
parser.add_argument('--momentum', dest='inputmomentum', action='store', default=6.5, type=float,
                    help='maximum momentum of particles generated. Default = 6.5 GeV/c')
parser.add_argument('--svd-only', dest='svd_only', action='store_const', const=True,
                    default=False, help='Use only SVD sector maps in VXDTF track finder')

args = parser.parse_args()

# ---------------------------------------------------------------------------------------
# Settings

# Logging and Debug Level
# TODO: Remove logLevel, as it can be set via basf2 option -l
set_log_level(LogLevel.ERROR)
log_to_file('logVXDTF2Preparation.log', append=False)


# ---------------------------------------------------------------------------------------
# Create paths
path = create_path()

path.add_module('Progress')


path.add_module('EventInfoSetter', evtNumList=[50000])

# add_geometry(path, magnet=not args.magnet_off, field_override=args.field, target=None)
if args.newGeo:
    geometry_xml = 'testbeam/vxd/2017_geometry_1.xml'
else:
    geometry_xml = 'testbeam/vxd/2017_geometry.xml'

print("used geometry: " + geometry_xml)

# removed the upstream scintilators, as they may produce curlers
add_geometry(path, magnet=not args.magnet_off, geometry_xml=geometry_xml,
             geometry_version=-666, excluded_components=['Scintilators'])


add_simulation(path, positrons=False)


# set some non default values for the particle gun
# using uniform momentum for the training between 2GeV and the input value (default 6.5GeV)
mom_min = 2  # minimum momentum used for training
if mom_min > args.inputmomentum:
    print("Warning: resetting the min momentum  " + str(args.inputmomentum))
    mom_min = args.inputmomentum
set_module_parameters(path, 'ParticleGun', recursive=False,
                      momentumParams=[mom_min, args.inputmomentum],
                      momentumGeneration='uniform',
                      zVertexGeneration='normal',
                      zVertexParams=[0.0, 3.0],
                      yVertexGeneration='normal',
                      yVertexParams=[0.0, 3.0],
                      thetaGeneration='normal',
                      thetaParams=[90.0, 0.1],
                      phiGeneration='normal',
                      phiParams=[0.0, 0.1]
                      )


add_svd_reconstruction(path, useNN=False, useCoG=True)
# add_clusterizer(path, args.svd_only)

# currently only support svd-only maps (svd + pxd still may work but has to be tested!)
usePXD = not args.svd_only
if usePXD:
    print("Currently the PXD is not supported!")
    exit()


# ---------------------------------------------------------------------------------------
# Setting up the MC based track finder.
mctrackfinder = register_module('TrackFinderMCTruthRecoTracks')
mctrackfinder.param('UseCDCHits', False)
mctrackfinder.param('UseSVDHits', True)
mctrackfinder.param('UsePXDHits', usePXD)
mctrackfinder.param('Smearing', False)
mctrackfinder.param('MinimalNDF', 6)
mctrackfinder.param('WhichParticles', ['primary'])
mctrackfinder.param('RecoTracksStoreArrayName', 'MCRecoTracks')
path.add_module(mctrackfinder)


eventinfoprinter = register_module('EventInfoPrinter')
path.add_module(eventinfoprinter)

# Event counter
eventCounter = register_module('EventCounter')
path.add_module(eventCounter)


# PXD not used, yet, as it is not supported.
if usePXD:
    spCreatorPXD = register_module('SpacePointCreatorPXD')
    # spCreatorPXD.param('NameOfInstance', 'SpacePointsCreatorPXD')
    spCreatorPXD.param('SpacePoints', 'SpacePoints')
    path.add_module(spCreatorPXD)

spCreatorSVD = register_module('SVDSpacePointCreator')
spCreatorSVD.param('OnlySingleClusterSpacePoints', False)
spCreatorSVD.param('MinClusterTime', -float('inf'))
# spCreatorSVD.param('NameOfInstance', 'SpacePointsCreatorSVD')
spCreatorSVD.param('SpacePoints', 'SpacePoints')
path.add_module(spCreatorSVD)


# Converts GenFit track candidates and checks them, with respect to the SecMap settings
# Produces SpacePoint TrackCand which is used in VXDTFTrainingDataCollector.
setup_RTCtoSPTCConverters(path=path,
                          SPscollection='SpacePoints',
                          RTCinput='MCRecoTracks',
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

# give the data an appropriate name
nameTag = "TB2017"
if args.magnet_off:
    nameTag = "MagnetOff"
else:
    nameTag = "MagnetOn"
if usePXD:
    nameTag += "VXD"
else:
    nameTag += "SVD"

if args.newGeo:
    nameTag += "_afterMarch1st"

# Module for generation of train sample for SecMap Training
SecMapTrainerBase = register_module('VXDTFTrainingDataCollector')
SecMapTrainerBase.param('SpacePointTrackCandsName', 'checkedSPTCs')
SecMapTrainerBase.param("NameTag", nameTag)
path.add_module(SecMapTrainerBase)

if args.display:
    path.add_module('Display', fullGeometry=True)


process(path)

print_path(path)

print(statistics)
