#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
<input>validationTestPlotsA.root</input>
<output>validationTestPlotsB.root</output>
<contact>Thomas Hauth, Thomas.Hauth@kit.edu</contact>
</header>
"""

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
generateTestPlots("Script B")

tfile.Close()
