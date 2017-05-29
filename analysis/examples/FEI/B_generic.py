#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2017

from basf2 import *
from modularAnalysis import *

path = create_path()
inputMdstList('default', [], path)

import fei
particles = fei.get_default_channels()
configuration = fei.config.FeiConfiguration(prefix='FEI_Belle2_Generic_2017_1', training=True)
feistate = fei.get_path(particles, configuration)
path.add_path(feistate.path)

if feistate.stage >= 0:
    path.add_module("RemoveParticlesNotInLists", particleLists=feistate.plists)
    path.add_module('RootOutput')

print(path)
process(path)
