#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""This steering file shows how to use 'EclDataAnalysis'
   module to dump ECL-related quantities in an ntuple.

Input:
    File with DST format

Output:
    Ntuple with ECL-related quantities

Usage:
    $ basf2 -i <path_to_input_file> -n <number_of_events>
            EclDataAnalysis.py [-- --withBkg]
"""

import os
import glob
import argparse
import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction

__authors__ = ['Benjamin Oberhof', 'Elisa Manoni',
               'Abtin Narimani Charan']
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
eventInfoSetter.param({'evtNumList': [100],
                       'runList': [1],
                       'expList': [0]})
mainPath.add_module(eventInfoSetter)

# Random number for generation
b2.set_random_seed(123456)

# Register and add 'ParticleGun' generator module and settings
particleGun = b2.register_module('ParticleGun')
param_particleGun = {
    'pdgCodes': [13],  # 13: muon
    'nTracks': 1,
    'momentumGeneration': 'fixed',
    'momentumParams': [0.5],
    'thetaGeneration': 'uniform',
    'thetaParams': [0., 30.],
    'phiGeneration': 'uniform',
    'phiParams': [0., 360.],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
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

# Register and add 'ECLDataAnalysis' module and settings
eclDataAnalysis = b2.register_module('ECLDataAnalysis')
eclDataAnalysis.param('rootFileName',
                      'EclDataAnalysis_Test.root')
eclDataAnalysis.param('doTracking', 1)
eclDataAnalysis.param('doPureCsIStudy', 0)
mainPath.add_module(eclDataAnalysis)

# Process the events and print call statistics
mainPath.add_module('Progress')
b2.process(mainPath)
print(b2.statistics)
