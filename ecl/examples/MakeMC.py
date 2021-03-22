#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""This steering file shows how to generate particle gun
   MC samples in DST root format for ECL charged PID analysis.

Input:
    No file is required

Output:
    Mdst file

Usage:
    $ basf2 MakeMC.py -- --pdgCode <integer>
            --momentum <min> <max>
            --theta <min> <max>
            --fileNumber <integer>
"""

import glob
import argparse
import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction

__authors__ = ['Benjamin Oberhof', 'Elisa Manoni',
               'Caitlin MacQueen', 'Marco Milesi',
               'Abtin Narimani Charan']
__copyright__ = 'Copyright 2020 - Belle II Collaboration'
__maintainer__ = 'Abtin Narimani Charan'
__email__ = 'abtin.narimani.charan@desy.de'


def argparser():

    parser = argparse.ArgumentParser()

    parser.add_argument('--pdgCode',
                        type=int,
                        required=True,
                        help='PDG code of particle'
                        'More information:'
                        'http://pdg.lbl.gov/2019/reviews/rpp2018-rev-monte-carlo-numbering.pdf')

    parser.add_argument('--momentum',
                        type=float,
                        required=True,
                        nargs=2,
                        help='Range for momentum of particle in GeV'
                        'First argument is minimum and second is maximum.'
                        'Example: --momentum 0.5 1')

    parser.add_argument('--theta',
                        type=float,
                        required=True,
                        nargs=2,
                        help='Range for polar angle of particle in degree'
                        'First argument is minimum and second is maximum.'
                        'Example: --theta 12.4 155.1')

    parser.add_argument('--fileNumber',
                        type=int,
                        required=True,
                        help='File numbering scheme')
    return parser


args = argparser().parse_args()

momentumRange = list(args.momentum)
thetaRange = list(args.theta)

# Create path. Register necessary modules to this path.
mainPath = b2.create_path()

# Register and add 'EventInfoSetter' module and settings
eventInfoSetter = b2.register_module('EventInfoSetter')
eventInfoSetter.param({'evtNumList': [100],
                       'runList': [1],
                       'expList': [0]})
mainPath.add_module(eventInfoSetter)

# Set log level
b2.set_log_level(b2.LogLevel.INFO)

# Random number for generation
b2.set_random_seed(123456)

# Register and add 'ParticleGun' generator module and settings
particleGun = b2.register_module('ParticleGun')
param_particleGun = {
    'pdgCodes': [args.pdgCode],
    'nTracks': 1,
    'momentumGeneration': 'uniform',
    'momentumParams': momentumRange,
    'thetaGeneration': 'uniform',
    'thetaParams': thetaRange,
    'phiGeneration': 'uniform',
    'phiParams': [0., 360.],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
}
particleGun.param(param_particleGun)
mainPath.add_module(particleGun)

# Add beam background 'BGx1'
pathToBkgDirectory = '/group/belle2/BGFile/OfficialBKG/15thCampaign/bgoverlay_phase3/'
bgFiles = glob.glob(pathToBkgDirectory + '*.root')

# Add simulation
add_simulation(mainPath, bkgfiles=bgFiles)

# Add reconstruction
add_reconstruction(mainPath)

# Register and add 'RootOutput' module
outputFile = b2.register_module('RootOutput')
outputFile.param('outputFileName',
                 'pdg{}_BGx1_{}.mdst.root'.format(args.pdgCode,
                                                  args.fileNumber))
mainPath.add_module(outputFile)

# Process the events and print call statistics
mainPath.add_module('Progress')
b2.process(mainPath)
print(b2.statistics)
