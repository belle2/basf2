#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *


def stdLightMesons(path=main):

    stdLooseKS0(path)
    stdRho(path)
    stdKstar(path)


def stdLooseKS0(path=main):
    makeParticle(
        'StdLooseKS0',
        310,
        ['StdLoosePi-', 'StdLoosePi+'],
        0.4,
        0.6,
        True,
        path,
        )


def stdRho(path=main):
    makeParticle(
        'StdRho',
        113,
        ['StdPi-', 'StdPi+'],
        0.5,
        1.0,
        True,
        path,
        )


def stdKstar(path=main):
    makeParticle(
        'StdKstar',
        323,
        ['StdPi-', 'StdK+'],
        0.8,
        1.1,
        True,
        path,
        )


