# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This module defines functions to add analysis DQM modules.
"""

from basf2 import *
from modularAnalysis import *


def add_analysis_dqm(path):
    module_names = [m.name() for m in path.modules()]
    if ('ParticleLoader_pi+:HLT' in module_names and 'SoftwareTrigger' in module_names):
        fillParticleList('gamma:physDQM', 'E > 0.15', path=path)
        reconstructDecay('pi0:physDQM -> gamma:physDQM gamma:physDQM', '0.10 < M < 0.15', 1, True, path)
        reconstructDecay('K_S0:physDQM -> pi-:HLT pi+:HLT', '0.48 < M < 0.52', 1, True, path)

        dqm = register_module('PhysicsObjectsDQM')
        dqm.param('PI0PListName', 'pi0:physDQM')
        dqm.param('KS0PListName', 'K_S0:physDQM')
        path.add_module(dqm)
