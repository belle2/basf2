#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
#                                                                        #
# Stuck? Ask for help at questions.belle2.org                            #
#                                                                        #
# This tutorial demonstrates how to reconstruct the                      #
# following  decay chain:                                                #
#                                                                        #
# B0 -> rho gamma                                                        #
#       |                                                                #
#       +-> pi+ pi-                                                      #
#                                                                        #
# and construct pi0/eta veto for the signal photon. In this              #
# example the pi0 veto is constructed in the following                   #
# way:                                                                   #
# - for each B0 -> rho gamma candidate                                   #
#     i) combine photon (gamma) used in the reconstruction               #
#        of the B0 candidate with all other soft photons defined         #
#        in analysis/scripts/modularAnalysis.py                          #
#    ii) find pi0/eta candidate with highest pi0/eta probability         #
#   iii) write value of pi0/eta probability                              #
#                                                                        #
##########################################################################

import basf2 as b2
import modularAnalysis as ma
import variables.collections as vc
import variables.utils as vu
from stdCharged import stdK, stdPi

# create path
my_path = b2.create_path()

# writePi0EtaVeto uses a payload in analysis global tag.
b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('B2rhogamma_rho2pipi.root', 'examples', False),
             path=my_path)

ma.fillParticleList(decayString='gamma:highE',
                    cut='E > 1.5',
                    path=my_path)
ma.fillParticleList(decayString='pi+:loose',
                    cut='abs(d0) < 0.5 and abs(z0) < 0.5 and pionID > 0.002',
                    path=my_path)

# reconstruct rho -> pi+ pi- decay
# keep only candidates with 0.6 < M(pi+pi-) < 1.0 GeV
ma.reconstructDecay(decayString='rho0 -> pi+:loose pi-:loose',
                    cut='0.6 < M < 1.0',
                    path=my_path)

# reconstruct B0 -> rho0 gamma decay
# keep only candidates with Mbc > 5.2 GeV
# and -2 < Delta E < 2 GeV
ma.reconstructDecay(decayString='B0 -> rho0 gamma:highE',
                    cut='5.2 < Mbc and abs(deltaE) < 2.0',
                    path=my_path)

# perform MC matching (MC truth association)
ma.matchMCTruth(list_name='B0',
                path=my_path)

# build RestOfEvent (ROE) object for each B0 candidate
# ROE is required by the veto
ma.buildRestOfEvent(target_list_name='B0',
                    path=my_path)

# perform pi0/eta veto
# particleList : Signal side particle's particleList
# decayString : DecayString specifying a particle which is used to calculate the pi0/eta probability
# mode : One can select the payload from 'standard'(default), 'tight', 'cluster', and 'both'.
#        Each payload is optimized for different soft-photon selection criteria.
#        If one wants to use one's own payload and soft-photon criteria, please use arguments,
#        pi0PayloadNameOverride, pi0SoftPhotonCutOverride, etaPayloadNameOverride, etaSoftPhotonCutOverride,
ma.writePi0EtaVeto(particleList='B0',
                   decayString='B0 -> rho0 ^gamma',
                   mode='standard',
                   path=my_path)

# Then one can obtain the pi0/eta probability by the variables, `pi0Prob(arg)` and `etaProb`.
# The argument corresponds to the mode which you set in writPieEtaVeto function.
# In above case, one can call `pi0Probe(standard)` and `etaProb(standard)`.
# For the B0 candidates whose gamma daughter could not be combined with
# any of the remaining photons to form pi0/eta because of soft photon selection.
# In these cases NaN will be written to the `pi0Probe(standard)` branch and `etaProb(standard)` branch.

# For the validation purpose, one may want to calculate the pi0/eta probability using a particle other than a photon.
# Example : B+ -> anti-D0 pi+. This is one of the mode to validate the pi0/eta veto tool.
stdK('loose', path=my_path)
stdPi('loose', path=my_path)
ma.reconstructDecay("D0:Kpi -> K-:loose pi+:loose", "", path=my_path)
ma.reconstructDecay("B+:Dpi -> anti-D0:Kpi pi+:loose", "useCMSFrame(daughter(1,E))>1.4", path=my_path)
ma.matchMCTruth("B+:Dpi", path=my_path)
ma.buildRestOfEvent("B+:Dpi", path=my_path)

# hardParticle : If one wants to use non-gamma particle to calculate the pi0/eta probability,
#                you have to tell the particle name with an argument hardParticle. (default: gamma)
ma.writePi0EtaVeto(particleList='B+:Dpi',
                   decayString='B+ -> [anti-D0 -> K+ pi-] ^pi+',
                   mode='standard',
                   hardParticle='pi+',
                   path=my_path)


