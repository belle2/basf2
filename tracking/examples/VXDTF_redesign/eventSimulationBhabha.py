#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#####################################################################
# VXDTF2 Example Scripts - Step 0 - Simulation
#
# This script can be used to produce MC data from particle guns or
# Y(4S) events for the training and validation of the VXDTF 1 and 2.
#
# The number of events which will be simulated can be set via the
# basf2 commandline option -n.
# The name of the root output file can be defined with the option -o.
#
# E.g.: 'basf2 eventSimulation.py -n 1000 -o trainingSample.root'
#
# The settings for the particle gun(s) and EvtGen simulation can be
# adapted in this script. Some convenience functions are outsourced
# to setup_modules.py.
#
# The script takes two optional command line arguments: the first will
# be interpreted as random seed, the second as directory for the output.
# e.g: basf2 'eventSimulation.py 12354 ./datadir/'
# will result in setting the random seed to 12354 and the output will
# be written to './datadir/'
#
# Contributors: Jonas Wagner, Felix Metzner, Thomas Lueck
#####################################################################


import basf2 as b2
from beamparameters import add_beamparameters
from simulation import add_simulation

import os
import sys
import random

from pxd import add_pxd_reconstruction
from svd import add_svd_reconstruction
from generators import add_babayaganlo_generator

# ---------------------------------------------------------------------------------------

# Set Random Seed for reproducable simulation. 0 means really random.
rndseed = 12345
# assume the first argument is the random seed
if(len(sys.argv) > 1):
    rndseed = sys.argv[1]

outputDir = './'
# assume second argument is the output directory
if(len(sys.argv) > 2):
    outputDir = sys.argv[2]

b2.set_random_seed(rndseed)


# Set log level. Can be overridden with the "-l LEVEL" flag for basf2.
b2.set_log_level(b2.LogLevel.ERROR)

# ---------------------------------------------------------------------------------------
main = b2.create_path()

eventinfosetter = b2.register_module('EventInfoSetter')
# default phase3 geometry:
exp_number = 0
# if environment variable is set then phase2 (aka Beast2) geometry will be taken
if os.environ.get('USE_BEAST2_GEOMETRY'):
    exp_number = 1002
eventinfosetter.param("expList", [exp_number])
main.add_module(eventinfosetter)

eventinfoprinter = b2.register_module('EventInfoPrinter')
main.add_module(eventinfoprinter)

progress = b2.register_module('Progress')
main.add_module(progress)

# ---------------------------------------------------------------------------------------
# Simulation Settings:


# randomize the vertex position (flatly distributed) to make the sectormap more robust wrt. changing beam position
# minima and maxima of the beam position given in cm
random.seed(rndseed)
vertex_x_min = -0.1
vertex_x_max = 0.1
vertex_y_min = -0.1
vertex_y_max = 0.1
vertex_z_min = -0.5
vertex_z_max = 0.5

vertex_x = random.uniform(vertex_x_min, vertex_x_max)
vertex_y = random.uniform(vertex_y_min, vertex_y_max)
vertex_z = random.uniform(vertex_z_min, vertex_z_max)

print("WARNING: setting non-default beam vertex at x= " + str(vertex_x) + " y= " + str(vertex_y) + " z= " + str(vertex_z))

# Particle Gun:
# One can add more particle gun modules if wanted.

# additional flatly smear the muon vertex between +/- this value
vertex_delta = 0.005  # in cm

particlegun = b2.register_module('ParticleGun')
particlegun.logging.log_level = b2.LogLevel.WARNING
param_pGun = {
    'pdgCodes': [13, -13],   # 13 = muon --> negatively charged!
    'nTracks': 8,
    'momentumGeneration': 'uniform',
    'momentumParams': [0.1, 7],
    'vertexGeneration': 'uniform',
    'xVertexParams': [vertex_x - vertex_delta, vertex_x + vertex_delta],            # in cm...
    'yVertexParams': [vertex_y - vertex_delta, vertex_y + vertex_delta],
    'zVertexParams': [vertex_z - vertex_delta, vertex_z + vertex_delta]
}

particlegun.param(param_pGun)
main.add_module(particlegun)

