# !/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2


def add_cdc_trigger(path):
    """
    This function adds the CDC trigger modules to a path.
    """

    # TSF
    path.add_module('CDCTriggerTSF',
                    InnerTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_v3.0.coe"),
                    OuterTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_v3.0.coe"))
    # 2D finder
    path.add_module('CDCTriggerHoughtracking')
    # ETF
    path.add_module('CDCTriggerETF')
    # fitters
    path.add_module('CDCTrigger2DFitter')
    path.add_module('CDCTrigger3DFitter')
    # neurotrigger
    path.add_module('NeuroTrigger',
                    filename=Belle2.FileSystem.findFile("data/trg/cdc/Neuro20160601NonlinBkg.root"))
