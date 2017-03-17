#!/usr/bin/env python
# -*- coding: utf-8 -*-

#################################################################
#                                                               #
#    run standard tracking on root files                        #
#                                                               #
#    written by Giulia Casarosa, Pisa                           #
#    giulia.casarosa@pi.infn.it                                 #
#                                                               #
#################################################################

import sys
from basf2 import *
from reconstruction import add_reconstruction
from tracking import *

mcTrackFinding = False

release = sys.argv[2]

input_root_files = './' + release + '/TV_data_' + release + '_' + sys.argv[1] + '.root'
output_file_name = './' + release + '/TV_reco_' + release + '_' + sys.argv[1] + '.root'

print('Tracking will run over these files: ')
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

add_tracking_reconstruction(
    path,
    components=None,
    pruneTracks=False,
    mcTrackFinding=mcTrackFinding,
    trigger_mode="all",
    skipGeometryAdding=False
)

modList = path.modules()
for modItem in modList:
    if modItem.name() == 'V0Finder':
        modItem.param('Validation', True)


v0matcher = register_module('MCV0Matcher')
v0matcher.param('V0ColName', 'V0ValidationVertexs')
v0matcher.logging.log_level = LogLevel.INFO
path.add_module(v0matcher)

progress = register_module('Progress')
path.add_module(progress)

root_output = register_module('RootOutput')
root_output.param('outputFileName', output_file_name)
path.add_module(root_output)

process(path)

print(statistics)
