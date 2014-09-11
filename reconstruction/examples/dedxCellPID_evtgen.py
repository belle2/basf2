#!/usr/bin/env python
# -*- coding: utf-8 -*-

######################################################
# This steering file generates several D* events
# and does a simple dE/dx measurement. The results
# are stored in a ROOT file.
#
# Example steering file - 2011 Belle II Collaboration
######################################################

import os
from basf2 import *

# register necessary modules
eventinfosetter = register_module('EventInfoSetter')

# generate events
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [10])

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
geometry.param('excludedComponents', ['EKLM'])

# generation of events according to the specified DECAY table
# e+e- -> ccbar -> D*+ anthing
# D*+ -> D0 pi+; D0 -> K- pi+
#
# generateContinuum function is defined in analysis/scripts/modularAnalysis.py
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('ParentParticle', 'vpho')
evtgeninput.param('userDECFile', 'DstarToDpi.dec')
evtgeninput.param('boost2LAB', True)

# simulation
g4sim = register_module('FullSim')

# make the simulation less noisy
g4sim.logging.log_level = LogLevel.ERROR

# create paths
main = create_path()

# add modules to paths
main.add_module(eventinfosetter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(evtgeninput)
main.add_module(g4sim)

main.add_module(register_module('CDCDigitizer'))

mctrackfinder = register_module('TrackFinderMCTruth')
mctrackfinder.param('UseCDCHits', True)
mctrackfinder.param('UseClusters', True)
main.add_module(mctrackfinder)

genfit = register_module('GenFitter')
genfit.param('UseClusters', True)
main.add_module(genfit)

# run the events through the dE/dx module
dedx = register_module('DedxCellPID')
dedx_params = {  # 'pdfFile': 'YourPDFFile.root',
    'useIndividualHits': False,
    'removeLowest': 0.0,
    'removeHighest': 0.2,
    'enableDebugOutput': True,
    'ignoreMissingParticles': False,
    }
dedx.param(dedx_params)
main.add_module(dedx)

output = register_module('RootOutput')
output.param('outputFileName', 'dedxCellPID_evtgen.root')
# output.param('branchNames','DedxCells')
main.add_module(output)
process(main)
print statistics
