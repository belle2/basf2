#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""Pure CsI simulation for the ECL

Input:
    No file is reguired

Output:
    Mdst file named 'Ecl_Pure.mdst.root'

Usage:
    $ basf2 EclPureCsIExample.py [-- --withBkg]
"""

import os
import glob
import argparse
import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction

__authors__ = ['Benjamin Oberhof', 'Abtin Narimani Charan']
__copyright__ = 'Copyright 2020 - Belle II Collaboration'
__maintainer__ = 'Abtin Narimani Charan'
__email__ = 'abtin.narimani.charan@desy.de'


def argparser():

    parser = argparse.ArgumentParser()

    parser.add_argument('--withBkg',
                        action='store_true',
                        default=False,
                        help='Add beam background'
                        'Default is False i.e. no beam background.')
    return parser


args = argparser().parse_args()

# Create path. Register necessary modules to this path.
mainPath = b2.create_path()

# Register and add 'EventInfoSetter' module and settings
eventInfoSetter = b2.register_module('EventInfoSetter')
eventInfoSetter.param({'evtNumList': [1000],
                       'runList': [1],
                       'expList': [0]})
mainPath.add_module(eventInfoSetter)

# Random number for generation
b2.set_random_seed(123456)

# Register and add 'ParticleGun' generator module and settings
particleGun = b2.register_module('ParticleGun')
param_particleGun = {
    'pdgCodes': [22],  # 22: photon
    'nTracks': 1,
    'momentumGeneration': 'fixed',
    'momentumParams': [.1],
    'thetaGeneration': 'uniform',
    'thetaParams': [13., 150.],
    'phiGeneration': 'uniform',
    'phiParams': [0., 360.],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0., 0.],
    'yVertexParams': [0., 0.],
    'zVertexParams': [0., 0.],
}
particleGun.param(param_particleGun)
mainPath.add_module(particleGun)

if args.withBkg:
    # Add beam background overlay files
    bgFiles = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '*.root')
    # Add simulation
    add_simulation(mainPath, bkgfiles=bgFiles)
else:
    # Simulation
    add_simulation(mainPath)

# Reconstruction
add_reconstruction(mainPath)

# Register and add 'ECLDigitizerPureCsI' module and settings
eclDigitizerPureCsI = b2.register_module('ECLDigitizerPureCsI')
eclDigitizerPureCsI.param('adcTickFactor', 8)
eclDigitizerPureCsI.param('sigmaTrigger', 0.)
eclDigitizerPureCsI.param('elecNoise', 1.3)
eclDigitizerPureCsI.param('photostatresolution', 0.4)
eclDigitizerPureCsI.param('sigmaTrigger', 0)
eclDigitizerPureCsI.param('LastRing', 12)
eclDigitizerPureCsI.param('NoCovMatrix', 1)
eclDigitizerPureCsI.param('Background', 0)
mainPath.add_module(eclDigitizerPureCsI)

# Register and add 'ECLDigitCalibratorPureCsI' module
eclDigitCalibratorPureCsI = b2.register_module('ECLDigitCalibratorPureCsI')
# It is IMPORTANT to set 'simulatePure' to 1 for pure CsI simulation.
eclDigitCalibratorPureCsI.param('simulatePure', 1)
mainPath.add_module(eclDigitCalibratorPureCsI)

# Register and add 'ECLCRFinderPureCsI' module
eclCRFinderPureCsI = b2.register_module('ECLCRFinderPureCsI')
mainPath.add_module(eclCRFinderPureCsI)

# Register and add 'ECLLocalMaximumFinderPureCsI' module
eclLocalMaximumFinderPureCsI = b2.register_module('ECLLocalMaximumFinderPureCsI')
mainPath.add_module(eclLocalMaximumFinderPureCsI)

# Register and add 'ECLSplitterN1PureCsI' module
eclSplitterN1PureCsI = b2.register_module('ECLSplitterN1PureCsI')
mainPath.add_module(eclSplitterN1PureCsI)

# Register and add 'ECLSplitterN2PureCsI' module
eclSplitterN2PureCsI = b2.register_module('ECLSplitterN2PureCsI')
mainPath.add_module(eclSplitterN2PureCsI)

# Register and add 'ECLShowerCorrectorPureCsI' module
eclShowerCorrectorPureCsI = b2.register_module('ECLShowerCorrectorPureCsI')
mainPath.add_module(eclShowerCorrectorPureCsI)

# Register and add 'ECLShowerCalibratorPureCsI' module
eclShowerCalibratorPureCsI = b2.register_module('ECLShowerCalibratorPureCsI')
mainPath.add_module(eclShowerCalibratorPureCsI)

# Register and add 'ECLShowerShapePureCsI' module
eclShowerShapePureCsI = b2.register_module('ECLShowerShapePureCsI')
mainPath.add_module(eclShowerShapePureCsI)

# Register and add 'ECLCovarianceMatrixPureCsI' module
eclCovarianceMatrixPureCsI = b2.register_module('ECLCovarianceMatrixPureCsI')
mainPath.add_module(eclCovarianceMatrixPureCsI)

# Register and add 'ECLFinalizerPureCsI' module
eclFinalizerPureCsI = b2.register_module('ECLFinalizerPureCsI')
mainPath.add_module(eclFinalizerPureCsI)

# Register and add 'MCMatcherECLClustersPureCsI' module
mcMatcherECLClustersPureCsI = b2.register_module('MCMatcherECLClustersPureCsI')
mainPath.add_module(mcMatcherECLClustersPureCsI)

# Register and add 'ECLDataAnalysis' module
eclDataAnalysis = b2.register_module('ECLDataAnalysis')
eclDataAnalysis.param('writeToRoot', 1)
eclDataAnalysis.param('rootFileName', 'Ecl_Pure_CsI_Example.root')
eclDataAnalysis.param('doSimulation', 0)
eclDataAnalysis.param('doTracking', 1)
eclDataAnalysis.param('doPureCsIStudy', 1)
mainPath.add_module(eclDataAnalysis)

# Register and add 'RootOutput' module
outputFile = b2.register_module('RootOutput')
outputFile.param('outputFileName', 'Ecl_Pure.mdst.root')
mainPath.add_module(outputFile)

# Process the events and print call statistics
mainPath.add_module('Progress')
b2.process(mainPath)
print(b2.statistics)
