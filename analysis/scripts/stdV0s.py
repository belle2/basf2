#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *


# Prepare Kshorts from V0 mdst table
# the ParticleList is named "K_S0:all"
# vertex fit is performed and only
# candidates with inv. mass between
# 0.450 - 0.550 GeV are kept

def stdKshorts(path=analysis_main):
    fillParticleList('K_S0:all', '0.3 < M < 0.7', True, path)
    vertexKFit('K_S0:all', 0.0, '', '', path)
    applyCuts('K_S0:all', '0.450 < M < 0.550', path)


