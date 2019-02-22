#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# **************  Flavor Tagging   **************
# * Authors: Fernando Abudinen                  *
# *                                             *
# * This script sets global parameters for the  *
# * evaluation of the flavor tagger's           *
# * performance.                                *
#                                               *
# ***********************************************

import ROOT
from array import array
import numpy as np


class Quiet:
    """Context handler class to quiet errors in a 'with' statement"""

    def __init__(self, level=ROOT.kInfo + 1):
        """Class constructor"""
        #: the level to quiet
        self.level = level

    def __enter__(self):
        """Enter the context"""
        #: the previously set level to be ignored
        self.oldlevel = ROOT.gErrorIgnoreLevel
        ROOT.gErrorIgnoreLevel = self.level

    def __exit__(self, type, value, traceback):
        """Exit the context"""
        ROOT.gErrorIgnoreLevel = self.oldlevel


# dilution factor binning of Belle
r_subsample = array('d', [
    0.0,
    0.1,
    0.25,
    0.5,
    0.625,
    0.75,
    0.875,
    1.0])
r_size = len(r_subsample)
rbins = np.array(r_subsample)

# All possible tagging categories
categories = [
    'Electron',
    'IntermediateElectron',
    'Muon',
    'IntermediateMuon',
    'KinLepton',
    'IntermediateKinLepton',
    'Kaon',
    'KaonPion',
    'SlowPion',
    'FSC',
    'MaximumPstar',
    'FastHadron',
    'Lambda']
