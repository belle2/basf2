#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor: [e-e+ -> pi- pi+ gamma]

"""
<header>
  <output>../3110021000.dst.root</output>
  <contact>karim.trabelsi@kek.jp</contact>
</header>
"""

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output
from HLTTrigger import add_HLT_Y4S
from ROOT import Belle2
import glob

set_random_seed(12345)

# background (collision) files
bg = glob.glob('./BG/[A-Z]*.root')

# create the path
main = create_path()

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [100])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [1])
main.add_module(eventinfosetter)

# Register the PHOKHARA module
phokhara = register_module('PhokharaInput')

# Set the logging level for the PHOKHARA module to INFO in order to see the cross sections etc.
phokhara.set_log_level(LogLevel.INFO)

# Set the final state particles
# pi+pi-(1)
phokhara.param('FinalState', 1)

# soft photon cutoff, final result is indepedent of the cut off as long as its small (<1e-3)
# photon multiplicity (and exclusive cross sections depent on that parameter)
# EXPERTS ONLY
phokhara.param('Epsilon', 0.0001)

# Events (weighted) to be used for maximum weight search before generation
phokhara.param('SearchMax', 5000)

# Events (unweighted) before event loop is aborted
phokhara.param('nMaxTrials', 25000)

# LO switch --> Born corresponds to 1 photon (0), Born corresponds to 0 photons (1), only Born: 0 photons (-1)
phokhara.param('LO', 0)

# NLO switch --> only for LO=0: off(0), on(1)
phokhara.param('NLO', 1)

# QED corrections: ISR only(0), ISR+FSR(1), ISR+INT+FSR(2), if NLO=1 only 0 and 2 are possible
phokhara.param('QED', 0)

# NLO options (only if NLO=1 and QED=2) - CODE RUNS VERY (!) SLOW
phokhara.param('NLOIFI', 0)

# Vacuum polarization switch: off (0), on (1, [by Fred Jegerlehner, alphaQED/hadr5]), on (2,[by Thomas Teubner])
phokhara.param('Alpha', 0)

# Pion FormFactor: KS PionFormFactor(0),GS old (1),GS new (2)
phokhara.param('PionFF', 0)

# Kaon FormFactor: KaonFormFactor constrained (0),
# KaonFormFactor unconstrained (1),KaonFormFactor old (2)
phokhara.param('KaonFF', 0)

# Pion Structure: for pi+pi- only: f0+f0(600):
# K+K- model(0), "no structure" model(1), no f0+f0(600)(2), f0 KLOE(3)
phokhara.param('PionStructure', 0)

# Include narrow resonances (no Upsilon included yet!!!):
# no narrow resonances (0), j/psi (1) OR psi2s (2)
phokhara.param('NarrowRes', 0)

# Proton FormFactor: ProtonFormFactor old(0), ProtonFormFactor new(1)
phokhara.param('ProtonFF', 1)

# min/max angle of the photon
phokhara.param('ScatteringAngleRangePhoton', [0., 180.])

# min/max angle of the other final state particles
phokhara.param('ScatteringAngleRangeFinalStates', [0., 180.])

# Minimal hadrons/muons-gamma invariant mass squared [GeV^2]
phokhara.param('MinInvMassHadronsGamma', 0.)

# Minimal hadrons/muons invariant mass squared [GeV^2]
phokhara.param('MinInvMassHadrons', 0.01)

# Maximal hadrons/muons invariant mass squared [GeV^2]
phokhara.param('MaxInvMassHadrons', 112.0)

# Minimal photon energy/missing energy, must be larger than 0.01*(CMS energy) [GeV]
phokhara.param('MinEnergyGamma', 0.1058)

# Generate events with phokhara
main.add_module(phokhara)

# Simulation
# add_simulation(main, bkgfiles=bg)
add_simulation(main)

# HLT L3 simulation
main.add_module('Level3')

# Reconstruction
add_reconstruction(main)

# HLT physics trigger
add_HLT_Y4S(main)

# Add mdst output
output_filename = "../3110021000.dst.root"
add_mdst_output(main, filename=output_filename)

# generate events
process(main)

# show call statistics
print(statistics)
