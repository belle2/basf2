#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *


def add_ecl_trigger(path):
    """
    add ecl trigger module to path
    """
    trgeclfam = register_module("TRGECLFAM")
    trgeclfam.param('TCWaveform', 0)  # Output TC Waveform (0 : no save, 1 : save)
    path.add_module(trgeclfam)

    trgecl = register_module("TRGECL")
    # Output Clustering method(0: Use only ICN, 1: ICN + Max TC)
    trgecl.param('Clustering', 0)
    path.add_module(trgecl)
