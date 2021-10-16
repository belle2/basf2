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
from basf2 import conditions as b2conditions
import tracking as trk
import rawdata as raw
import daqdqm.commondqm as cdqm
import sys

b2conditions.override_globaltags()
b2conditions.globaltags = ['online', 'dp_recon_release6_patch']

# main main
main = b2.create_path()

# RAW
files = ['/group/belle2/dataprod/Data/Raw/e' +
         str(sys.argv[1]) +
         '/r' +
         str(sys.argv[2]) +
         '/sub00/' +
         str(sys.argv[3]) +
         '.' +
         str(sys.argv[1]) +
         '.' +
         str(sys.argv[2]) +
         '.HLT1.f00001.root']

# read input rootfile
# -> can be overwritten with the -i option
main.add_module("RootInput", inputFileNames=files)

# register the HistoManager and specify output file
main.add_module("HistoManager", histoFileName="TrackingDQM_e" + str(sys.argv[1]) + "r" + str(sys.argv[2]) + ".root")

# nee to know geometry to create histograms
main.add_module('Gearbox')
main.add_module('Geometry')

# unpack
raw.add_unpackers(main, components=['PXD', 'SVD', 'CDC'])

trk.add_tracking_reconstruction(main, components=['PXD', 'SVD', 'CDC'])

# add DQM
# cdqm.add_common_dqm(main,components=['SVD'], dqm_environment=sys.argv[4])
main.add_module('TrackingHLTDQM')
main.add_module('TrackingERDQM')

# == Show progress
main.add_module('Progress')

b2.print_path(main)

# Process events
b2.process(main)

print(b2.statistics)
