#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
<output>validationTestPlotsA.root</output>
<contact>Thomas Hauth, Thomas.Hauth@kit.edu</contact>
</header>
"""

"""
This file will generate various output into a root-file to have
a fast turn-around during development
"""

VALIDATION_OUTPUT_FILE = 'validationTestPlotsA.root'
N_EVENTS = 10
ACTIVE = True

import array
from ROOT import TFile, TNtuple, TH1F, TF1, TRandom3
from validationtest import generateTestPlots

# make sure we are able to always create the same plots
import basf2
basf2.set_random_seed(1337)

tfile = TFile(VALIDATION_OUTPUT_FILE, "RECREATE")
generateTestPlots("Script A")

tfile.Close()
