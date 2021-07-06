#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""This steering file shows how to use EclDataAnalysis
   module to dump ECL related quantities in an ntuple
   starting from dst root file.

The user should provide input and output root filnames
as first and second argument respectively.

Usage:
    $ basf2 EclMaterialAnalysis.py [-- --withBkg]
"""

import argparse
import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction


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
eventInfoSetter.param({'evtNumList': [100000],
                       'runList': [1],
                       'expList': [0]})
mainPath.add_module(eventInfoSetter)

# Random number for generation
b2.set_random_seed(123456)

# Create geometry

# Geometry parameter loader
# gearbox = b2.register_module('Gearbox')
# mainPath.add_module(gearbox)

# Geometry builder
# geometry = b2.register_module('Geometry')
# geometry.param('components', ['ECL'])
# mainPath.add_module(geometry)

components = [  # 'MagneticField',
    # 'BeamPipe',
    'ECL',
    'PXD',
    'SVD',
    'CDC',
    # 'EKLM',
    # 'BKLM',
    # 'TOP',
    'ARICH',
]

# Simulation
# g4sim = b2.register_module('FullSim')
# mainPath.add_module(g4sim)

# Register and add 'ParticleGun' generator module and settings
particleGun = b2.register_module('ParticleGun')
param_particleGun = {
    'pdgCodes': [22],  # 22: photon
    'nTracks': 1,
    'momentumGeneration': 'fixed',
    'momentumParams': [0.5],
    'thetaGeneration': 'uniform',  # uniformCos,
    'thetaParams': [(12.01), (31.36)],
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
    bkgdir = 'bkg/'
    # bkgFiles = glob.glob(bkgdir+'*.root')
    bkgFiles = [
        bkgdir + 'Coulomb_HER_100us.root',
        bkgdir + 'Coulomb_LER_100us.root',
        bkgdir + 'Coulomb_HER_100usECL.root',
        bkgdir + 'Coulomb_LER_100usECL.root',
        bkgdir + 'RBB_HER_100us.root',
        bkgdir + 'RBB_LER_100us.root',
        bkgdir + 'RBB_HER_100usECL.root',
        bkgdir + 'RBB_LER_100usECL.root',
        bkgdir + 'Touschek_HER_100us.root',
        bkgdir + 'Touschek_LER_100us.root',
        bkgdir + 'Touschek_HER_100usECL.root',
        bkgdir + 'Touschek_LER_100usECL.root',
    ]
    # Simulation
    add_simulation(mainPath, bkgfiles=bkgFiles)
else:
    add_simulation(mainPath)

# Reconstruction
add_reconstruction(mainPath)

# display = b2.register_module('Display')
# mainPath.add_module(display)

# Register and add 'ECLDataAnalysis' module
eclDataAnalysis = b2.register_module('ECLDataAnalysis')
eclDataAnalysis.param('rootFileName',
                      'EclDataAnalysis_500MeV_100000_Full_FWD.root')
eclDataAnalysis.param('doTracking', 0)
mainPath.add_module(eclDataAnalysis)

# Process the events and print call statistics
mainPath.add_module('Progress')
b2.process(mainPath)
print(b2.statistics)
