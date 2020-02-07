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

import sys
import basf2 as b2

particle_type = sys.argv[1]
file_num = sys.argv[2]

# Input File
inputFile = b2.register_module('RootInput')
inputFile.param('inputFileName',
                './MDST_pdg{}_BGx1_{}.root'.format(particle_type, file_num))

# Create paths
main = b2.create_path()

main.add_module(inputFile)

# ECLChargedPIDDataAnalysis module
eclchargedpid = b2.register_module('ECLChargedPIDDataAnalysis')
eclchargedpid.param(
    'rootFileName',
    './RootOut_pdg{}_BGx1_{}.root'.format(particle_type, file_num))

main.add_module(eclchargedpid)

b2.process(main)
print(b2.statistics)
