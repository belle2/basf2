#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
This file will generate various output into a root-file to have
a fast turn-around during development
"""

VALIDATION_OUTPUT_FILE = 'validationTestPlotsB.root'
N_EVENTS = 10
ACTIVE = True

import array
from ROOT import TFile, TNtuple, TH1F, TF1, TRandom3
from validationtest import generateTestPlots

# make sure we are able to always create the same plots
import basf2
basf2.set_random_seed(20)

tfile = TFile(VALIDATION_OUTPUT_FILE, "RECREATE")
generateTestPlots("")

tfile.Close()
