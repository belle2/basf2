#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" This scripts test CNN_PID_ECL module.

CNN_PID_ECL module extracts CNN probabilities for a track being
muon or pion like.

Input:
    cDST file

Usage:
    basf2 -i <path_to_cdst_file> -n 10 EclCNNPID.py

NOTE:
    In order to use CNN_PID_ECL module it is essential to
    add the following modules:
        - Gearbox
        - Geometry
        - ECLFillCellIdMapping
"""

import basf2 as b2
import modularAnalysis as ma

from cnn_pid_ecl_module import CNN_PID_ECL

__author__ = 'Abtin Narimani Charan'
__copyright__ = 'Copyright 2021 - Belle II Collaboration'
__maintainer__ = 'Abtin Narimani Charan'
__email__ = 'abtin.narimani.charan@desy.de'


mainPath = b2.create_path()
ma.inputMdstList('default', [], path=mainPath)

mainPath.add_module('Gearbox')
mainPath.add_module('Geometry')
mainPath.add_module('ECLFillCellIdMapping',
                    logLevel=b2.LogLevel.INFO)

ma.fillParticleList('mu+:tracks', '', path=mainPath)

mainPath.add_module(CNN_PID_ECL('mu+:tracks'))

mainPath.add_module('Progress')
b2.process(mainPath)
print(b2.statistics)
