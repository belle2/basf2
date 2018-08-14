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
#
# Contributors: A. Zupanc (June 2014)
#               I. Komarov (Demeber 2017)
#
######################################################

from basf2 import *
from modularAnalysis import inputMdstList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from stdPhotons import stdPhotons
from stdCharged import stdLoosePi

# Run this tutorial either over signal MC or background MC (K*gamma)
# Add 10 signal MC files (each containing 1000 generated events)
filelistBKG = ['/group/belle2/tutorial/release_01-00-00/Bd_Kstgamma_GENSIMRECtoDST.dst.root']
filelistSIG = ['/group/belle2/tutorial/release_01-00-00/mdst_000001_prod00002326_task00000001.root']

# Run B0 -> rho gamma reconstruction over B0 -> rho gamma MC
rootOutputFile = 'B2A304-B02RhoGamma-Reconstruction-SIGMC.root'
inputMdstList('default', filelistSIG)

# Run B0 -> rho gamma reconstruction over B0 -> K* gamma MC
# (uncomment next two lines and comment above two to run over BKG MC)
# rootOutputFile = 'B2A304-B02RhoGamma-Reconstruction-BKGMC.root'
# inputMdstList('default',filelistBKG)

# use standard final state particle lists
#
# creates "gamma:tight" ParticleList
stdPhotons('tight')

# creates "pi+:loose" ParticleList (and c.c.)
stdLoosePi()

# reconstruct rho -> pi+ pi- decay
# keep only candidates with 0.6 < M(pi+pi-) < 1.0 GeV
reconstructDecay('rho0 -> pi+:loose pi-:loose', '0.6 < M < 1.0')

# reconstruct B0 -> rho0 gamma decay
# keep only candidates with Mbc > 5.2 GeV
# and -2 < Delta E < 2 GeV
reconstructDecay('B0 -> rho0 gamma:tight', '5.2 < Mbc < 5.29 and abs(deltaE) < 2.0')

# perform MC matching (MC truth asociation)
matchMCTruth('B0')

# Select variables that we want to store to ntuple
import variableCollections as vc

gammatools = vc.cluster + vc.mc_truth + vc.kinematics
rhotools = vc.cluster + vc.mc_truth + vc.kinematics + vc.inv_mass
pitools = vc.pid + vc.track
btools = vc.event_meta_data + vc.kinematics + vc.deltae_mbc + vc.mc_truth + \
    vc.convert_to_all_selected_vars(gammatools,
                                    'B0 -> rho0 ^gamma') + \
    vc.convert_to_all_selected_vars(rhotools,
                                    'B0 -> ^rho0 gamma') + \
    vc.convert_to_all_selected_vars(rhotools,
                                    'B0 -> [rho0 -> ^pi+ ^pi-] gamma')

# Saving variables to ntuple
from modularAnalysis import variablesToNtuple
variablesToNtuple('B0', btools,
                  filename=rootOutputFile, treename='b0')

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
