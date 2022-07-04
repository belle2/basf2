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
# VXDTF2 Example Scripts - Step 0 - Simulation
#
# This script can be used to produce MC data from particle guns or
# Y(4S) events for the training and validation of the VXDTF 1 and 2.
#
# The number of events which will be simulated can be set via the
# basf2 commandline option -n.
# The name of the root output file can be defined with the option -o.
#
# E.g.: 'basf2 eventSimulation.py -n 1000 -o trainingSample.root'
#
# The settings for the particle gun(s) and EvtGen simulation can be
# adapted in this script. Some convenience functions are outsourced
# to setup_modules.py.
#
# The script takes two optional command line arguments: the first will
# be interpreted as random seed, the second as directory for the output.
# e.g: basf2 'eventSimulation.py 12354 ./datadir/'
# will result in setting the random seed to 12354 and the output will
# be written to './datadir/'
#####################################################################

# default basf2 stuff
import basf2 as b2
# convenience functions to set up the SectorMap training
from SectorMapTrainingUtils import add_event_generation, add_simulation_and_reconstruction_modules
from SectorMapTrainingUtils import add_rootoutput, add_training_data_collector
import argparse

# Argument parser to enable training sample selection via comandline option.
parser = argparse.ArgumentParser(description='Training sample preparation:\
                                     Prepare a data sample to train the sector map.\n\
                                     Usage: basf2 CollectTrainingsData.py  -- --enable_selection boolean')

# TODO: add rest of the parameters. So far only those which are necessarry (except a few random ones)

parser.add_argument(
    '--enable_selection',
    dest='use_NoKick',
    action='store_const',
    const=True,
    default=False,
    help='Enable the selection of training sample based on track parameters (experimental)')

parser.add_argument(
    '--usePXD',
    dest='usePXD',
    action='store_const',
    const=True,
    default=False,
    help="By default PXD is ignored during training. Set to True so also PXD data is collected. ")

parser.add_argument(
    '--rndSeed',
    type=int,
    default=123456,
    dest="rndSeed",
    help="Sets the random seed used for this script")

parser.add_argument(
    '--eventType',
    type=str,
    default='BBbar',
    dest="eventType",
    help="Set the event type which is generated. As for now allowed options are \"BBbar\" and "
    "\"BhaBha\" ")

parser.add_argument(
    '--outputDir',
    type=str,
    default='./',
    dest="outputDir",
    help="Set the output directory. All output will be written to that directory (make sure it exists)")


# the arguments for this scripts
args = parser.parse_args()


b2.B2INFO("Using random Seed: " + str(args.rndSeed))


main = b2.create_path()


# Adds either BB or ee event generation (no detector simulation)
add_event_generation(path=main, randomSeed=args.rndSeed, eventType=args.eventType, expNumber=0)

# adds simulation and reconstruction as needed for the training
# for now we dont need the PXD
add_simulation_and_reconstruction_modules(path=main, usePXD=args.usePXD)

# make root output optional, as its a lot!!
if False:
    add_rootoutput(path=main, outputFileName=args.outputDir + "/RootOutput_" + args.eventType + str(args.rndSeed) + ".root")

# add the actual data collection
add_training_data_collector(path=main, usePXD=args.usePXD, nameTag=args.eventType +
                            str(args.rndSeed), outputDir=args.outputDir + "/")

# dump some logging
b2.log_to_file(args.outputDir + '/createSim_' + args.eventType + str(args.rndSeed) + '.log', append=False)


b2.print_path(main)

main.add_module("Progress")

b2.process(main)
print(b2.statistics)
