# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This module defines functions to add IP Position Monitor to DQM modules.
"""

from basf2 import *
from modularAnalysis import *
import vertex as vx


def add_IP_dqm(path):

    mySelection = 'p>1.0'
    mySelection += ' and abs(dz)<2.0 and dr<0.5'
    fillParticleList('mu+:DQM', mySelection, path=path)
    reconstructDecay('Upsilon(4S):IPDQM -> mu+:DQM mu-:DQM', '9.5<M<11.5', path=path)
    vx.KFit('Upsilon(4S):IPDQM', conf_level=0, path=path)

    dqm = register_module('IPDQMExpressReco')
    dqm.set_log_level(LogLevel.INFO)
    dqm.param('Y4SPListName', 'Upsilon(4S):IPDQM')
    path.add_module(dqm)
