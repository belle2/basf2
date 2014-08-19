#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *


def stdLightMesons(path=analysis_main):

    stdLooseKS0(path)
    stdRho(path)
    stdKstar(path)


def stdLooseKS0(path=analysis_main):
    reconstructDecay('K_S0:loose -> pi-:loose pi+:loose', '0.4 < M < 0.6', 1,
                     True, path)


def stdRho(path=analysis_main):
    reconstructDecay('rho0:loose -> pi-:loose pi+:loose', '0.5 < M < 1.0', 1,
                     True, path)


def stdKstar(path=analysis_main):
    reconstructDecay('K*0:loose -> pi-:loose K+:loose', '0.8 < M < 1.1', 1,
                     True, path)


