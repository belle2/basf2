# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This module defines functions to add IP Position Monitor to DQM modules.
"""

from basf2 import *
from modularAnalysis import *
import vertex as vx


def add_IP_dqm(path):
    module_names = [m.name() for m in path.modules()]

    applyEventCuts('nTracks==2', path)
    mySelection = 'p>1.0'
    mySelection += ' and abs(dz)<1.0 and dr<0.5'
    fillParticleList('mu+:DQM', mySelection, path=path)
    reconstructDecay('Upsilon(4S):IPDQM -> mu+:DQM mu-:DQM', '9.5<M<11.5', path=path)
    vx.vertexKFit('Upsilon(4S):IPDQM', conf_level=0, path=path, silence_warning=True)

    dqm = register_module('IPDQMExpressReco')
    dqm.set_log_level(LogLevel.INFO)
    dqm.param('Y4SPListName', 'Upsilon(4S):IPDQM')
    path.add_module(dqm)
