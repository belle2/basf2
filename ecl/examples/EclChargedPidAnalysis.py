#!/usr/bin/env python3
# -*- coding: utf-8 -*-

###############################################################
# This steering file shows how to use ECLChargedPIDDataAnalysis
# module to dump ECL related quantities in an ntuple
# starting from dst root file.
#
# Author: The Belle II Collaboration
# Contributor: Cate MacQueen
# Contact: cmq.centaurus@gmail.com
#
###############################################################

from basf2 import *
from ROOT import Belle2
from modularAnalysis import *
import os.path
import sys

particle_type = sys.argv[1]
file_num = sys.argv[2]

# Input File
inputFile = register_module('RootInput')
inputFile.param('inputFileName', './MDST_pdg' + str(particle_type) + '_BGx1_' + str(file_num) + '.root')

# Create paths
main = create_path()

main.add_module(inputFile)

# ECLChargedPIDDataAnalysis module
eclchargedpid = register_module('ECLChargedPIDDataAnalysis')
eclchargedpid.param(
    'rootFileName',
    './RootOut_pdg' + str(particle_type) + '_BGx1_' + str(file_num) + '.root')

main.add_module(eclchargedpid)

process(main)
print(statistics)
