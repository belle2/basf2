#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# BASF2 (Belle Analysis Framework 2)                                     *
# Copyright(C) 2011 - Belle II Collaboration                             *
#
# Author: The Belle II Collaboration                                     *
# Contributors: Benjamin Oberhof
# This steering file generates an ntuple for validation
# purposes
#######################################################

"""
<header>
<output>ECLBkgOutput.root</output>
<contact>Benjamin Oberhof, ecl2ml@bpost.kek.jp</contact>
</header>
"""

import os
import glob
from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction

set_log_level(LogLevel.ERROR)

# Register necessary modules
eventinfosetter = register_module('EventInfoSetter')
# Set the number of events to be processed (1000 events)
eventinfosetter.param({'evtNumList': [100], 'runList': [1]})

# eventinfoprinter = register_module('EventInfoPrinter')

# Fixed random seed
set_random_seed(123456)

eclanalysis = register_module('ECLDataAnalysis')
eclanalysis.param('rootFileName', '../ECLBkgOutput.root')
eclanalysis.param('doTracking', 1)

# bg = None
if 'BELLE2_BACKGROUND_DIR' in os.environ:
    bg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
else:
    print('Warning: variable BELLE2_BACKGROUND_DIR is not set')

# Create paths
main = create_path()
main.add_module(eventinfosetter)
# main.add_module(eventinfoprinter)
add_simulation(main, bkgfiles=bg)
add_reconstruction(main)
main.add_module(eclanalysis)

process(main)
# print(statistics)
