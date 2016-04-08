# !/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2


def add_cdc_trigger(path):
    """
    This function adds the CDC trigger modules to a path.
    """

    # standard CDC trigger
    trgcdc = register_module('TRGCDC')
    trgcdc_params = {
        'ConfigFile': Belle2.FileSystem.findFile("data/trg/cdc/TRGCDCConfig_0_20101111.dat"),
        'InnerTSLUTFile': Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_v2.2.coe"),
        'OuterTSLUTFile': Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_v2.2.coe"),
        'HoughFinderMappingFileMinus': Belle2.FileSystem.findFile("data/trg/cdc/HoughMappingMinus20160223.dat"),
        'HoughFinderMappingFilePlus': Belle2.FileSystem.findFile("data/trg/cdc/HoughMappingPlus20160223.dat")}
    trgcdc.param(trgcdc_params)
    path.add_module(trgcdc)
    # neurotrigger
    neuro = register_module('NeuroTrigger')
    neuro.param('filename', Belle2.FileSystem.findFile("data/trg/cdc/Neuro20160309Nonlin.root"))
    path.add_module(neuro)
