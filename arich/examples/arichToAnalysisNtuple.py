#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

################################################################################
#
# This example demonstrates how to reconstruct the K_S0 decay and append detailed arich
# information to daughter pions in the analysis output ntuple
# As input CDST file is needed (no arich info except likelihoods is available in mdst!)
#
# See https://confluence.desy.de/display/BI/Usage+of+arichToNtuple+module
# for more details, including description of all arich variables that are added to ntuple
#
# Contributors: Luka Santelj (Feb 2020)
#               (luka.santelj@ijs.si)
################################################################################

import basf2 as b2
import modularAnalysis as ma
import variables.collections as vc
import variables.utils as vu
import arich as arich
from variables import variables
import vertex as vx

# create path
my_path = b2.create_path()

# load example input ROOT file (can override with -i option)
fname = "/group/belle2/dataprod/Data/release-03-02-02/DB00000654/proc9/e0008/4S/GoodRuns/r00827/"\
    "skim/hlt_hadron/cdst/sub00/cdst.physics.0008.00827.HLT1.hlt_hadron.f00000.root"
ma.inputMdst(environmentType='default',
             filename=fname,
             path=my_path)


# if you want re-run arich reconstruction uncomment lines below
# ===========================
# arichHits = b2.register_module('ARICHFillHits')
# arichHits.param('MagFieldCorrection',1)
# my_path.add_module(arichHits)
# arichRecon = b2.register_module('ARICHReconstructor')
# arichRecon.param('storePhotons', 1)
# my_path.add_module(arichRecon)
# ===========================

# creates "pi+:all" ParticleList (and c.c.)
# includes all tracks with thetaInCDCAcceptance and nCDCHits>20
ma.fillParticleList('pi+:all', 'thetaInCDCAcceptance and nCDCHits>20', True, path=my_path)


# define aliases for daughter particles
# cosTheta
variables.addAlias('pi0_cosTheta', 'daughter(0, cosTheta)')
variables.addAlias('pi1_cosTheta', 'daughter(1, cosTheta)')
# momentum
variables.addAlias('pi0_p', 'daughter(0, p)')
variables.addAlias('pi1_p', 'daughter(1, p)')

# reconstruct ks -> pi+ pi- decay
# keep only candidates with 0.45 < M(pi+pi-) < 0.55 GeV and with
# at least one "forward" track with cosTheta>0.82 and p>0.5 GeV
ma.reconstructDecay(decayString='K_S0 -> pi+:all pi-:all',
                    cut='0.45 < M < 0.55 and [[pi0_cosTheta > 0.82 and pi0_p > 0.5] or [pi1_cosTheta > 0.82 and pi1_p>0.5]]',
                    path=my_path)

# do vertex fit
# only keeps candidates with conf_level>0.001
vx.KFit(list_name='K_S0',
        conf_level=0.001,
        path=my_path)

# only keep K_S0 candidates with cosAngleBetweenMomentumAndVertexVector > 0.9
# (i.e. momentum in the same direction as decay vertex position)
ma.cutAndCopyLists("K_S0:good", "K_S0", "cosAngleBetweenMomentumAndVertexVector > 0.9", path=my_path)

# Select variables that we want to store to ntuple
# this follows exactly same scheme and syntax as used by variablesToNtuple!
# make aliases for long PID variables
variables.addAlias('dll_arich', 'pidDeltaLogLikelihoodValueExpert(211,321,ARICH)')
variables.addAlias('R_Kpi_arich', 'pidPairProbabilityExpert(321, 211, ARICH)')
variables.addAlias('kaonID_arich', 'pidProbabilityExpert(321, ARICH)')
variables.addAlias('pionID_arich', 'pidProbabilityExpert(211, ARICH)')
variables.addAlias('R_Kpi_all', 'pidPairProbabilityExpert(321, 211, ARICH,TOP,CDC)')  # kaon/pion all
variables.addAlias('R_Kpi_top', 'pidPairProbabilityExpert(321, 211, TOP)')  # kaon/pion tion
variables.addAlias('R_Kpi_cdc', 'pidPairProbabilityExpert(321,211,CDC)')  # kaon/pion cdc
variables.addAlias('R_ppi_woarich', 'pidPairProbabilityExpert(2212, 211,TOP,CDC,ECL)')  # proton/pion w/o arich

# variables to store for daughter pions
pi_vars = vc.pid + vc.track + ['nCDCHits',
                               'cosTheta',
                               'clusterE',
                               'nMatchedKLMClusters',
                               'pt',
                               'minC2TDist',
                               'trackNECLClusters',
                               'dll_arich',
                               'R_Kpi_arich',
                               'kaonID_arich',
                               'pionID_arich',
                               'R_Kpi_all',
                               'R_Kpi_top',
                               'R_Kpi_cdc',
                               'R_ppi_woarich']
# variables to store for K_S0
ks_vars = vc.mc_truth + vc.kinematics + vc.vertex + vc.inv_mass + ['cosAngleBetweenMomentumAndVertexVector'] + \
    vu.create_aliases_for_selected(list_of_variables=pi_vars,
                                   decay_string='K_S0 -> ^pi+ ^pi-')


# Save variables to ntuple
# ===================================================
# this is the main point of this example file
# instead of using "variablesToNtuple" we use "arichVariablesToNtuple" which takes the same parameters,
# but in addition one should specify string "arichSelector" which selects particles for which detailed arich information is stored
# (like number of detected photons, track position on aerogel plane, distribution of Cherenkov photons, etc)
# ===================================================
rootOutputFile = 'arich_ks_reco.root'

arich.arichVariablesToNtuple(decayString='K_S0:good',  # list of particles to fill
                             variables=ks_vars,  # list of all variables
                             arichSelector='K_S0 -> ^pi+ ^pi-',  # select particles for which arich detail info should be stored
                             filename=rootOutputFile,
                             treename='ks',
                             path=my_path)

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
