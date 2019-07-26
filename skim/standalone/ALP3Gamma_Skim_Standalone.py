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
import argparse
skimCode = encodeSkimName('ALP3Gamma')
gb2_setuprel = 'release-03-02-00'

# Read optional --data argument
parser = argparse.ArgumentParser()
parser.add_argument('--data',
                    help='Provide this flag if running on data.',
                    action='store_true', default=False)
args = parser.parse_args()

if args.data:
    use_central_database("data_reprocessing_prompt_bucket6")

# create a new path
darkskimpath = Path()

# fileList contains the input file which the skim will run on

fileList = get_test_file("mixedBGx1", "MC12")
inputMdstList('default', fileList, path=darkskimpath)

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
