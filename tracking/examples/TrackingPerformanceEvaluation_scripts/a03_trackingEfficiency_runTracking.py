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
#    run standard tracking on root files                        #
#                                                               #
#    written by Giulia Casarosa, Pisa                           #
#    giulia.casarosa@pi.infn.it                                 #
#                                                               #
# USAGE:
#
# reported in a01_trackingEfficiency_createData.py
#
#################################################################

import sys
import basf2 as b2
from tracking import add_tracking_reconstruction

mcTrackFinding = False

release = sys.argv[1]

# roi = {noROI, vxdtf1, vxdtf2}
# bkg = {noBkg, stdBKG, std2GBKG}
# vxdtf = {vxdtf1, vxdtf2}

roi = sys.argv[2]
bkg = sys.argv[3]
vxdtf = sys.argv[4]

input_root_files = './' + release + '/TV_sim_' + bkg + '_' + roi + '_' + release + '.root'
output_file_name = './' + release + '/TV_reco_' + bkg + '_' + roi + '_' + vxdtf + '_' + release + '.root'

print('Tracking will run over these files: ')
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
path.add_module(geometry)

add_tracking_reconstruction(
    path,
    components=None,
    pruneTracks=False,
    mcTrackFinding=mcTrackFinding,
    skipGeometryAdding=False
)

modList = path.modules()
for modItem in modList:
    if modItem.name() == 'V0Finder':
        modItem.param('Validation', True)


v0matcher = b2.register_module('MCV0Matcher')
v0matcher.param('V0ColName', 'V0ValidationVertexs')
v0matcher.logging.log_level = b2.LogLevel.INFO
path.add_module(v0matcher)


path.add_module('Progress')

root_output = b2.register_module('RootOutput')
root_output.param('outputFileName', output_file_name)
path.add_module(root_output)

b2.process(path)

print(b2.statistics)
