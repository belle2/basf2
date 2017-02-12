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
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
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

# set the location of the Belle DB server
# options are: ekpbelle.physik.uni-karlsruhe.de
# or ekpbelle.physik.uni-karlsruhe.de (if you're running outside KEKCC computers)
os.environ['BELLE_POSTGRES_SERVER'] = 'can01'
os.environ['USE_GRAND_REPROCESS_DATA'] = '1'

# Convert
convertBelleMdstToBelleIIMdst(inputBelleMDSTFile)

# Reconstruct
# first the gearbox needs to be loaded
loadGearbox()

# Create monitoring histograms
addBeamParamsConversionMonitors()
addTrackConversionMonitors()
addKshortConversionMonitors()
addKlongConversionMonitors()
addLambda0ConversionMonitors()
addConvertedPhotonConversionMonitors()
addNeutralsConversionMonitors()
addMCParticlesConversionMonitors()

# progress
progress = register_module('Progress')
analysis_main.add_module(progress)

process(analysis_main)

# Print call statistics
print(statistics)
