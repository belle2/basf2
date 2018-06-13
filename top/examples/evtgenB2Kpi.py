#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from reconstruction import add_mdst_output
import glob
from ROOT import Belle2

# ----------------------------------------------------------------------------------
# Example of generating signal MC (B0 -> K- pi+, one of the benchmarks for TOP)
# Beam BG is added if variable BELLE2_BACKGROUND_DIR is set with the path to BG files
# At KEKCC the path to BG files is /sw/belle2/bkg
# Two outputs are provided:
#  - mdst format, suitable for physics studies (analysis package)
#  - flat ntuple, suitable for TOP efficiency studies
# ----------------------------------------------------------------------------------

# Suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# Create path
main = create_path()

# Number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [100])
eventinfosetter.param('runList', [0])
eventinfosetter.param('expList', [0])
main.add_module(eventinfosetter)

# Event generator (B0 -> K+pi- + cc, other B0 generic)
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('userDECFile',
                  Belle2.FileSystem.findFile('top/examples/B2Kpi.dec'))
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
ntuple = register_module('TOPNtuple')
ntuple.param('outputFileName', 'ntupleB2Kpi.root')
main.add_module(ntuple)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
