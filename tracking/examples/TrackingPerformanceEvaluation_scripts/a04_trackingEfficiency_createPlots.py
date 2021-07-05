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
#    produce tracking validation plots                          #
#                                                               #
# USAGE:
#
# reported in a01_trackingEfficiency_createData.py
#
#################################################################

import sys
import basf2 as b2

release = sys.argv[1]

# roi = {noROI, vxdtf1, vxdtf2}
# bkg = {noBkg, stdBKG, std2GBKG}
# vxdtf = {vxdtf1, vxdtf2}

roi = sys.argv[2]
bkg = sys.argv[3]
vxdtf = sys.argv[4]


input_root_files = './' + release + '/TV_reco_' + bkg + '_' + roi + '_' + vxdtf + '_' + release + '.root'
root_file_name_TRK = './' + release + '/TV_TRK_analysis_' + bkg + '_' + roi + '_' + vxdtf + '_' + release + '.root'
root_file_name_V0 = './' + release + '/TV_V0_analysis_' + bkg + '_' + roi + '_' + vxdtf + '_' + release + '.root'

print('Tracking Validation will run over these files: ')
print(input_root_files)
print('simulation: ' + roi + ' ' + bkg)
print('reconstruction ' + vxdtf)
print()

path = b2.create_path()

root_input = b2.register_module('RootInput')
root_input.param('inputFileNames', input_root_files)
path.add_module(root_input)

gearbox = b2.register_module('Gearbox')
path.add_module(gearbox)

geometry = b2.register_module('Geometry')
# geometry.param('components',['PXD','SVD','CDC'])
path.add_module(geometry)

create_plots_TRK = b2.register_module('TrackingPerformanceEvaluation')
create_plots_TRK.param('outputFileName', root_file_name_TRK)
create_plots_TRK.logging.log_level = b2.LogLevel.INFO
path.add_module(create_plots_TRK)

create_plots_V0 = b2.register_module('V0findingPerformanceEvaluation')
create_plots_V0.param('outputFileName', root_file_name_V0)
create_plots_V0.logging.log_level = b2.LogLevel.INFO
path.add_module(create_plots_V0)

path.add_module('Progress')
b2.process(path)

print(b2.statistics)
