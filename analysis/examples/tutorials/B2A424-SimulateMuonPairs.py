#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# This steering file generates, simulates, and reconstructs
# a sample of 10 BBbar events.
#
# Usage: basf2 B2SimulateMuonPairs.py
#
# Input: None
# Output: output.root
#
# Example steering file - 2011 Belle II Collaboration
#############################################################

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output
from ROOT import Belle2
from beamparameters import add_beamparameters

# create path
main = create_path()

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10])
main.add_module(eventinfosetter)

beamparameters = add_beamparameters(main, "Y4S")
beamparameters.param("smearVertex", False)
beamparameters.param("smearEnergy", False)
print_params(beamparameters)

# muon pair generator (BABAYAGA.NLO, born only)
babayaganlo = register_module('BabayagaNLOInput')
babayaganlo.param('FinalState', 'mm')  # ee, gg, mm
babayaganlo.param('SearchMax', 10000)
babayaganlo.param('VacuumPolarization', 'hlmnt')
babayaganlo.param('Order', 'born')  # born, alpha, exp
babayaganlo.param('MaxAcollinearity', 180.)
babayaganlo.param('ScatteringAngleRange', [15.0, 165.0])
babayaganlo.param('DebugEnergySpread', 5.e-3)
babayaganlo.param('VPUncertainty', True)
main.add_module(babayaganlo)

# detector simulation
add_simulation(main)

# reconstruction
add_reconstruction(main)

# mdst output
add_mdst_output(
    main,
    mc=True,
    filename='B2A424-SimulateMuonPairs.root',
    additionalBranches=[
        'MCInitialParticles'])

# display a progress bar while running
main.add_module('ProgressBar')

# process events and print call statistics
process(main)
print(statistics)
