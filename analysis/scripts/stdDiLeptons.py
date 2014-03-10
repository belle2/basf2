#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *


def stdDiLeptons(path=analysis_main):

    stdLooseJpsi2mumu(path)


def stdLooseJpsi2mumu(path=analysis_main):
    makeParticle(
        'LooseJpsi2mumu',
        443,
        ['StdLooseMu-', 'StdLooseMu+'],
        2.8,
        3.7,
        True,
        path,
        )
    fitVertex('LooseJpsi2mumu', 0.001, path)


