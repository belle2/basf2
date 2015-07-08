#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
<output>validationTestEmptyRootFile.root</output>
<contact>Thomas Hauth, Thomas.Hauth@kit.edu</contact>
</header>
"""

"""
This file will not generate an output root file to probe
the validation suite properly handles this case.
"""

VALIDATION_OUTPUT_FILE = 'validationTestEmptyRootFile.root'
N_EVENTS = 10
ACTIVE = True

import array
from ROOT import TFile, TNtuple, TH1F, TF1, TRandom3

# make sure we are able to always create the same plots
import basf2
basf2.set_random_seed(1337)

tfile = TFile(VALIDATION_OUTPUT_FILE, "RECREATE")
tfile.Close()
