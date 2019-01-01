#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
#
#  This steering file shows how to correctly "load"
#  the Muid variables and use them in a steering file
#  for calibration studies.
#
#  In the first part of the script we generate a sample
#  of muons and we simulate and reconstuct them.
#  In the second part we run a (very) simple analysis
#  and save some variables to ntuple.
#
#  Note that the Muid variables are intended to be used
#  by experts for calibration studies!
#
#  Contact: Giacomo De Pietro (2018)
#           giacomo.depietro@roma3.infn.it
#
########################################################

import basf2 as b2
import generators as gen
import simulation as sim
import reconstruction as rec
import modularAnalysis as ma

# Load the tracking libraries
# Fundamental for using the Muid variables!
from ROOT import gSystem
gSystem.Load('libtracking.so')

# Create the main path
main = b2.create_path()

#############################################
# GENERATION, SIMULATION AND RECONSTRUCTION #
#############################################

# Set EventInfoSetter and add a progress bar
main.add_module('EventInfoSetter',
                expList=0,
                runList=1,
                evtNumList=100)
main.add_module('Progress')
main.add_module('ProgressBar')

use_KKMC = True
if use_KKMC:  # Use KKMC to generate generic mu+mu- events
    gen.add_kkmc_generator(finalstate='mu+mu-',
                           path=main)
else:  # Use ParticleGun to generate 4GeV mu+ and mu-
    main.add_module('ParticleGun',
                    nTracks=1,
                    pdgCodes=[13, -13],
                    momentumGeneration='fixed',
                    momentumParams=[4],
                    thetaGeneration='uniformCos',
                    thetaParams=[20, 47])

# Add simulation and reconstruction
# Note that we skip the trigger simulation in this example
sim.add_simulation(path=main)
rec.add_reconstruction(path=main)

############
# ANALYSIS #
############

# Reconstruct muon-candidates in the event with ECL-based cuts
ma.fillParticleList(
    'mu+:basic',
    'nCDCHits > 20 and abs(dz) < 2.0 and abs(dr) < 0.5 and 0.15 < clusterE < 0.4 and formula(clusterE/p) < 0.4',
    path=main)

# Select better muon-candidates requiring at least one hit after layer 3 of KLM
# Note that we are using a Muid variable
ma.cutAndCopyList('mu+:muid',
                  'mu+:basic',
                  'muidHitLayer > 3',
                  path=main)

# Saving variables to a flat ntuple
listOfVariables = ['p',
                   'charge',
                   'theta',
                   'phi',
                   'muonID',
                   'pionID',
                   # Here we select some Muid variables
                   'muidMuonProbability',
                   'muidPionProbability',
                   'muidMuonLogLikelihood',
                   'muidPionLogLikelihood',
                   'muidOutcomeExtTrack',
                   'muidHitLayer',
                   'muidExtLayer',
                   'muidHitLayerPattern',
                   'muidExtLayerPattern',
                   # Here we select some KLMCluster variables
                   'klmClusterIsBKLM',
                   'klmClusterInnermostLayer',
                   'klmClusterLayers']
ma.variablesToNtuple('mu+:muid',
                     listOfVariables,
                     filename='MuidVariables.root',
                     treename='muons',
                     path=main)

# Process the path
b2.process(main)
print(b2.statistics)
