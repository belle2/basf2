#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# *****************************************************************************

# title           : 3B_CalculateAlignment.py
# description     : Calculate alignment using collected data
# author          : Jakub Kandra (jakub.kandra@karlov.mff.cuni.cz)
# date            : 8. 2. 2018

# *****************************************************************************

import os
import sys
import inspect
from basf2 import *
from ROOT import Belle2

import alignment
import alignment.setups as setups

import argparse
parser = argparse.ArgumentParser(
    description="Universal tool to run Millepede collection and calibration (outside CAF, but easily portable)")

# Basic commands for the tool
parser.add_argument(
    '-i',
    '--input',
    action='store',
    type=str,
    help='Input file for calibration (not basf2 option!) if output of collection is not RootOutput.root',
    dest='input',
    default='RootOutput.root')

args, basf2_args = parser.parse_known_args()

main = create_path()

millepede = alignment.MillepedeCalibration(['VXDAlignment', 'BeamParameters'],
                                           tracks=['CosmicRecoTracks'],
                                           particles=['mu+:bbmu'],
                                           vertices=[],
                                           primary_vertices=['Z0:mumu'],
                                           path=main)
millepede.algo.invertSign()

# millepede.fixPXDYing()
# millepede.fixPXDYang()
# millepede.fixSVDPat()
# millepede.fixSVDMat()

# Fix all ladders (only ladder=1 in Beast II)
# ladder = 1
# for layer in range(1, 7):
#    millepede.fixVXDid(layer, ladder, 0)

beast2_sensors = [
    (1, 1, 1), (1, 1, 2),
    (2, 1, 1), (2, 1, 2),
    (3, 1, 1), (3, 1, 2),
    (4, 1, 1), (4, 1, 2), (4, 1, 3),
    (5, 1, 1), (5, 1, 2), (5, 1, 3), (5, 1, 4),
    (6, 1, 1), (6, 1, 2), (6, 1, 3), (6, 1, 4), (6, 1, 5)
]

# for sensor_id in beast2_sensors:
#    layer, ladder, sensor = sensor_id
#    millepede.fixVXDid(layer, ladder, sensor, parameters=[1, 2, 3, 4, 5, 6])

# Simple in-place module modifications .. no need to construct path again
millepede.set_param('/geometry/Beast2_phase2.xml', 'fileName', 'Gearbox')
components = [
    'MagneticField',
    'BeamPipe',
    'PXD',
    'SVD',
    'CDC',
    'EKLM',
    'BKLM']

millepede.set_param(components, 'components', 'Geometry')

calibration = millepede.create('beast2_alignment', args.input)

alignment.calibrate(calibration, args.input)
