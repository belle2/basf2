#!/usr/bin/env python
# -*- coding: utf-8 -*-

#################################################################
#                                                               #
#     run standard tracking on root files                       #
#                                                               #
#    written by Michael Ziegler, KIT                            #
#    michael.ziegler2@kit.edu                                   #
#                                                               #
#################################################################

from basf2 import *
from reconstruction import add_reconstruction
import glob

input_root_files = glob.glob('../trackingEfficiency_pt_*GeV.root')
print 'Tracking will run over these files: '
print input_root_files
print

path = create_path()

root_input = register_module('RootInput')
root_input.param('inputFileNames', input_root_files)
path.add_module(root_input)

gearbox = register_module('Gearbox')
path.add_module(gearbox)

components = ['MagneticFieldConstant4LimitedRCDC', 'BeamPipe', 'PXD', 'SVD',
              'CDC']
print components
print
geometry = register_module('Geometry')
geometry.param('Components', components)
path.add_module(geometry)

add_reconstruction(path, components)

output_file_name = '../trackingEfficiency_FinalData.root'
tracking_efficiency = register_module('StandardTrackingPerformance')
# tracking_efficiency.logging.log_level = LogLevel.DEBUG
tracking_efficiency.param('outputFileName', output_file_name)
path.add_module(tracking_efficiency)

process(path)

print statistics
