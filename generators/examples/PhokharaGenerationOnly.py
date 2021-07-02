#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2

# Set the global log level
basf2.set_log_level(basf2.LogLevel.INFO)

main = basf2.create_path()

main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=100)

# Register the PHOKHARA module
phokhara = basf2.register_module('PhokharaInput')

# Set the logging level for the PHOKHARA module to INFO in order to see the cross sections etc.
phokhara.set_log_level(basf2.LogLevel.INFO)

# Set the parameter file (EXPERTS ONLY!)
# phokhara.param('ParameterFile', 'test.dat')

# Set the final state particles
# mu+mu-(0), pi+pi-(1), 2pi0pi+pi-(2), 2pi+2pi-(3), ppbar(4), nnbar(5), K+K-(6),
# K0K0bar(7), pi+pi-pi0(8), lamb(->pi-p)lambbar(->pi+pbar)(9), eta pi+ pi- (10)
phokhara.param('FinalState', 0)

# soft photon cutoff, final result is indepedent of the cut off as long as its small (<1e-3)
# photon multiplicity (and exclusive cross sections depent on that parameter)
# EXPERTS ONLY
phokhara.param('Epsilon', 0.0001)

# Events (weighted) to be used for maximum weight search before generation
phokhara.param('SearchMax', 5000)

# Events (unweighted) before event loop is aborted
phokhara.param('nMaxTrials', 25000)

# LO switch --> Born corresponds to 1 photon (0), Born corresponds to 0 photons (1), only Born: 0 photons (-1)
# original comment: ph0  Born: 1ph(0), Born: 0ph(1), only Born: 0ph(-1)
phokhara.param('LO', 0)

# NLO switch --> only for LO=0: off(0), on(1)
# original comment: 1 photon : Born(0), NLO(1)
phokhara.param('NLO', 1)

# QED corrections: ISR only(0), ISR+FSR(1), ISR+INT+FSR(2), if NLO=1 only 0 and 2 are possible
# original comment: ISR only(0), ISR+FSR(1), ISR+INT+FSR(2)
phokhara.param('QED', 0)

# NLO options (only if NLO=1 and QED=2) - CODE RUNS VERY (!) SLOW
# original comment: IFSNLO: no(0), yes(1)
phokhara.param('NLOIFI', 0)

# Vacuum polarization switch: off (0), on (1, [by Fred Jegerlehner, alphaQED/hadr5]), on (2,[by Thomas Teubner])
phokhara.param('Alpha', 0)

# Pion FormFactor switch
# original comment: FF_pion: KS PionFormFactor(0),GS old (1),GS new (2)
phokhara.param('PionFF', 0)

# Kaon FormFactor switch
# original comment: FF_kaon: KaonFormFactor constrained (0),KaonFormFactor unconstrained (1),KaonFormFactor old (2)
phokhara.param('KaonFF', 0)

# Pion Structure
# original comment: for pi+pi- only: f0+f0(600): K+K- model(0), "no structure" model(1), no f0+f0(600)(2), f0 KLOE(3)
phokhara.param('PionStructure', 0)

# Include narrow resonances (no Upsilon included yet!!!): no narrow resonances (0), j/psi (1) OR psi2s (2)
# original comment: narr_res: no narrow resonances (0), J/psi (1) and psi(2S) (2) only for pion = 0,1,6,7
phokhara.param('NarrowRes', 0)

# Proton FormFactor switch
# original comment: FF_pp: ProtonFormFactor old(0), ProtonFormFactor new(1)
phokhara.param('ProtonFF', 1)

# min/max angle of the photon
# original comment: minimal photon angle/missing momentum angle, maximal photon angle/missing momentum angle
phokhara.param('ScatteringAngleRangePhoton', [0., 180.])

# min/max angle of the other final state particles
# original comment: minimal pions(muons,nucleons,kaons) angle, maximal pions(muons,nucleons,kaons) angle
phokhara.param('ScatteringAngleRangeFinalStates', [0., 180.])

# Minimal hadrons/muons-gamma invariant mass squared [GeV^2]
# original comment: minimal hadrons(muons)-gamma-inv. mass squared
phokhara.param('MinInvMassHadronsGamma', 0.)

# Minimal hadrons/muons invariant mass squared [GeV^2]
# original comment: minimal inv. mass squared of the hadrons(muons)
phokhara.param('MinInvMassHadrons', 0.5)

# Maximal hadrons/muons invariant mass squared [GeV^2]
# original comment: maximal inv. mass squared of the hadrons(muons)
phokhara.param('MaxInvMassHadrons', 105.0)

# Minimal photon energy/missing energy, must be larger than 0.01*(CMS energy) [GeV]
# original comment: minimal photon energy/missing energy
phokhara.param('MinEnergyGamma', 0.25)

# output
output = basf2.register_module('RootOutput')
output.param('outputFileName', './phokhara_out.root')

# Create the main path and add the modules
main.add_module("Progress")
main.add_module(phokhara)
main.add_module(output)
main.add_module("PrintMCParticles", logLevel=basf2.LogLevel.DEBUG, onlyPrimaries=False)

# generate events
basf2.process(main)

# show call statistics
print(basf2.statistics)
