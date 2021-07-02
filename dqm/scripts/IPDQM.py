##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This module defines functions to add IP Position Monitor to DQM modules.
"""

import basf2
import modularAnalysis as ma
import vertex as vx


def add_IP_dqm(path):

    mySelection = 'p>1.0'
    mySelection += ' and abs(dz)<2.0 and dr<0.5'
    ma.fillParticleList('mu+:DQM', mySelection, path=path)
    ma.reconstructDecay('Upsilon(4S):IPDQM -> mu+:DQM mu-:DQM', '9.5<M<11.5', path=path)
    vx.kFit('Upsilon(4S):IPDQM', conf_level=0, path=path)

    dqm = basf2.register_module('IPDQMExpressReco')
    dqm.set_log_level(basf2.LogLevel.INFO)
    dqm.param('Y4SPListName', 'Upsilon(4S):IPDQM')
    path.add_module(dqm)
