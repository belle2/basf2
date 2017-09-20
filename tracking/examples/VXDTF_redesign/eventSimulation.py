#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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


from basf2 import *
from beamparameters import add_beamparameters
from simulation import add_simulation

import os
import sys

from pxd import add_pxd_reconstruction
from svd import add_svd_reconstruction

# If later the use of bg is wanted, you can as well import setup_bg
from setup_modules import setup_sim
from setup_modules import setup_Geometry

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

set_random_seed(rndseed)

# Set log level. Can be overridden with the "-l LEVEL" flag for basf2.
set_log_level(LogLevel.ERROR)

# ---------------------------------------------------------------------------------------
main = create_path()

eventinfosetter = register_module('EventInfoSetter')
main.add_module(eventinfosetter)

eventinfoprinter = register_module('EventInfoPrinter')
main.add_module(eventinfoprinter)

progress = register_module('Progress')
main.add_module(progress)

# ---------------------------------------------------------------------------------------
# Simulation Settings:

# Particle Gun:
# One can add more particle gun modules if wanted.
particlegun = register_module('ParticleGun')
particlegun.logging.log_level = LogLevel.WARNING
param_pGun = {
    'pdgCodes': [13, -13],   # 13 = muon --> negatively charged!
    'nTracks': 1,
    'momentumGeneration': 'uniform',
    'momentumParams': [0.1, 4]
}
"""
              'momentumGeneration': 'fixed',
              'momentumParams': [2, 2],           # 2 values: [min, max] in GeV
              'thetaGeneration': 'fixed',
              'thetaParams': [90., 90.],               # 2 values: [min, max] in degree
              'phiGeneration': 'uniform',
              'phiParams': [0., 90.],                  # [min, max] in degree
              'vertexGeneration': 'uniform',
              'xVertexParams': [-0.1, 0.1],            # in cm...
              'yVertexParams': [-0.1, 0.1],
              'zVertexParams': [-0.5, 0.5],
             }
"""

particlegun.param(param_pGun)
main.add_module(particlegun)

# EvtGen Simulation:
# TODO: There are newer convenience functions for this task -> Include them!
# Beam parameters
beamparameters = add_beamparameters(main, "Y4S")
evtgenInput = register_module('EvtGenInput')
evtgenInput.logging.log_level = LogLevel.WARNING
main.add_module(evtgenInput)


# ---------------------------------------------------------------------------------------


# setup the geometry (the Geometry and the Gearbox will be ignore in add_simulation if they are already in the path)
setup_Geometry(path=main)

# Detector Simulation:
add_simulation(path=main,
               usePXDDataReduction=False,  # for training one does not want the data reduction
               components=['BeamPipe',
                           'MagneticFieldConstant4LimitedRSVD',
                           'PXD',
                           'SVD',
                           'CDC'])

# this adds the clusters for PXD and SVD (was done by the usePXDDataReduction previously) which are needed in the next steps
add_pxd_reconstruction(path=main)
add_svd_reconstruction(path=main)

# ---------------------------------------------------------------------------------------
# Setting up the MC based track finder.
mctrackfinder = register_module('TrackFinderMCTruthRecoTracks')
mctrackfinder.param('UseCDCHits', False)
mctrackfinder.param('UseSVDHits', True)
# Always use PXD hits! For SVD only, these will be filtered later when converting to SPTrackCand
mctrackfinder.param('UsePXDHits', True)
mctrackfinder.param('Smearing', False)
mctrackfinder.param('MinimalNDF', 6)
mctrackfinder.param('WhichParticles', ['primary'])
mctrackfinder.param('RecoTracksStoreArrayName', 'MCRecoTracks')
main.add_module(mctrackfinder)

# build the name of the output file
outputFileName = outputDir + './'
if os.environ.get('USE_BEAST2_GEOMETRY'):
    outputFileName += "SimEvts_Beast2"
else:
    outputFileName += "SimEvts_Belle2"
outputFileName += '_' + str(rndseed) + '.root'

# Root output. Default filename can be overriden with '-o' basf2 option.
rootOutput = register_module('RootOutput')
rootOutput.param('outputFileName', outputFileName)
main.add_module(rootOutput)

log_to_file('createSim.log', append=False)

process(main)
print(statistics)
