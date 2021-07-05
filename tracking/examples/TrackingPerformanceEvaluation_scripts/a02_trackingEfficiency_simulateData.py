#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#################################################################
#                                                               #
#    script to simulate 10k Y(4S) events                        #
#                                                               #
# USAGE:
#
# reported in a01_trackingEfficiency_createData.py
#
#################################################################

import sys
import glob
import basf2 as b2
from simulation import add_simulation


b2.set_random_seed(1509)

release = sys.argv[1]

# roi = {noROI, vxdtf1, vxdtf2}
# bkg = {noBkg, stdBKG, std2GBKG}

roi = sys.argv[2]
bkg = sys.argv[3]

usePXDDataReduction = True

if (roi == 'noROI'):
    usePXDDataReduction = False

bkgFiles = None

if bkg == 'stdBKG':
    bkgFiles = glob.glob('/sw/belle2/bkg/*.root')
if bkg == 'std2GsmallBKG':
    bkgFiles = glob.glob('/sw/belle2/bkg/*.root')
    bkgFiles = bkgFiles + glob.glob('/sw/belle2/bkg.twoPhoton/*usual.root')
if bkg == 'std2GBKG':
    bkgFiles = glob.glob('/sw/belle2/bkg/*.root')
    bkgFiles = bkgFiles + glob.glob('/sw/belle2/bkg.twoPhoton/*.root')

print('background: ' + bkg)
if(usePXDDataReduction):
    print('simulating PXD Data Reduction')
else:
    print('NOT simulating PXD Data Reduction')
print('using vxdtf2')

print(release)

input_filename = './' + release + '/TV_data_' + release + '_1509.root'
output_filename = './' + release + '/TV_sim_' + bkg + '_' + roi + '_' + release + '.root'

print(output_filename)

path = b2.create_path()

progress = b2.register_module('Progress')

root_output = b2.register_module('RootOutput')
root_output.param('outputFileName', output_filename)

root_input = b2.register_module('RootInput')
root_input.param('inputFileName', input_filename)

path.add_module(root_input)
path.add_module(progress)

add_simulation(path, None, bkgFiles, None, 1.0, usePXDDataReduction)

path.add_module(root_output)

b2.process(path)
print(b2.statistics)
