#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Check KLM strip efficiency payloads.

import ROOT
from ROOT.Belle2 import KLMCalibrationChecker

ROOT.gROOT.SetBatch(True)
ROOT.gStyle.SetOptStat(0)

checker = KLMCalibrationChecker()

checker.setExperimentRun(0, 0)

# checker.setTestingPayload('localdb/database.txt')
# checker.setGlobalTag('')

checker.checkStripEfficiency()
