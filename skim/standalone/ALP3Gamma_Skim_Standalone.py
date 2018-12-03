#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Three photon skim standalone for the ALP analysis in Dark/low multi.
Physics channel: ee → aγ; a → γγ; Skim LFN code:   18020300
"""

__author__ = "Michael De Nuccio"

from basf2 import *
from modularAnalysis import *  # Standard Analysis Tools
from stdPhotons import *
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file
skimCode = encodeSkimName('ALP3Gamma')

# create a new path
darkskimpath = Path()

# fileList contains the input file which the skim will run on
fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]
inputMdstList('MC9', fileList, path=darkskimpath)

from skim.dark import ALP3GammaList
BeamList = ALP3GammaList(path=darkskimpath)

# output to Udst file
skimOutputUdst(skimCode, BeamList, path=darkskimpath)

# print out Particle List statistics
summaryOfLists(BeamList, path=darkskimpath)

# process the path
process(darkskimpath)

# print out the summary
print(statistics)
