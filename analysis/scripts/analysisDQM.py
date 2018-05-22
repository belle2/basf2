# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This module defines functions to add analysis DQM modules.
"""

from basf2 import *
import modularAnalysis
import stdPhotons


def add_analysis_dqm(path):
    module_names = [m.name() for m in path.modules()]
    if ('ParticleLoader_pi+:HLT' in module_names and
            'ParticleLoader_gamma:HLT' in module_names and
            'SoftwareTrigger' in module_names):
        stdPhotons.stdPhotons('all', path)
        modularAnalysis.reconstructDecay('pi0:all -> gamma:all gamma:all', '0.10 < M < 0.15', 1, True, path)
        modularAnalysis.reconstructDecay('K_S0:all -> pi-:HLT pi+:HLT', '0.48 < M < 0.52', 1, True, path)

        dqm = register_module('PhysicsObjectsDQM')
        path.add_module(dqm)
