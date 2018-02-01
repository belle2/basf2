#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from svd.testbeam_utils import *
from svd import *
import argparse

##################################################################################
#
# script to run the 2017 TB SVD-only reconstruction
#
# to make it work
# 1. create a folder named local_rootfiles containing the root files for
#    RUN111: beam.0003.000111.root
#    RUN400: beam.0003.000400.root
#
# 2.  choose the reconstruction algorithm to be used in add_svd_reconstruction():
#     a. old TB reconstruction: useNN=False, useCoG=False, and uncomment SVDDigitSorter
#     b. Neural Networks: useNN=True, useCoG=False
#     c. Center of gravity: useNN=False, useCoG=True (DEFAULT)
#
# 3. execute the script like the following:
# RUN111: basf2 testbeam_evaluateSVD.py -- --magnet-off
# RUN400: basf2 testbeam_evaluateSVD.py
#
# a rootfile with performance histograms will be created
# uncomment the RootOutput line if you want to perform your own analysis
# on reconstructed data
#
##################################################################################


parser = argparse.ArgumentParser(description="SVD Reconstruction for DESY VXD Testbeam 2017")
parser.add_argument('--magnet-off', dest='magnet_off', action='store_const', const=True, default=False,
                    help='Turn off magnetic field')
args = parser.parse_args()

# define the DB Global Tag
globalTag = "beamtest_vxd_april2017_rev1"
reset_database()
use_database_chain()
use_central_database(globalTag)

# create the path
main = create_path()

# rootinput
rootin = register_module('RootInput')
if (args.magnet_off):
    rootin.param('inputFileNames', 'local_rootfile/beam.0003.000111.root')
else:
    rootin.param('inputFileNames', 'local_rootfile/beam.0003.000400.root')
rootin.param('branchNames', 'RawSVDs')
main.add_module(rootin)

# define geometry version
geom = 1
if (args.magnet_off):
    geom = 0

add_geometry(main, magnet=not args.magnet_off, field_override=None, target=None, geometry_version=geom)

# unpack data
main.add_module('SVDUnpacker', GenerateShaperDigits=True, shutUpFTBError=1)

# uncomment only if using the old TB reconstruction (SVDClusterizer)
# path.add_module('SVDDigitSorter')

# reconstruct SVD raw data
add_svd_reconstruction(main, useNN=False, useCoG=True)

# add SVD-only tracking
add_reconstruction(main, geometry_version=geom, magnet=not args.magnet_off, vxdtf2=True)


# add SVD performance module
svdperf = register_module('SVDPerformance')
if (args.magnet_off):
    svdperf.param('outputFileName', "SVDPerformance_VXDTF2_run111.root")
else:
    svdperf.param('outputFileName', "SVDPerformance_VXDTF2_run400.root")
svdperf.param('is2017TBanalysis', True)
main.add_module(svdperf)

# uncomment if you want to perform your own analysis on reconstructed data
# main.add_module('RootOutput')

main.add_module('Progress')
print_path(main)

process(main)

print(statistics)
