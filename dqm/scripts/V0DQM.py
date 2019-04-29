# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This module defines functions to add the V0 DQM module.
"""

from basf2 import *
from modularAnalysis import *


def add_V0_dqm(path):
    fillParticleList("pi+:V0DQM", '', False, path)
    reconstructDecay('K_S0:V0DQM -> pi-:V0DQM pi+:V0DQM', '', 1, True, path)
    vertexKFit('K_S0:V0DQM', 0.001, '', '', path, True)

    dqm = register_module('V0ObjectsDQM')
    dqm.param('V0PListName', 'K_S0:V0DQM')
    path.add_module(dqm)
