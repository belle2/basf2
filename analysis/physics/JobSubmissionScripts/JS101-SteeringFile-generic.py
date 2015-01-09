#!/usr/bin/env python
# -*- coding: utf-8 -*-

######################################################
#
# This is a python steering file which loads 100 files
# of generic MC data of a specific type, background and stream.
# Use the JS201-JobSubmissionScript.py to loop over all generic data.
#
# Contributors: M. Lubej (Jan 2015)
#
######################################################

# load all necessary scripts
from basf2 import *
from modularAnalysis import *

import sys
import os

# input parameters: e.g. basf2 JS101-SteeringFile.py charged 1405 BGx0 s12 3
# this loads 100 files with index 03xx from stream s12, BGx0, mcprod1405 folder, where each file has 1k events

mcType = sys.argv[1]
bkg = sys.argv[2]
stream = sys.argv[3]
fileNo = sys.argv[4]

inputFiles = ['/group/belle2/MC/generic/' + mcType + '/mcprod1405/' + bkg
              + '/*' + stream + '/' + mcType + '_e0001r0' + fileNo + '*.root']
# there is no r000 file, so include r1000 in it's place
if int(fileNo) == 0:
    inputFiles += ['/group/belle2/MC/generic/' + mcType + '/mcprod1405/' + bkg
                   + '/*' + stream + '/' + mcType + '_e0001r1000*.root']

outputFile = 'rootFiles/' + mcType + '/data_' + mcType + '_mc35_' + bkg + '_' \
    + stream + '-' + fileNo + '.root'
directory = os.path.dirname(outputFile)

# check if the rootFiles directory exists
# if not, create it
if not os.path.exists(directory):
    os.makedirs(directory)

# load files
inputMdstList(inputFiles)

# -------------------------------------------

# HERE IS WHERE YOU PUT YOUR ANALYSIS

printDataStore()

# -------------------------------------------

# process the events
process(analysis_main)

# print out the summary
print statistics
