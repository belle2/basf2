#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2017

from basf2 import *
from modularAnalysis import *

path = create_path()
inputMdstList('default', [], path)

import fei
particles = fei.get_unittest_channels()
configuration = fei.config.FeiConfiguration(prefix='FEI_Unittest', training=True)
feistate = fei.get_path(particles, configuration)
path.add_path(feistate.path)
path.add_module('RootOutput')

print(path)
process(path)
