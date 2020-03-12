# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This module defines functions to add analysis MiraBelle modules.
"""

from basf2 import *
from modularAnalysis import *


def add_analysis_mirabelle(path):
    module_names = [m.name() for m in path.modules()]
    print(module_names)
    if ('SoftwareTrigger' in module_names):
        fillParticleList('mu+:physMiraBelle', '', path=path)
        mirabelle = register_module('PhysicsObjectsMiraBelle')
        mirabelle.param('MuPListName', 'mu+:physMiraBelle')
        path.add_module(mirabelle)
