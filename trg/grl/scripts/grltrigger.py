##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
# !/usr/bin/env python
# -*- coding: utf-8 -*-

import basf2 as b2


def add_grl_trigger(path, SimulationMode=1):
    """
    do match and produce the objects on grl
    @path modules are added to this path
    @SimulationMode 0: fast simulation 1:full simulation
    """
    match = b2.register_module('TRGGRLMatch')
    match.param('SimulationMode', SimulationMode)
    path.add_module(match)

    objects = b2.register_module('TRGGRLProjects')
    objects.param('SimulationMode', SimulationMode)
    path.add_module(objects)
