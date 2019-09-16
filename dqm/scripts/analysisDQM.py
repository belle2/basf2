# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This module defines functions to add analysis DQM modules.
"""

from basf2 import *
from modularAnalysis import *


def add_analysis_dqm(path):
    module_names = [m.name() for m in path.modules()]
    if ('SoftwareTrigger' in module_names):
        fillParticleList('gamma:physDQM', 'E > 0.15', path=path)
        fillParticleList('pi+:physDQM', 'pt>0.2 and abs(d0) < 2 and abs(z0) < 4', path=path)
        reconstructDecay('pi0:physDQM -> gamma:physDQM gamma:physDQM', '0.10 < M < 0.15', 1, True, path)
        reconstructDecay('K_S0:physDQM -> pi-:physDQM pi+:physDQM', '0.48 < M < 0.52', 1, True, path)

        dqm = register_module('PhysicsObjectsDQM')
        dqm.param('PI0PListName', 'pi0:physDQM')
        dqm.param('KS0PListName', 'K_S0:physDQM')
        path.add_module(dqm)
