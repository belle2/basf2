#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys

from basf2 import *
import ROOT
from ROOT import Belle2
from modularAnalysis import inputMdstList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import fillParticleList
from modularAnalysis import fillConvertedPhotonsList
from modularAnalysis import loadGearbox
from modularAnalysis import vertexKFit
from modularAnalysis import vertexRave
from modularAnalysis import printVariableValues
from b2biiConversion import convertBelleMdstToBelleIIMdst, setupB2BIIDatabase
from b2biiMonitors import addBeamParamsConversionMonitors
from b2biiMonitors import addTrackConversionMonitors
from b2biiMonitors import addKshortConversionMonitors
from b2biiMonitors import addConvertedPhotonConversionMonitors
from b2biiMonitors import addLambda0ConversionMonitors
from b2biiMonitors import addNeutralsConversionMonitors
from b2biiMonitors import addMCParticlesConversionMonitors
from b2biiMonitors import addKlongConversionMonitors

if len(sys.argv) != 3:
    sys.exit('Must provide two input parameters: [mc|data] [input_Belle_MDST_file].\n'
             'A small example Belle MDST file can be downloaded from '
             'http://www-f9.ijs.si/~zupanc/evtgen_exp_07_BptoD0pip-D0toKpipi0-0.mdst')

mc_or_data = sys.argv[1].lower()
isMC = {"mc": True, "data": False}.get(mc_or_data, None)
if isMC is None:
    sys.exit('First parameter must be "mc" or "data" to indicate whether we run on MC or real data')

setupB2BIIDatabase(isMC)

inputBelleMDSTFile = sys.argv[2]

os.environ['USE_GRAND_REPROCESS_DATA'] = '1'

# Convert
mypath = create_path()
convertBelleMdstToBelleIIMdst(inputBelleMDSTFile, path=mypath)

# Reconstruct
# first the gearbox needs to be loaded
loadGearbox(mypath)

# Create monitoring histograms
addBeamParamsConversionMonitors(path=mypath)
addTrackConversionMonitors(path=mypath)
addKshortConversionMonitors(path=mypath)
addKlongConversionMonitors(path=mypath)
addLambda0ConversionMonitors(path=mypath)
addConvertedPhotonConversionMonitors(path=mypath)
addNeutralsConversionMonitors(path=mypath)
addMCParticlesConversionMonitors(path=mypath)

# progress
progress = register_module('Progress')
mypath.add_module(progress)

process(mypath)

# Print call statistics
print(statistics)
