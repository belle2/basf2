#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# This steering file is used to measure the execution time
# for high or low multiplicity events
#############################################################

import basf2 as b2
from generators import add_kkmc_generator
from simulation import add_simulation
from validation_gt import get_validation_globaltags
from rawdata import add_unpackers
from reconstruction import add_reconstruction
from argparse import ArgumentParser
import os
import glob
import sys

# parse command line options
parser = ArgumentParser(description='Measure the execution time.')
parser.add_argument('-m', '--multiplicity', default='high', choices=['high', 'low', 'data'], help='Multiplicity or type of events')
parser.add_argument('-l', '--limits', help='Name of file containing limits')
parser.add_argument('-f', '--file', help='Name of benchmark output file')
args = parser.parse_args()

# create path and reduce log level
main = b2.create_path()
b2.set_log_level(b2.LogLevel.ERROR)

if args.multiplicity == 'data':
    # Global Tag needed as these are Raw data
    b2.conditions.override_globaltags(get_validation_globaltags())

    input_files = glob.glob(os.environ.get('BELLE2_VALIDATION_DATA_DIR', '') + '/rawdata/physics.0010.05095*.root')
    main.add_module("RootInput", inputFileNames=input_files)

    main.add_module('EventInfoPrinter').set_log_level(b2.LogLevel.INFO)

    # gearbox and geometry
    main.add_module('Gearbox')
    main.add_module('Geometry', useDB=True)

    # unpacking
    add_unpackers(main)

else:
    # specify number of events to be generated
    main.add_module('EventInfoSetter', evtNumList=[1000])
    main.add_module('EventInfoPrinter').set_log_level(b2.LogLevel.INFO)

    if args.multiplicity == 'high':
        # generate BBbar events if high multiplicity is selected
        main.add_module('EvtGenInput')

    elif args.multiplicity == 'low':
        # generate mu pair events if low multiplicity is selected
        add_kkmc_generator(main, 'mu+mu-')

    # detector and L1 trigger simulation
    add_simulation(main, bkgfiles=glob.glob(os.environ.get('BELLE2_BACKGROUND_DIR', '/sw/belle2/bkg') + '/*.root'))

# reconstruction
add_reconstruction(main)

# process events and print call statistics
b2.process(main)
print(b2.statistics)

# read limits
limits = {}
limits_file = args.limits
default_limits_file = args.multiplicity + '.limits'
if limits_file is None and os.path.isfile(default_limits_file):
    limits_file = default_limits_file
if limits_file is not None:
    for line in open(limits_file).readlines():
        entries = line.split()
        limits[entries[0]] = float(entries[1])
        if len(entries) > 2:
            limits[entries[0]] /= float(entries[2])

# get execution times
categories = [
    'Simulation',
    'TriggerSimulation',
    'Clustering',
    'Prefilter_Tracking',
    'Posttracking_Reconstruction',
    'Postfilter_Reconstruction']
times = {}
for module in b2.statistics.modules:
    if module.name not in ['Sum_' + category for category in categories]:
        continue
    category = module.name[4:]
    if category not in times.keys():
        times[category] = 0
    times[category] += module.time_mean(b2.statistics.EVENT) * 1e-6

# open output file
output = None
if args.file is not None:
    output = open(args.file, 'wt')

# print benchmark results and write them to the output file
b2.set_log_level(b2.LogLevel.INFO)
max_fraction = -1
for category in categories:
    if category not in times.keys():
        continue
    time = times[category]
    message = 'Execution time per event for %s is %.f ms' % (category, time)
    fraction = -1
    if category in limits.keys():
        fraction = time / limits[category]
        if fraction > max_fraction:
            max_fraction = fraction
        message += ' = %.f%% of the limit.' % (100 * fraction)
        if fraction <= 0.9:
            b2.B2INFO(message)
        elif fraction <= 1:
            b2.B2WARNING(message)
        else:
            b2.B2ERROR(message)
    else:
        b2.B2INFO(message)

    if output is not None:
        output.write('%s %.2f' % (category, time))
        if fraction >= 0:
            output.write(' %.4f' % fraction)
        output.write('\n')

# fail if above limit
sys.exit(0 if max_fraction <= 1 else 1)
