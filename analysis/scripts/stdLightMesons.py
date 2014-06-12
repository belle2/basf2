#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *


def stdLightMesons(path=analysis_main):

    stdLooseKS0(path)
    stdRho(path)
    stdKstar(path)


def stdLooseKS0(path=analysis_main):
    reconDecay('K_S0:loose -> pi-:loose pi+:loose', {'M': (0.4, 0.6)}, True,
               path)


def stdRho(path=analysis_main):
    reconDecay('rho0:loose -> pi-:loose pi+:loose', {'M': (0.5, 1.0)}, True,
               path)


def stdKstar(path=analysis_main):
    reconDecay('K*0:loose -> pi-:loose K+:loose', {'M': (0.8, 1.1)}, True,
               path)


