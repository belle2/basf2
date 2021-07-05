#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# -----------------------------------------------------------------------------------------------
# Simulation of local laser run with raw data output in sroot format.
# Output file mimics entirely the local data (format, time mis-calibrations etc)
# and can be used directly in run_channelT0_laser_calibration.py
#
# Usage: simulateLaserData.py experiment run [fileNumber] -n <num_of_events>
# -----------------------------------------------------------------------------------------------

from basf2 import create_path, process, statistics, conditions
import sys

# Argument parsing
argvs = sys.argv
if len(argvs) < 3:
    print("usage: basf2", argvs[0], "experiment run [fileNumber] -n <num_of_events>")
    sys.exit()
experiment = int(argvs[1])
run = int(argvs[2])
num = 'MC'
if len(argvs) > 3:
    num = 'MC-' + argvs[3]

# output file name
expRun = '{:0=4d}'.format(experiment) + '.' + '{:0=5d}'.format(run)
filename = f"top.{expRun}.{num}.sroot"

# global tag for run dependent simulation (the latest one recommended!)
conditions.append_globaltag('data_reprocessing_proc11')


def addLaserSource(x, angle, slotID, path):
    '''
    Adds a laser source to the path
    :param x local x coordinate of source in the local frame
    :param angle vertical tilt of the source
    :param slotID 1-16, slot number. If it's 0, then all the coorinates are in the BelleII frame
    :param path path
    '''
    path.add_module('OpticalGun',
                    minAlpha=0.0,  # not used if angulardistribution == 'Gaussian'
                    maxAlpha=33.0,  # not used if angulardistribution == 'Gaussian'
                    na=0.50,       # used only if angulardistribution == 'Gaussian'
                    startTime=-53.0,  # start time relative to the first cal pulse (according to run 8/414)
                    pulseWidth=10.0e-3,  # laser time jitter (1 Gaussian sigma), [ns]
                    numPhotons=62.5,  # according to run 8/414
                    diameter=10.0e-3,  # source diameter in cm
                    slotID=slotID,  # if nonzero, local (slot) frame, otherwise Belle II
                    x=x,
                    y=-3.26,
                    z=-131.33,
                    theta=180 + angle,
                    phi=0.0,
                    psi=0.0,
                    angularDistribution='uniform'
                    # angularDistribution='(40-x)*TMath::Sin(x)'
                    )


# Create path
main = create_path()

# Set number of events to generate
main.add_module('EventInfoSetter', expList=[experiment], runList=[run])

# Gearbox
main.add_module('Gearbox')

# Geometry
main.add_module('Geometry')

# Pulse generator
main.add_module('TOPCalPulseGenerator', asicChannels=[0])

# Optical sources
for slot in range(1, 17):
    for pos in [0.9, 5.7, 11.3, 16.9, 22.5, 28.1, 33.7, 39.3, 44.1]:
        angle = 17
        x = -45. / 2. + pos
        addLaserSource(x, angle, slot, main)

# Simulation
main.add_module('FullSim')

# Digitization
main.add_module('TOPDigitizer', lookBackWindows=28)

# Raw data packing
main.add_module('TOPPacker')

# Output in sroot format
main.add_module('SeqRootOutput', saveObjs=['EventMetaData', 'RawTOPs'], outputFileName=filename)

# Show progress of processing
main.add_module('Progress')

# Process events
process(main)

# Print call statistics
print(statistics)
