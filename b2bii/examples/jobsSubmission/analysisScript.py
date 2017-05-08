#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This is the main file for the analysis script
# G. Caria

import os
import sys
from tools import *
from basf2 import *
from modularAnalysis import inputMdstList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from modularAnalysis import fillParticleList
from modularAnalysis import fillConvertedPhotonsList
from modularAnalysis import loadGearbox
from modularAnalysis import vertexKFit
from modularAnalysis import vertexRave
from modularAnalysis import printVariableValues
from stdCharged import *

import b2biiConversion
import ROOT
from ROOT import Belle2
ROOT.Belle2.BFieldManager.getInstance().setConstantOverride(0, 0, 1.5 * ROOT.Belle2.Unit.T)


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
    streamNo = sys.argv[4]
    dataType = sys.argv[5]
    belleLevel = sys.argv[6]
    minRunNo = sys.argv[7]
    maxRunNo = sys.argv[8]
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

b2biiConversion.setupB2BIIDatabase(isMC)

os.environ['USE_GRAND_REPROCESS_DATA'] = '1'

if isMC:
    url = getBelleUrl_mc(expNo, minRunNo, maxRunNo,
                         eventType, dataType, belleLevel, streamNo)
else:
    url = getBelleUrl_data(expNo, minRunNo, maxRunNo,
                           skimType, dataType, belleLevel)

b2biiConversion.convertBelleMdstToBelleIIMdst(url, applyHadronBJSkim=True)
loadGearbox()


# ------- Output file

outDir = './analysisOutput'

filenameEnd = '_'.join(sys.argv[2:]) + '.root'

outputFileName = outDir + '/output_' + filenameEnd

ntupleFile(outputFileName)


# ------- Rest of analysis script goes here...

# this sample code is take from b2bii/examples

fillParticleList('pi+:all', '')

toolsTrackPI = ['EventMetaData', 'pi+']
toolsTrackPI += ['Kinematics', '^pi+']

ntupleTree('pion', 'pi+:all', toolsTrackPI)

# progress
progress = register_module('Progress')
analysis_main.add_module(progress)

process(analysis_main)

# Print call statistics
print(statistics)