# EvtGen Simulation:
# TODO: There are newer convenience functions for this task -> Include them!
# Beam parameters
beamparameters = add_beamparameters(main, "Y4S")
beamparameters.param("vertex", [vertex_x, vertex_y, vertex_z])
# print_params(beamparameters)

# evtgenInput = register_module('EvtGenInput')
# evtgenInput.logging.log_level = LogLevel.WARNING
# main.add_module(evtgenInput)

# generate some Bhabha
add_babayaganlo_generator(path=main, finalstate='ee')


# ---------------------------------------------------------------------------------------


# Detector Simulation:
add_simulation(path=main,
               usePXDDataReduction=False,  # for training one does not want the data reduction
               components=None)  # dont specify components because else not the whole geometry will be loaded!

# needed for fitting
main.add_module('SetupGenfitExtrapolation')

# this adds the clusters for PXD and SVD (was done by the usePXDDataReduction previously) which are needed in the next steps
add_pxd_reconstruction(path=main)
add_svd_reconstruction(path=main)

# ---------------------------------------------------------------------------------------
# Setting up the MC based track finder.
mctrackfinder = b2.register_module('TrackFinderMCTruthRecoTracks')
mctrackfinder.param('UseCDCHits', False)
mctrackfinder.param('UseSVDHits', True)
# Always use PXD hits! For SVD only, these will be filtered later when converting to SPTrackCand
# 15.02.2018: deactivated PXD again as we dont do pxd track finding anymore and to not bias the fit
mctrackfinder.param('UsePXDHits', False)
mctrackfinder.param('Smearing', False)
mctrackfinder.param('MinimalNDF', 6)
mctrackfinder.param('WhichParticles', ['primary'])
mctrackfinder.param('RecoTracksStoreArrayName', 'MCRecoTracks')
# set up the track finder to only use the first half loop of the track and discard all other hits
mctrackfinder.param('UseNLoops', 0.5)
mctrackfinder.param('discardAuxiliaryHits', True)
main.add_module(mctrackfinder)

# include a track fit into the chain (sequence adopted from the tracking scripts)
# Correct time seed: Do I need it for VXD only tracks ????
main.add_module("IPTrackTimeEstimator", recoTracksStoreArrayName="MCRecoTracks", useFittedInformation=False)
# track fitting
daffitter = b2.register_module("DAFRecoFitter")
daffitter.param('recoTracksStoreArrayName', "MCRecoTracks")
# daffitter.logging.log_level = LogLevel.DEBUG
main.add_module(daffitter)
# also used in the tracking sequence (multi hypothesis)
# may be overkill
main.add_module('TrackCreator', recoTrackColName="MCRecoTracks", pdgCodes=[211, 321, 2212])


# build the name of the output file
outputFileName = outputDir + './'
if os.environ.get('USE_BEAST2_GEOMETRY'):
    outputFileName += "SimEvts_Beast2"
else:
    outputFileName += "SimEvts_Belle2"
outputFileName += '_' + str(rndseed) + '.root'

# Root output. Default filename can be overriden with '-o' basf2 option.
rootOutput = b2.register_module('RootOutput')
rootOutput.param('outputFileName', outputFileName)
# to save some space exclude everything except stuff needed for tracking
rootOutput.param('excludeBranchNames', ["ARICHAeroHits",
                                        "ARICHDigits",
                                        "ARICHSimHits",
                                        "BKLMDigits",
                                        "BKLMSimHitPositions",
                                        "BKLMSimHits",
                                        "CDCHits",
                                        "CDCHits4Trg",
                                        "CDCSimHits",
                                        "CDCSimHitsToCDCHits4Trg",
                                        "ECLDigits",
                                        "ECLDsps",
                                        "ECLHits",
                                        "ECLSimHits",
                                        "ECLTrigs",
                                        "ECLDiodeHits",
                                        "EKLMDigits",
                                        "EKLMSimHits",
                                        "TOPBarHits",
                                        "TOPDigits",
                                        "TOPRawDigits",
                                        "TOPSimHits"
                                        ])
main.add_module(rootOutput)

b2.log_to_file('createSim.log', append=False)

b2.print_path(main)

b2.process(main)
print(b2.statistics)
