#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *


def add_ecl_trigger(path):
    """
    add ecl trigger module to path
    """
    trgeclfam = register_module("TRGECLFAM")
    # Output TC Waveform (0 : no save, 1 : save)
    trgeclfam.param('TCWaveform', 0)
    # save only measured TC data(=0) or both measured and true TC data(=1)
    trgeclfam.param('FAMAnaTable', 0)
    #
    path.add_module(trgeclfam)
    #
    trgecl = register_module("TRGECL")
    # trgecl.logging.log_level = LogLevel.DEBUG

    # Output Clustering method(0: Use only ICN, 1: ICN + Max TC)
    trgecl.param('Clustering', 1)
    # The limit # of cluster in clustering logic
    trgecl.param('ClusterLimit', 6)
    # Theta ID region(low and high) of 3DBhabhaVetoInTrack
    trgecl.param('3DBhabhaVetoInTrackThetaRegion', [3, 15])
    #
    path.add_module(trgecl)
