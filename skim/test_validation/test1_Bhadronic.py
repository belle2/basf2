#!/usr/bin/env python3
# -*- coding: utf-8 -*-

__author__ = "R. Cheaib & S. Hollitt"


import sys
import glob
import os.path

from basf2 import *
from modularAnalysis import *
from analysisPath import analysis_main
from beamparameters import add_beamparameters
from skimExpertFunctions import *


fileList = ['../Bhadronic.dst.root']

inputMdstList('MC9', fileList)

applyEventCuts('R2EventLevel<0.4 and nTracks>=4')

# Run FEI
from fei import backward_compatibility_layer
backward_compatibility_layer.pid_renaming_oktober_2017()
use_central_database('GT_gen_ana_004.40_AAT-parameters', LogLevel.DEBUG, 'fei_database')

import fei
particles = fei.get_default_channels(neutralB=True, chargedB=False, hadronic=True, semileptonic=False, KLong=False)
configuration = fei.config.FeiConfiguration(prefix='FEIv4_2018_MC9_release_02_00_00', training=False, monitor=False)
feistate = fei.get_path(particles, configuration)
analysis_main.add_path(feistate.path)

analysis_main.add_module('MCMatcherParticles', listName='B0:generic', looseMCMatching=True)

# Hadronic B0 skim
from skim.fei import *
B0hadronicList = B0hadronic()
skimOutputUdst('Bhadronic.udst.root', B0hadronicList)
summaryOfLists(B0hadronicList)

# Suppress noisy modules, and then process
setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
