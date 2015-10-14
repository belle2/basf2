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

mctf = register_module('TrackFinderMCTruth')
mctf.param('GFTrackCandidatesColName', 'MCGFTrackCands')

geometry = register_module('Geometry')
path.add_module(geometry)

add_reconstruction(path)

path.add_module(mctf)
matcher = register_module('MCTrackMatcher')
path.add_module(matcher)

root_output = register_module('RootOutput')
root_output.param('outputFileName', output_file_name)
path.add_module(root_output)

modList = path.modules()
for modItem in modList:
    if modItem.name() == 'PruneGenfitTracks':
        modItem.m_flags = ''


process(path)

print(statistics)
