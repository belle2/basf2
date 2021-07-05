#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Example of writing TOP cdst variable information using analysis package tools
#
# Sam Cunliffe (sam.cunliffe@desy.de)
# November 2018

from basf2 import create_path, use_central_database, process, statistics
from modularAnalysis import variablesToNtuple
from ROOT import gSystem

# you need to choose a consistent global tag that corresponds to the data
# you are analysing this will change from production to production
inputdata = "/hsm/belle2/bdata/Data/release-02-00-01/DB00000425/prod00000005/e0003/4S/r00784/all/cdst/sub00/*.root"
datadbtag = "data_reprocessing_prod5"

# Load the top libraries -- needed until the "top/variables" directory
# gets upgraded and variables linked automatically
gSystem.Load('libtop.so')

# setup and input
path = create_path()
use_central_database(datadbtag)
path.add_module('RootInput', inputFileName=inputdata)
goodtracks = 'abs(dz) < 2.0 and abs(dr) < 0.5 and pt > 0.15 and nCDCHits > 0'
# units:                 cm                cm          GeV/c
# note that this selection is more-or-less sensible on phase2 data
path.add_module('ParticleLoader',
                decayStringsWithCuts=[('K+:goodtracks', goodtracks)])

# variables one might care about
variables_of_interest = [
    "topModuleDigitCount",
    "topModuleReflectedDigitCount",
    "topModuleDigitGapSize",
    "px", "py", "pz", "E", "pt", "p", "theta", "phi"
]
variablesToNtuple("K+:goodtracks", variables=variables_of_interest, path=path)

# process path
process(path, 10)  # process 10 events, override with basf2 -n option
print(statistics)
