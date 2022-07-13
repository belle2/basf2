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
# VXDTF2 Script to write out the training data needed to train a SectorMap
#
# This script can be used to produce MC data. Two types of events can be
# generated: 1) "BBbar" which are Y(4S) events mixed with ParticleGun tracks.
#            2) "BhaBha" which ee->ee events mixed with ParticleGun tracks
#    The event type can be set with the "--eventType" option followed by
#    one of the two options above ("BBbar" or "BhaBha")
#
# The number of events which will be simulated can be set via the
# basf2 commandline option -n.
#
# There are tow types of outputs:
#    1) The full root output which can be used to study the trainings data
#       collection or the generated events. These events have to undergo an
#       additional step to be usable for the training of a SectorMap
#       (see SectorMapTrainingUtils.add_training_data_collector) This option
#       is by default deactivated as these files are very big and not needed for
#       the actual training. To enable it use the option
#       "--enableFullRootOutput".
#    2) Root files where only the data is stored which is needed to perform
#       the training. For 10Mio BBbar plus 2Mio BhaBha events (default training)
#       this will generate approximately 100GB of output (log files + root files).
#
# All output will be written to the output directory (default "./"). The output
# directory can be set with the option "--outputDir". The names of the output files
# are generated automatically using the random seed and the type of generated events.
# The random seed can be set with the option "--rndSeed".
#
# By default no PXD data are collected during the data collection. To enable it use
# the option "--usePXD"
#
# The settings for the particle gun(s) and EvtGen simulation have to be
# adapted in the convenience functions defined in SectorMapTrainingUtils.py
#
# By default the user should specify the random seed, the output directory, and the
# event type to be generated. An example call generating 1000 BBbar events is given below:
# basf2  -n 1000 CollectTrainingsData.py -- --rndSeed 12345 --outputDir "./outdir/" --eventType "BBbar"
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

parser.add_argument(
    '--enableFullRootOutput',
    dest='fullRootOutput',
    action='store_const',
    const=True,
    default=False,
    help="If this option is called the full root output is enabled. WARNING: this will generate large files!")

# the arguments for this scripts
args = parser.parse_args()


b2.B2INFO("Using random Seed: " + str(args.rndSeed))


main = b2.create_path()


# Adds either BB or ee event generation (no detector simulation)
add_event_generation(path=main, randomSeed=args.rndSeed, eventType=args.eventType, expNumber=0)

# adds simulation and reconstruction as needed for the training
# for now we dont need the PXD
add_simulation_and_reconstruction_modules(path=main, usePXD=args.usePXD)

# full root output. Optional, as its a lot!!
if args.fullRootOutput:
    add_rootoutput(path=main, outputFileName=args.outputDir + "/RootOutput_" + args.eventType + str(args.rndSeed) + ".root")

# add the actual data collection
add_training_data_collector(path=main, usePXD=args.usePXD, nameTag=args.eventType +
                            str(args.rndSeed), outputDir=args.outputDir + "/")

# dump some logging
b2.log_to_file(args.outputDir + '/createSim_' + args.eventType + str(args.rndSeed) + '.log', append=False)

# show path before starting (useful for debugging)
b2.print_path(main)

main.add_module("Progress")

b2.process(main)
print(b2.statistics)
