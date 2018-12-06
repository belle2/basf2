#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from svd.testbeam_utils import *
from svd import *
import argparse

##################################################################################
#
# script to run the 2017 TB SVD-only simulation
#
# 1. execute the script like the following:
# simulation of RUN111: basf2 testbeam_simulate.py -- --magnet-off
# simulation of RUN400: basf2 testbeam_simulate.py
#
# a rootfile containing SVDShaperDigits (and other dataobjects) will be produced
#
##################################################################################


parser = argparse.ArgumentParser(description="SVD Simulation for DESY VXD Testbeam 2017")
parser.add_argument('--magnet-off', dest='magnet_off', action='store_const', const=True, default=False,
                    help='Turn off magnetic field')
parser.add_argument('--reconstruct', dest='reconstruct', action='store_const', const=True, default=False,
                    help='Do SVD reconstruction')
args = parser.parse_args()

'''
NOT USED FOR THE MOMENT
# define the DB Global Tag
globalTag = "beamtest_vxd_april2017_rev1"
reset_database()
use_database_chain()
use_central_database(globalTag)
'''

# create the path
main = create_path()

geom = 1
filename = 'TB2017Simulation_magnetON.root'
run = 400
if (args.magnet_off):
    geom = 0
    filename = 'TB2017Simulation_magnetOFF.root'
    run = 111

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [run])
main.add_module(eventinfosetter)
main.add_module('EventInfoPrinter')

# define geometry version

add_geometry(main, magnet=not args.magnet_off, field_override=None, target=None, geometry_version=geom)


# simulate
add_simulation(main)

# uncomment if you want to recontruct too
if (args.reconstruct):
    add_svd_reconstruction(main, useNN=False, useCoG=True)

# save the output
main.add_module('RootOutput', outputFileName=filename)

main.add_module('Progress')
print_path(main)

process(main)

print(statistics)
