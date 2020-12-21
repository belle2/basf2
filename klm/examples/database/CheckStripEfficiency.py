#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
