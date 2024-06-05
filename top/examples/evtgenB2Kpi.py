#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ----------------------------------------------------------------------------------
# Example of generating signal MC (B0 -> K- pi+, one of the benchmarks for TOP)
# Beam BG is added if variable BELLE2_BACKGROUND_DIR is set with the path to BG files
# At KEKCC the path to BG files is /sw/belle2/bkg
# Two outputs are provided:
#  - mdst format, suitable for physics studies (analysis package)
#  - flat ntuple, suitable for TOP efficiency studies
# ----------------------------------------------------------------------------------

import basf2 as b2
import os
from simulation import add_simulation
from reconstruction import add_reconstruction
from mdst import add_mdst_output
import glob

# Suppress messages and warnings during processing:
b2.set_log_level(b2.LogLevel.ERROR)

# Create path
main = b2.create_path()

# Number of events to be generated
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [100])
main.add_module(eventinfosetter)

# Event generator (B0 -> K+pi- + cc, other B0 generic)
evtgeninput = b2.register_module('EvtGenInput')
evtgeninput.param('userDECFile',
                  b2.find_file('top/examples/B2Kpi.dec'))
main.add_module(evtgeninput)

# Detector simulation
bg = None
if 'BELLE2_BACKGROUND_DIR' in os.environ:
    bg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
add_simulation(main, bkgfiles=bg)

# Reconstruction
add_reconstruction(main)

# Output to mdst
add_mdst_output(main, filename='evtgenB2Kpi.mdst.root')

# Output to a flat ntuple with TOP likelihoods, track info and MC truth
ntuple = b2.register_module('TOPNtuple')
ntuple.param('outputFileName', 'ntupleB2Kpi.root')
main.add_module(ntuple)

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
