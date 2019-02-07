#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# *****************************************************************************

# title           : 4_CalculateAlignment.py
# description     : Calculate alignment using collected data
# author          : Jakub Kandra (jakub.kandra@karlov.mff.cuni.cz)
# date            : 7. 2. 2019

# *****************************************************************************

from basf2 import *
import sys
import os
from ROOT import Belle2

import alignment
import alignment.setups as setups

import ROOT
from ROOT import Belle2
if len(sys.argv) == 1:
    inputroot = "CollectorOutput.root"
elif len(sys.argv) == 2:
    inputroot = sys.argv[1]

main = create_path()

millepede = alignment.MillepedeCalibration(['VXDAlignment'],
                                           tracks=['RecoTracks'],
                                           particles=[],
                                           vertices=[],
                                           primary_vertices=[],
                                           path=main)

millepede.fixPXDYing()
millepede.fixPXDYang()
millepede.fixSVDPat()
millepede.fixSVDMat()

ladderNumber = [0, 9, 13, 8, 11, 13, 17]
sensorNumber = [0, 3, 3, 3, 4, 5, 6]
# Fix useless sensors in phase 3 early
for layer in range(1, 7):
    for ladder in range(1, ladderNumber[layer]):
        millepede.fixVXDid(layer, ladder, 0)
        for sensor in range(1, sensorNumber[layer]):
            # if layer == 4 and ladder == 3 and sensor == 2:
            millepede.fixVXDid(layer, ladder, sensor, 0, [1, 2, 3, 4, 5, 6])

components = [
    'PXD',
    'SVD',
]

millepede.set_param(components, 'components', 'Geometry')

millepede.set_param(0.0, 'minPValue')
millepede.algo.invertSign()

millepede.set_command('method diagonalization 3 0.1')

# millepede.set_command('constraints.txt')

# millepede.set_command('checkinput')
millepede.set_command('scaleerrors 3.0')
millepede.set_command('entries 10 6 2')

# calibration = millepede.create('vxdcr_alignment', inputroot)
calibration = millepede.create('planarity_alignment', inputroot)

# alignment.calibrate(calibration, args.input)
alignment.calibrate(calibration, inputroot)
