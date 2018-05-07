# !/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import os


def statisticstime_hltdqm(path):
    """
    histograms to show the statistics timing information in hlt reconstruction
    """
    modstat = register_module('StatisticsTimingHLTDQM')
    path.add_module(modstat)


def cosmic_hltdqm(path):
    """
    histograms to show the cosmic information in hlt reconstruction
    """
    cosmicrec = register_module('CosmicRayHLTDQM')
    path.add_module(cosmicrec)
    statisticstime_hltdqm(path)


def standard_hltdqm(path):
    """
    histograms to show the trigger information in hlt
    """
    hltrec = register_module('SoftwareTriggerHLTDQM')
    path.add_module(hltrec)
    statisticstime_hltdqm(path)
