#!/usr/bin/env python
# -*- coding: utf-8 -*-

######################################################## This steering file
# shows all options for the generation of radiative Bhabha scattering.
#
# 10000 radiative Bhabha events are generated using the BHWide Fortran
# generator and some plots are shown at the end.
#
# Example steering file - 2011 Belle II Collaboration
########################################################

import sys
import math
from basf2 import *

## Create main path
main = create_path()

## input
roiname = sys.argv[1]
roinput = register_module('RootInput')
roinput.param('inputFileName', roiname)
main.add_module(roinput)

## Set the global log level
logging.log_level = LogLevel.WARNING

##
rooname = sys.argv[2]
ecldataanalysis = register_module('ECLDataAnalysis')
ecldataanalysis.param('rootFileName', rooname)
ecldataanalysis.param('doTracking', 0)
main.add_module(ecldataanalysis)
## Register the Progress module and the Python histogram module
# progress = register_module('Progress')
# main.add_module(paramloader)

##
process(main)

##
print statistics

