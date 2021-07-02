#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""Dump digit information for pure CsI vs CsI(Tl)
   using 'ParticleGun' to generate single particles
   enabling pure CsI digitization
   dump useful in a flat ntuple + enriched mdst

Usage:
    $ basf2 ECLPureCsIStudy.py -- --momentum <number>
            --fileNamePrefix <name> --elecNoise <number>
            --photoStatResolution <number>
            [--beamBkgPath <path_to_files> --pdgCode <integer>]
"""

import glob
import argparse
import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction
from reconstruction import add_mdst_output
from beamparameters import add_beamparameters

__authors__ = ['Guglielmo De Nardo', 'Abtin Narimani Charan']
__copyright__ = 'Copyright 2020 - Belle II Collaboration'
__maintainer__ = 'Abtin Narimani Charan'
__email__ = 'abtin.narimani.charan@desy.de'


def argparser():

    parser = argparse.ArgumentParser()

    parser.add_argument('--momentum',
                        type=float,
                        required=True,
                        help='Fixed momentum of particle equivalent in GeV')

    parser.add_argument('--fileNamePrefix',
                        required=True,
                        help='Output file name')

    parser.add_argument('--elecNoise',
                        type=float,
                        required=True,
                        help='Electronics noise energy equivalent in MeV')

    parser.add_argument('--photoStatResolution',
                        type=float,
                        required=True,
                        help='Sigma for 1 MeV energy deposit')

    parser.add_argument('--beamBkgPath',
                        help='If you want to add beam background, pass this'
                        'option with path to beam background files.')

    parser.add_argument('--pdgCode',
                        type=int,
                        help='PDG code of particle'
                        ' For more information:'
                        'http://pdg.lbl.gov/2019/reviews/rpp2018-rev-monte-carlo-numbering.pdf')
    return parser


args = argparser().parse_args()

# Create path. Register necessary modules to this path.
mainPath = b2.create_path()

# Register and add 'EventInfoSetter' module
eventInfoSetter = b2.register_module('EventInfoSetter')
mainPath.add_module(eventInfoSetter)

if args.pdgCode:
    # single particle generator settings
    particleGun = b2.register_module('ParticleGun')
    param_particleGun = {
        'pdgCodes': [args.pdgCode],
        'nTracks': 1,
        'momentumGeneration': 'fixed',
        'momentumParams': [args.momentum],
        'thetaGeneration': 'uniform',
        'thetaParams': [13.0, 30.0],
        'phiGeneration': 'uniform',
        'phiParams': [0., 360.],
        'vertexGeneration': 'uniform',
        'xVertexParams': [0.0, 0.0],
        'yVertexParams': [0.0, 0.0],
        'zVertexParams': [0.0, 0.0],
    }
    particleGun.param(param_particleGun)
    mainPath.add_module(particleGun)
else:
    # Beam parameters
    beamparameters = add_beamparameters(mainPath, 'Y4S')
    evtGenInput = b2.register_module('EvtGenInput')
    mainPath.add_module(evtGenInput)

if args.beamBkgPath:
    # Add beam background
    bgFiles = glob.glob(args.beamBkgPath + '/*.root')
    # Simulation
    add_simulation(mainPath, bkgfiles=bgFiles)
else:
    add_simulation(mainPath)

# Reconstruction
add_reconstruction(mainPath, components='ECL')

# Register and add 'ECLDigitizerPureCsI' module
eclDigitizerPureCsI = b2.register_module('ECLDigitizerPureCsI')
eclDigitizerPureCsI.param('adcTickFactor', 8)
eclDigitizerPureCsI.param('sigmaTrigger', 0.)
eclDigitizerPureCsI.param('elecNoise',
                          args.elecNoise)
eclDigitizerPureCsI.param('photostatresolution',
                          args.photoStatResolution)
eclDigitizerPureCsI.param('sigmaTrigger', 0)
eclDigitizerPureCsI.param('LastRing', 12)
eclDigitizerPureCsI.param('NoCovMatrix', 1)
if args.withBkg:
    eclDigitizerPureCsI.param('Background', 1)
mainPath.add_module(eclDigitizerPureCsI)

# Register and add 'ECLDigitCalibratorPureCsI' module
eclDigitCalibratorPureCsI = b2.register_module('ECLDigitCalibratorPureCsI')
mainPath.add_module(eclDigitCalibratorPureCsI)

# Register and add 'ECLReconstructorPureCsI' module
eclReconstructorPureCsI = b2.register_module('ECLReconstructorPureCsI')
mainPath.add_module(eclReconstructorPureCsI)

# Register and add 'ECLDigiStudy' module
eclDigiStudy = b2.register_module('ECLDigiStudy')
eclDigiStudy.param('outputFileName',
                   args.fileNamePrefix + '_digi')
mainPath.add_module(eclDigiStudy)

# display = b2.register_module('Display')
# mainPath.add_module(display)

# Add output mdst file with ECL information
add_mdst_output(
    mainPath,
    mc=True,
    filename=args.fileNamePrefix + '.mdst.root',
    additionalBranches=[
        'ECLHits',
        'ECLClustersPureCsI',
        'ECLDsps',
        'ECLDspsPureCsI',
        'ECLDigits',
        'ECLCalDigits',
        'ECLDigitsPureCsI',
        'ECLCalDigitsPureCsI',
        'ECLDigitsPureCsIToECLDspsPureCsI',
        'ECLDigitsPureCsIToECLHits'])

# Show progress of processing
progressBar = b2.register_module('ProgressBar')
mainPath.add_module(progressBar)

# Process the events and print call statistics
mainPath.add_module('Progress')
b2.process(mainPath)
print(b2.statistics)
