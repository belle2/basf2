#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file will
import os
from sys import argv
from time import time
from basf2 import *
from beamparameters import add_beamparameters
from testbeam.utils import *

import datetime
from ROOT import Belle2


class SelectROIs(Module):

    """A VERY simple module to discard events based on ROI size."""

    def __init__(self):
        """Initialize the module"""

        super(SelectROIs, self).__init__()

    def event(self):
        """Look at ROI sizes and search for a full frame ROI"""

        rois = Belle2.PyStoreArray('ROIs')
        is_fullframe = False
        for roi in rois:
            if (roi.getMaxVid() - roi.getMinVid()) > 760:
                is_fullframe = True

        self.return_value(is_fullframe)


import argparse
parser = argparse.ArgumentParser(description="PXD Efficiencies")
# parser.add_argument('--local-db', dest='local_db', action='store', default=None, type=str, help='Location of local db')
parser.add_argument(
    '--global-tag',
    dest='global_tag',
    action='store',
    default='beamtest_vxd_april2017_rev1',
    type=str,
    help='Global tag to use at central DB in PNNL')
parser.add_argument(
    '--magnet-off',
    dest='magnet_off',
    action='store_const',
    const=True,
    default=False,
    help='Turn off magnetic field')
parser.add_argument('--field', dest='field', action='store', default=1., type=float,
                    help='Magnetic field in Tesla. If different from 1. and magnet ON, the provided value is used')
parser.add_argument('--after341', dest='after_run341', action='store_const', const=True,
                    default=False, help='Use geometry with 4 PXD from run 341 onwards')
parser.add_argument('--momentum', dest='momentum', action='store', default=6., type=float,
                    help='Nominal momentum of particles (if magnet is off). Default = 6 GeV/c')
parser.add_argument('--pxd-tracking', dest='pxd_tracking', action='store_const', const=True,
                    default=False, help='Use Sector Maps including PXD in the track finding')
parser.add_argument(
    '--histo-file',
    dest='histo_file',
    action='store',
    default='efficiency.root',
    type=str,
    help='Name of output file with histograms and tree')
parser.add_argument('--no_alignment', dest='notUseAlignment', action='store_const', const=True,
                    default=False, help='Use Alignment for the runs.')
parser.add_argument('--sroot', dest='sroot', action='store_const', const=True,
                    default=False, help='Use sroot-Files as input')
parser.add_argument('--fullframe', dest='fullframe', action='store_const', const=True,
                    default=False, help='Take only events with full-frame rois')


args = parser.parse_args()

# if true alignment parameters will be applied to the trackfitting and space point transformation
useAlignment = not args.notUseAlignment

# magnet on or off
magnetOn = not args.magnet_off

set_log_level(LogLevel.ERROR)
# set_random_seed(initialValue)
set_random_seed(12345)


#
# Setup Modules #
#
histo = register_module('HistoManager')
histo.param('histoFileName', args.histo_file)

if args.sroot:
    roinput = register_module('SeqRootInput')
else:
    roinput = register_module('RootInput')
    # roinput.param('skipNEvents',0)


setupGenfit = register_module('SetupGenfitExtrapolation')
setupGenfit.param('useVXDAlignment', useAlignment)


# Unpacker
SVDUNPACK = register_module('SVDUnpacker')

SVDDIGISORTER = register_module('SVDDigitSorter')

# Clusterizer
SVDCLUST = register_module('SVDClusterizer')

# unpacker
PXDUNPACK = register_module('PXDUnpacker')
PXDUNPACK.param('IgnoreDATCON', True)
PXDUNPACK.param('RemapFlag', True)
PXDUNPACK.param('IgnoreFrameCount', True)
PXDUNPACK.param('IgnoreSorFlag', True)

# Digitizer
PXDDIGISORTER = register_module('PXDRawHitSorter')

