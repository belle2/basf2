#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# This is the main file for the analysis script

import os
import sys
from tools import getBelleUrl_data, getBelleUrl_mc
import basf2 as b2
from modularAnalysis import variablesToNtuple
from modularAnalysis import fillParticleList

from b2biiConversion import convertBelleMdstToBelleIIMdst


# ------- Arguments sorting

mc_or_data = sys.argv[1].lower()
isMC = {"mc": True, "data": False}.get(mc_or_data, None)
if isMC is None:
    sys.exit('First parameter must be "mc" or "data" to indicate whether we run on MC or real data')

if isMC:
    if len(sys.argv) != 9:
        sys.exit('Must provide all 8 parameters !')
    expNo = sys.argv[2]
    eventType = sys.argv[3]
    dataType = sys.argv[4]
    belleLevel = sys.argv[5]
    minRunNo = sys.argv[6]
    maxRunNo = sys.argv[7]
    streamNo = sys.argv[8]
else:
    if len(sys.argv) != 8:
        sys.exit('Must provide all 7 parameters !')
    expNo = sys.argv[2]
    skimType = sys.argv[3]
    dataType = sys.argv[4]
    belleLevel = sys.argv[5]
    minRunNo = sys.argv[6]
    maxRunNo = sys.argv[7]


# ------- B2BII

os.environ['PGUSER'] = 'g0db'
os.environ['USE_GRAND_REPROCESS_DATA'] = '1'

if isMC:
    url = getBelleUrl_mc(expNo, minRunNo, maxRunNo,
                         eventType, dataType, belleLevel, streamNo)
else:
    url = getBelleUrl_data(expNo, minRunNo, maxRunNo,
                           skimType, dataType, belleLevel)

mypath = b2.create_path()
convertBelleMdstToBelleIIMdst(url, applySkim=True, path=mypath)


# ------- Output file

outDir = './analysisOutput'

filenameEnd = '_'.join(sys.argv[2:]) + '.root'

outputFileName = outDir + '/output_' + filenameEnd

# ------- Rest of analysis script goes here...

# this sample code is taken from b2bii/examples

fillParticleList('pi+:all', '', path=mypath)

kinematic_variables = ['px', 'py', 'pz', 'E']

variablesToNtuple(
    'pi+:all', kinematic_variables, filename=outputFileName, path=mypath)

# progress
progress = b2.register_module('Progress')
mypath.add_module(progress)

b2.process(mypath)

# Print call statistics
print(b2.statistics)
