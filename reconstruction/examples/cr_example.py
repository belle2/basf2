#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#############################################################
# This steering file reconstructs events from the CDC CR sample.
#
# Usage: basf2 cr_example.py
#
# Input: CDC CR data after unpacker
# Output: cr_output.root
#
# Example steering file - 2011 Belle II Collaboration
#############################################################

import os

import basf2 as b2
from reconstruction import add_cosmics_reconstruction
from cdc import cr

# use the CDC CR tag
b2.use_central_database("cdc_cr_test1", b2.LogLevel.WARNING)

# create path
main = b2.create_path()

# specify number of events to be generated
rootinput = b2.register_module('RootInput')
input = '/ghi/fs01/belle2/bdata/group/detector/CDC/unpacked/exp00/cr.cdc.0000.001733.root'
rootinput.param('inputFileName', input)
main.add_module(rootinput)

# this sample is for CDC cosmics only
components = ['CDC']

# get the run number
run_number = cr.getRunNumber(input)

# Set the peiod of data taking.
data_period = cr.getDataPeriod(run_number)

# create a working directory for this sample
if os.path.exists(data_period) is False:
    os.mkdir(data_period)

# gearbox & geometry needs to be registered
globalPhiRotation = cr.getPhiRotation()
main.add_module('Gearbox',
                override=[("/DetectorComponent[@name='CDC']//GlobalPhiRotation", str(globalPhiRotation), "deg")
                          ],
                fileName='/geometry/Beast2_phase2.xml')
main.add_module('Geometry', components=components)

# Set CDC CR parameters.
cr.set_cdc_cr_parameters(data_period)

# reconstruction
add_cosmics_reconstruction(main, components=components)

# full output
main.add_module('RootOutput', outputFileName='cr_output.root')

# display a progress bar while running
main.add_module('ProgressBar')

# process events and print call statistics
b2.process(main)
print(b2.statistics)
