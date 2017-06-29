# !/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *


def add_grl_trigger(path, SimulationMode=1, Belle2Phase="Phase2"):
    """
    do match and produce the objects on grl
    @path modules are added to this path
    @SimulationMode 0: fast simulation 1:full simulation
    """
    match = register_module('TRGGRLMatch')
    match.param('SimulationMode', SimulationMode)
    path.add_module(match)

    objects = register_module('TRGGRLProjects')
    objects.param('SimulationMode', SimulationMode)
    objects.param('Belle2Phase', Belle2Phase)
    path.add_module(objects)
