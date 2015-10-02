#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
# This steering file runs the DedxScanModule, which
# loops over CDC wires and creates dE/dx drift cells
# for debugging purposes. No particles are generated.
#
# Example steering file - 2011 Belle II Collaboration
######################################################

import os
from basf2 import *

# register necessary modules
eventinfosetter = register_module('EventInfoSetter')

# generate one event
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [1])

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
geometry.param('excludedComponents', ['EKLM'])

genfit = register_module('SetupGenfitExtrapolation')

# create paths
main = create_path()

# add modules to paths
main.add_module(eventinfosetter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(genfit)

dedx = register_module('DedxScan')
main.add_module(dedx)

output = register_module('RootOutput')
output.param('outputFileName', 'dedxPID_scan.root')
output.param('branchNames', 'DedxTracks')
main.add_module(output)
process(main)
print(statistics)
