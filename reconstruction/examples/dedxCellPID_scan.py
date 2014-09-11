#!/usr/bin/env python
# -*- coding: utf-8 -*-

######################################################
# This steering file generates several D* events
# and does a simple dE/dx measurement with path
# correction. The results are stored in a ROOT file.
#
# Example steering file - 2011 Belle II Collaboration
######################################################

import os
from basf2 import *

# change to True if you want to use PXD hits (fairly small benefit, if any)
use_pxd = False

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

# create paths
main = create_path()

# add modules to paths
main.add_module(eventinfosetter)
main.add_module(gearbox)
main.add_module(geometry)

dedx = register_module('DedxScan')
main.add_module(dedx)

output = register_module('RootOutput')
output.param('outputFileName', 'dedxCellPID_scan.root')
# output.param('branchNames','DedxCells')
main.add_module(output)
process(main)
print statistics
