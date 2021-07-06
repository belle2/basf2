#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os

import basf2
from b2biiConversion import convertBelleMdstToBelleIIMdst
from b2biiMonitors import addBeamParamsConversionMonitors
from b2biiMonitors import addTrackConversionMonitors
from b2biiMonitors import addKshortConversionMonitors
from b2biiMonitors import addConvertedPhotonConversionMonitors
from b2biiMonitors import addLambda0ConversionMonitors
from b2biiMonitors import addNeutralsConversionMonitors
from b2biiMonitors import addMCParticlesConversionMonitors
from b2biiMonitors import addKlongConversionMonitors

os.environ['USE_GRAND_REPROCESS_DATA'] = '1'
os.environ['PGUSER'] = 'g0db'

# Convert
mypath = basf2.create_path()
inputfile = basf2.find_file('b2bii_input_evtgen_exp_07_BptoD0pip-D0toKpipi0-0.mdst', 'examples', False)
convertBelleMdstToBelleIIMdst(inputfile, path=mypath)

# Reconstruct
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
mypath.add_module('Progress')

basf2.process(mypath)

# Print call statistics
print(basf2.statistics)
