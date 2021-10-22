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
#
# Test script to produce Tracking DQM plots from raw data
#
# Usage: basf2 runTrackingDQM.py -i "<path/to/file.root>" exp[4] run[5] runtype[cosmic, debug] dqm_env[hlt, expressreco]
#
# the rootfiles MUST contain RAW DATA
# you need the dqm (and daq) packages
#
#############################################################

import basf2 as b2
import tracking as trk
import rawdata as raw
import daqdqm.commondqm as cdqm
import sys

# main
main = b2.create_path()

# read input rootfile
# -> can be overwritten with the -i option
main.add_module("RootInput")

# register the HistoManager and specify output file
main.add_module("HistoManager", histoFileName="TrackingDQM_e" + str(sys.argv[1]) + "r" + str(sys.argv[2]) + ".root")

# nee to know geometry to create histograms
main.add_module('Gearbox')
main.add_module('Geometry')

# unpack
raw.add_unpackers(main, components=['PXD', 'SVD', 'CDC'])

trk.add_tracking_reconstruction(main, components=['PXD', 'SVD', 'CDC'])

# add DQM - official way
# cdqm.add_common_dqm(main,components=['SVD'], dqm_environment=sys.argv[4])

# add DQM - test both modules at once
main.add_module('TrackingHLTDQM')
main.add_module('TrackingExpressRecoDQM')

# == Show progress
main.add_module('Progress')

b2.print_path(main)

# Process events
b2.process(main)

print(b2.statistics)
