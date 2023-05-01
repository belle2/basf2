#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
# This tutorial shows how to use the weighted PID variables with the     #
# dbobject created by SamplePIDAnalysis.py                               #
##########################################################################

import basf2 as b2
import modularAnalysis as ma

localDB = 'localdb/database.txt'
b2.conditions.append_testing_payloads(localDB)

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(filename=b2.find_file('B2pi0D_D2hh_D2hhh_B2munu.root', 'examples', False),
             path=my_path)

ma.fillParticleList('pi+:all', cut='', path=my_path)

matrixName = "PIDCalibrationWeight_Example"
ma.variablesToNtuple('pi+:all',
                     ['pionID', 'kaonID', 'weightedPionID('+matrixName+')', 'weightedKaonID('+matrixName+')'],
                     path=my_path)

b2.process(my_path)
