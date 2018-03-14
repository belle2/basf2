#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# This steering file shows how to use EclDataAnalysis
# module to dump ECL related quantities in an ntuple
# starting from dst root file.
#
# Author: The Belle II Collaboration
# Contributors: Benjamin Oberhof
#
########################################################

# import os
# import random
from basf2 import *
from ROOT import Belle2
from modularAnalysis import *
import os.path
import sys

particle_type = sys.argv[1]
p_min = sys.argv[2]  # input in MeV
p_max = sys.argv[3]  # input in MeV
theta_min = sys.argv[4]  # input in deg
theta_max = sys.argv[5]  # input in deg

# Input File
inputFile = register_module('RootInput')
inputFile.param('inputFileName', '/hsm/belle2/bdata/users/cguenthe/LeptonIDWork/MDST/BGx1/pdg' + str(particle_type) +
                '_mom' + str(p_min) + 'to' + str(p_max) + 'MeV_theta' + str(theta_min) + 'to' + str(theta_max) + 'deg.root')

# Create paths
main = create_path()

main.add_module(inputFile)

# eclDataAnalysis module
eclchargedpid = register_module('ECLChargedPID')
eclchargedpid.param(
    'rootFileName',
    '/home/belle/cguenthe/B2Analysis/LeptonIDWork/InputRoot/BGx1/pdg' +
    str(particle_type) +
    '_mom' +
    str(p_min) +
    'to' +
    str(p_max) +
    'MeV_theta' +
    str(theta_min) +
    'to' +
    str(theta_max) +
    'deg.root')

main.add_module(eclchargedpid)

process(main)
print(statistics)
