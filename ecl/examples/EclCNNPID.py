#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

""" This script creates 10 events with 2 particles per event
    then extracts CNN value for each particles.
    The CNN values represent probabilities for a track being
    muon or pion like.

INPUT:
    No input

USAGE:
    basf2 EclCNNPID.py -- [OPTIONS]

EXAMPLE:
    basf2 EclCNNPID.py -- --pdg-code 211

IMPORTANT NOTE:
    In this example script Gearbox and Geometry modules
    are automatically registered in add_simulation().

    In order to use CNN_PID_ECL module in your script,
    it is essential to add the following lines before
    fillParticleList() function:
        mainPath.add_module('Gearbox')
        mainPath.add_module('Geometry')
"""

import sys
import argparse
import basf2 as b2
import modularAnalysis as ma
from simulation import add_simulation
from reconstruction import add_reconstruction

sys.path.append('../scripts/eclCNNPID')
from cnn_pid_ecl_module import CNN_PID_ECL  # noqa


def argparser():
    parser = argparse.ArgumentParser()
    parser.add_argument('--pdg-code', type=int, default=13,
                        choices=[13, -211, -13, 211, 11, -11, 321, -321, 22,
                                 2212, -2212, 1000010020, -1000010020],
                        help='PDG code of particle')
    return parser


args = argparser().parse_args()

pdg_dict = {
    13: ('mu', 'minus'),
    -13: ('mu', 'plus'),
    211: ('pi', 'plus'),
    -211: ('pi', 'minus'),
    11: ('e', 'minus'),
    -11: ('e', 'plus'),
    321: ('K', 'plus'),
    -321: ('K', 'minus'),
    2212: ('p', 'plus'),
    -2212: ('p', 'minus'),
    1000010020: ('deuteron', 'plus'),
    -1000010020: ('anti-deuteron', 'minus'),
    22: ('gamma', 'neutral')
}
particle = pdg_dict[args.pdg_code][0]
charge = pdg_dict[args.pdg_code][1]

mainPath = b2.create_path()

b2.set_log_level(b2.LogLevel.WARNING)

# Register ParticleGun module
particlegun = b2.register_module('ParticleGun')
b2.set_random_seed(123)
particlegun.param('pdgCodes', [args.pdg_code])
particlegun.param('nTracks', 2)
particlegun.param('momentumGeneration', 'uniformPt')
particlegun.param('momentumParams', [0.3, 0.9])
particlegun.param('thetaGeneration', 'uniform')
particlegun.param('thetaParams', [70, 90])  # In the ECL barrel
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 360])
mainPath.add_module(particlegun)

mainPath.add_module(
    'EventInfoSetter',
    expList=1003,  # Exp 1003 is early phase 3
    runList=0,
    evtNumList=10)

# Simulation
add_simulation(mainPath)

# Reconstruction
add_reconstruction(mainPath)

name = f'{particle}+:particles'

if particle in ['gamma', 'deuteron', 'anti-deuteron']:
    name = f'{particle}:particles'

ma.fillParticleList(name, '', path=mainPath)

# CNN_PID_ECL module
mainPath.add_module(CNN_PID_ECL(particleList=name, path=mainPath))

ma.variablesToNtuple(
    decayString=name,
    variables=['cnn_pid_ecl_pion', 'cnn_pid_ecl_muon'],
    treename='particles',
    filename=f'test_{particle}_{charge}_cnn.root',
    path=mainPath
)

mainPath.add_module('Progress')
b2.process(mainPath)
print(b2.statistics)
