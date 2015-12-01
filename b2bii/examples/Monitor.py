#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys

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
from b2biiConversion import convertBelleMdstToBelleIIMdst
from b2biiMonitors import addBeamParamsConversionMonitors
from b2biiMonitors import addTrackConversionMonitors
from b2biiMonitors import addKshortConversionMonitors
from b2biiMonitors import addNeutralsConversionMonitors

if len(sys.argv) != 2:
    sys.exit('Must provide one input parameter: [input_Belle_MDST_file].\n'
             'A small example Belle MDST file can be downloaded from '
             'http://www-f9.ijs.si/~zupanc/evtgen_exp_07_BptoD0pip-D0toKpipi0-0.mdst')

inputBelleMDSTFile = sys.argv[1]

# set the location of the Belle DB server
# options are: ekpbelle.physik.uni-karlsruhe.de
# or ekpbelle.physik.uni-karlsruhe.de (if you're running outside KEKCC computers)
os.putenv('BELLE_POSTGRES_SERVER', 'can51')
os.putenv('USE_GRAND_REPROCESS_DATA', '1')

# Convert
convertBelleMdstToBelleIIMdst(inputBelleMDSTFile)

# Reconstruct
# first the gearbox needs to be loaded
loadGearbox()

# Create monitoring histograms
addBeamParamsConversionMonitors()
addTrackConversionMonitors()
addKshortConversionMonitors()
addNeutralsConversionMonitors()

# progress
progress = register_module('Progress')
analysis_main.add_module(progress)

process(analysis_main)

# Print call statistics
print(statistics)
