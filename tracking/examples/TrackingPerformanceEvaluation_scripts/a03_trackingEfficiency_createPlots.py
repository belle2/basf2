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
input_root_files = ['./' + release + '/TV_reco_' + release + '_*.root']
root_file_name = './' + release + '/TV_analysis_' + release + '_10k.root'

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

create_plots = register_module('TrackingPerformanceEvaluation')
create_plots.param('outputFileName', root_file_name)
create_plots.logging.log_level = LogLevel.INFO
path.add_module(create_plots)

process(path)

print(statistics)
