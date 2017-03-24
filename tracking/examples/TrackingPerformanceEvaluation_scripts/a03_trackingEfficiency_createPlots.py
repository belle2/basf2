#!/usr/bin/env python
# -*- coding: utf-8 -*-

#################################################################
#                                                               #
#    produce tracking validation plots                          #
#                                                               #
#    written by Giulia Casarosa, Pisa                           #
#    giulia.casarosa@pi.infn.it                                 #
#                                                               #
#################################################################

import sys
from basf2 import *
from ROOT import Belle2

release = sys.argv[1]
input_root_files = ['./' + release + '/TV_reco_' + release + '.root']
root_file_name_TRK = './' + release + '/TV_TRK_analysis_' + release + '.root'
root_file_name_V0 = './' + release + '/TV_V0_analysis_' + release + '.root'

print('Tracking Validation will run over these files: ')
print(input_root_files)
print('(full simulation)')
print()

path = create_path()

root_input = register_module('RootInput')
root_input.param('inputFileNames', input_root_files)
path.add_module(root_input)

gearbox = register_module('Gearbox')
path.add_module(gearbox)

geometry = register_module('Geometry')
path.add_module(geometry)

create_plots_TRK = register_module('TrackingPerformanceEvaluation')
create_plots_TRK.param('outputFileName', root_file_name_TRK)
create_plots_TRK.logging.log_level = LogLevel.INFO
path.add_module(create_plots_TRK)

create_plots_V0 = register_module('V0findingPerformanceEvaluation')
create_plots_V0.param('outputFileName', root_file_name_V0)
create_plots_V0.logging.log_level = LogLevel.INFO
path.add_module(create_plots_V0)

process(path)

print(statistics)
