#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <output>PhokharaEvtgenData.root</output>
    <contact>Kirill Chilikin (chilikin@lebedev.ru)</contact>
    <description>Generation of 25000 e+ e- -> J/psi eta_c events.</description>
</header>
"""

from basf2 import *

set_log_level(LogLevel.INFO)

set_random_seed(12345)

main = create_path()

# Event information
main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=25000)

# PHOKHARA
phokhara = register_module('PhokharaInput')
phokhara.param('FinalState', 0)
phokhara.param('ReplaceMuonsByVirtualPhoton', True)
phokhara.param('Epsilon', 0.0001)
phokhara.param('SearchMax', 5000)
phokhara.param('nMaxTrials', 25000)
phokhara.param('LO', 1)
phokhara.param('NLO', 1)
phokhara.param('QED', 0)
phokhara.param('NLOIFI', 0)
phokhara.param('Alpha', 1)
phokhara.param('PionFF', 0)
phokhara.param('KaonFF', 0)
phokhara.param('PionStructure', 0)
phokhara.param('NarrowRes', 0)
phokhara.param('ProtonFF', 1)
phokhara.param('ScatteringAngleRangePhoton', [0., 180.])
phokhara.param('ScatteringAngleRangeFinalStates', [0., 180.])
phokhara.param('MinInvMassHadronsGamma', 0.)
phokhara.param('MinInvMassHadrons', 36.932554310656)
phokhara.param('ForceMinInvMassHadronsCut', True)
phokhara.param('MaxInvMassHadrons', 200.0)
phokhara.param('MinEnergyGamma', 0.01)

# EvtGen
evtgendecay = register_module('EvtGenDecay')
evtgendecay.param('UserDecFile', find_file('generators/examples/PhokharaEvtgenDoubleCharmonium.dec'))

# Output
output = register_module('RootOutput')
output.param('outputFileName', 'PhokharaEvtgenData.root')

# Create the main path and add the modules
main.add_module(phokhara)
main.add_module(evtgendecay)
main.add_module(output)
main.add_module('Progress')

# generate events
process(main)

# show call statistics
print(statistics)
