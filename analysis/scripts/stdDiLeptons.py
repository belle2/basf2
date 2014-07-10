#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *


def stdDiLeptons(path=analysis_main):

    stdLooseJpsi2mumu(path)


def stdLooseJpsi2mumu(path=analysis_main):
    reconDecay('J/psi:mumuLoose -> mu-:loose mu+:loose', '2.8 < M < 3.7',
               True, path)
    fitVertex('J/psi:mumuLoose', 0.001, path)