# The weight files are optimised by MC campaign 12.
# If you train by yourself, you should refer to
# B2A701-ContinuumSuppression_Input.py
# B2A702-ContinuumSuppression_MVATrain.py


# You can also do a simple veto using delta mass ranking as below.

# VETO starts here
# ----------------

# Create a new path (called ROE path) which will be executed for
# each ROE in an event.
# Note that ROE exists for each B0 candidate, so when we loop
# over each ROE, we effectively loop over signal B0 candidates

roe_path = b2.create_path()

# The ROE objects might in general be related to Particle from multiple
# particle lists therefore we need to check if the current ROE object
# is related to the Particle from our signal decay. If it is not
# the execution of roe_path will be finished (by starting empty,
# dead end path). Note that in this example this x-check is not
# necessary, but is anyway added for sake of completeness
deadEndPath = b2.create_path()

# Note again: all actions (modules) included in roe_path will be
# executed for each ROE in the event
# First we check that the current ROE is related to B0 candidate
ma.signalSideParticleFilter(particleList='B0',
                            selection='',
                            roe_path=roe_path,
                            deadEndPath=deadEndPath)

# create and fill gamma ParticleList that will contain
# all photons found in ROE (not used to reconstruct current B0 candidate)
# The photons need to have energy above 50 MeV to be considered
# (one can add any cut)
ma.fillParticleList(decayString='gamma:roe',
                    cut='isInRestOfEvent == 1 and E > 0.050',
                    path=roe_path)

# in order to be able to use modularAnalysis functions (reconstructDecay in particular)
# we need a ParticleList containing the photon candidate used to reconstruct the
# current B meson as well
# The DecayString is used to specify the selected particle (^)
ma.fillSignalSideParticleList(outputListName='gamma:sig',
                              decayString='B0 -> rho0 ^gamma',
                              path=roe_path)

# make combinations of signal photon candidates with all photons from ROE
# keep only combinations in given invariant mass range
ma.reconstructDecay(decayString='pi0:veto -> gamma:sig gamma:roe',
                    cut='0.080 < M < 0.200',
                    path=roe_path)

# at this point one could use all features provided by the analysis software
# to make the veto as effective as possible. For example, one can perform truth
# matching, training/applying TMVA classifier, save pi0 candidates with ntuple
# maker for offline analysis/study.

# in this example the variable, which is used to veto pi0 is very simple:
# invariant mass of pi0 that is closest to the pi0's nominal mass
# Therefore, we just simply rank pi0 candidates according to their distance
# from nominal mass (dM variable) and keep only the best candidate
ma.rankByLowest(particleList='pi0:veto',
                variable='abs(dM)',
                numBest=1,
                path=roe_path)

# write the invariant mass of the best pi0 candidate to the current B0
# candidate as the 'pi0veto' extraInfo
ma.variableToSignalSideExtraInfo(particleList='pi0:veto', varToExtraInfo={'M': 'pi0veto'}, path=roe_path)

# execute roe_path for each RestOfEvent in the event
my_path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

# VETO ends here
# ----------------

# we're now out of the ROE path
# at this stage the B0 candidates should have
# extraInfo(pi0veto) value attached. For the B0
# candidates whose gamma daughter could not be combined with
# any of the remaining photons to form pi0 within given mass
# range the extraInfo(pi0veto) does not exist. In these cases
# NaN will be written to the extraInfo(pi0veto) branch
# Select variables that we want to store to ntuple

gamma_vars = vc.cluster + \
    vc.mc_truth + \
    vc.kinematics

rho_vars = vc.cluster + \
    vc.mc_truth + \
    vc.kinematics + \
    vc.inv_mass

pi_vars = vc.track

b_vars = vc.kinematics + \
    vc.deltae_mbc + \
    vc.mc_truth + \
    vu.create_aliases_for_selected(list_of_variables=gamma_vars,
                                   decay_string='B0 -> rho0 ^gamma') + \
    vu.create_aliases_for_selected(list_of_variables=rho_vars,
                                   decay_string='B0 -> ^rho0 gamma') + \
    vu.create_aliases_for_selected(list_of_variables=pi_vars,
                                   decay_string='B0 -> [rho0 -> ^pi+ ^pi-] gamma') + \
    ['pi0Prob(standard)', 'etaProb(standard)', 'extraInfo(pi0veto)']


# Saving variables to ntuple
rootOutputFile = "B2A306-B02RhoGamma-withPi0EtaVeto.root"
ma.variablesToNtuple(decayString='B0',
                     variables=b_vars,
                     filename=rootOutputFile,
                     treename='b0',
                     path=my_path)

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
