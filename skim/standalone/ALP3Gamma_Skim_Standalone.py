#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Three photon skim standalone for the ALP analysis in Dark/low multi.
Physics channel: ee → aγ; a → γγ; Skim LFN code:   18020300
"""

__author__ = "Michael De Nuccio"

import basf2 as b2
import modularAnalysis as ma  # Standard Analysis Tools
from stdPhotons import *
import skimExpertFunctions as expert
skimCode = expert.encodeSkimName('ALP3Gamma')
gb2_setuprel = 'release-04-00-00'

# create a new path
darkskimpath = b2.Path()

# fileList contains the input file which the skim will run on

fileList = expert.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=darkskimpath)

from skim.dark import ALP3GammaList
BeamList = ALP3GammaList(path=darkskimpath)

# output to Udst file
expert.skimOutputUdst(skimCode, BeamList, path=darkskimpath)

# print out Particle List statistics
ma.summaryOfLists(BeamList, path=darkskimpath)

# process the path
b2.process(darkskimpath)

# print out the summary
print(statistics)