PXDCLUST = register_module('PXDClusterizer')
"""
param_pxdclust = {
    'NoiseSN' : 2,
    'SeedSN'  : 2,
    'ClusterSN' : 2,
    'Gq'  : 0.5,
    'ADCFineMode' : False,
}
PXDCLUST.param(param_pxdclust)
"""

eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', 25)

daf = register_module('DAFRecoFitter')
daf.param('initializeCDCTranslators', False)
daf.logging.log_level = LogLevel.ERROR


track_creator = register_module('TrackCreator')
track_creator.param('beamSpot', [0., 0., 0.])
track_creator.param('pdgCodes', 11)
track_creator.logging.log_level = LogLevel.ERROR


pxdeffi = register_module('PXDEfficiency')
pxdeffi.param('distCut', 0.04)  # distance cut wrt. the track fit in cm
pxdeffi.param('otherLayerDistCut', 0.01)
pxdeffi.param('pxdclustersname', '')
pxdeffi.param('pxddigitsname', '')
pxdeffi.param('tracksname', 'RecoTracks')
pxdeffi.param('useAlignment', useAlignment)
pxdeffi.param('writeTree', True)
pxdeffi.param('ROIsName', 'ROIs')

pxdoccupancy = register_module('PXDOccupancy')
pxdoccupancy.param('distCut', 0.5)  # distance cut wrt. the track fit in cm
pxdoccupancy.param('pxdclustersname', '')
pxdoccupancy.param('pxddigitsname', '')
pxdoccupancy.param('tracksname', 'RecoTracks')
pxdoccupancy.param('useAlignment', useAlignment)
pxdoccupancy.param('writeTree', True)


# #################################################################################################
# ## creation of the path #########################################################################
# #################################################################################################

main = create_path()

reset_database()
use_database_chain()
# use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"), "", True)
use_central_database(args.global_tag, LogLevel.DEBUG)

# Add modules to paths
main.add_module(histo)

main.add_module(roinput)

dead_end = create_path()

if args.fullframe:
    discarder = register_module(SelectROIs())
    main.add_module(discarder)
    discarder.if_false(dead_end)

# Adds gearbox and geometry
if args.after_run341:
    add_geometry(
        main,
        magnet=not args.magnet_off,
        field_override=args.field,
        target=None,
        geometry_xml='testbeam/vxd/2017_geometry_1.xml',
        excluded_components=['Scintilators'],
        geometry_version=1)
else:
    add_geometry(
        main,
        magnet=not args.magnet_off,
        field_override=args.field,
        target=None,
        geometry_xml='testbeam/vxd/2017_geometry.xml',
        excluded_components=['Scintilators'],
        geometry_version=0)

main.add_module(setupGenfit)

# when running over the root files the svd cluster already exist!
main.add_module(SVDUNPACK)
main.add_module(SVDDIGISORTER)
main.add_module(SVDCLUST)

main.add_module(PXDUNPACK)

main.add_module(PXDDIGISORTER)
main.add_module(PXDCLUST)

main.add_module(eventCounter)

if args.sroot:
    main.add_module("ROIDQM", ROIsName="ROIs")
    main.add_module("PXDROIDQM")

useThisGeometry = 'TB2017'
if args.after_run341:
    useThisGeometry = 'TB2017newGeo'

add_vxdtf_v2(main,
             use_pxd=args.pxd_tracking,
             magnet_on=magnetOn,
             filter_overlapping=True,
             use_segment_network_filters=True,
             observerType=0,
             quality_estimator='circleFit',
             overlap_filter='greedy',  # 'hopfield' or 'greedy'
             log_level=LogLevel.ERROR,
             debug_level=1,
             usedGeometry=useThisGeometry
             )
main.add_module(daf)
main.add_module(track_creator)


main.add_module(pxdeffi)
# main.add_module(pxdoccupancy)


# print the list of modules
modList = main.modules()
print("\nList of used module")
for modItem in modList:
    print(modItem.type())

# Process events
process(main)


print('Event Statistics :')
print(statistics)
