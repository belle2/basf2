#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to reconstruct the
# following  decay chain:
#
# B0 -> rho gamma
#       |
#       +-> pi+ pi-
#
# and construct pi0/eta veto for the signal photon. In this
# example the pi0 veto is constructed in the following
# way:
# - for each B0 -> rho gamma candidate
#     i) combine photon (gamma) used in the reconstruction
#        of the B0 candidate with all other soft photons defined
#        in analysis/scripts/modularAnalysis.py
#    ii) find pi0/eta candidate with highest pi0/eta probability
#   iii) write value of pi0/eta probability
#
# Note: This example uses the signal MC sample created in
# MC campaign 9, therefore it can be ran only on KEKCC computers.
#
# Contributors: A. Zupanc (June 2014)
#               K. Ota (Oct 2017)
#               I. Komarov (December 2017)
#               I. Komarov (September 2018)
#
################################################################################

import basf2 as b2
import modularAnalysis as ma
import variables.collections as vc
import variables.utils as vu
import stdCharged as stdc

# create path
my_path = b2.create_path()

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

# perform MC matching (MC truth asociation)
ma.matchMCTruth(list_name='B0',
                path=my_path)

# build RestOfEvent (ROE) object for each B0 candidate
# ROE is required by the veto
ma.buildRestOfEvent(target_list_name='B0',
                    path=my_path)

# perform pi0/eta veto
ma.writePi0EtaVeto(particleList='B0',
                   decayString='B0 -> rho0 ^gamma',
                   workingDirectory='./pi0etaveto',
                   path=my_path)

# at this stage the B0 candidates should have
# extraInfo(Pi0_Prob) and extraInfo(Eta_Prob) value attached.
# extraInfo(Pi0_Prob) means pi0 probability for the B0 candidates whose gamma daughter.
# extraInfo(Eta_Prob) means eta probability for the B0 candidates whose gamma daughter.
# For the B0 candidates whose gamma daughter could not be combined with
# any of the remaining photons to form pi0/eta because of soft photon selection
# the extraInfo(Pi0_Prob) and extraInfo(Eta_Prob) does not exist. In these cases
# -999 will be written to the extraInfo(Pi0_Prob) branch and extraInfo(Eta_Prob) branch.
# You can change extraInfo names of pi0/eta probability by setting pi0vetoname and etavetoname parameters. For example,
# writePi0EtaVeto('B0', 'B0 -> rho0 ^gamma', workingDirectory='./pi0etaveto', pi0vetoname='Pi0_Prob2', etavetoname='Eta_Prob2')

# You need at least the default weight files: pi0veto.root and etaveto.root for writePi0EtaVeto.
# The default files are optimised by MC campaign 9.
# If you don't have weight files in your workingDirectory,
# these files are downloaded from database to your workingDirectory automatically.
# The default workingDirectory is '.'
# You can also download them from following directory in KEKCC:
# /gpfs/group/belle2/users/akimasa/pi0etaveto
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
# particle lists therfore we need to check if the current ROE object
# is related to the Particle from our signal decay. If it is not
# the execution of roe_path will be finished (by starting empty,
# dead end path). Note that in this example this x-check is not
# neccessary, but is anyway added for sake of completness
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
# we need a ParticleList containg the photon candidate used to reconstruct the
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
# Therfore, we just simply rank pi0 candidates according to their distance
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
# -999 will be written to the extraInfo(pi0veto) branch
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
    vu.create_aliases_for_selected(list_of_variables=rho_vars,
                                   decay_string='B0 -> [rho0 -> ^pi+ ^pi-] gamma') + \
    vu.create_aliases(list_of_variables=['Pi0_Prob', 'Eta_Prob', 'pi0veto'],
                      wrapper='extraInfo(variable)',
                      prefix="B")

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
