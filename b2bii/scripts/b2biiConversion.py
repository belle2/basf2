#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import analysis_main
import os


def convertBelleMdstToBelleIIMdst(inputBelleMDSTFile, applyHadronBJSkim=True, path=analysis_main):
    """
    Loads Belle MDST file and converts in each event the Belle MDST dataobjects to Belle II MDST
    data objects and loads them to the StoreArray.
    """

    input = register_module('B2BIIMdstInput')
    input.param('inputFileName', inputBelleMDSTFile)
    # input.logging.set_log_level(LogLevel.DEBUG)
    # input.logging.set_info(LogLevel.DEBUG, LogInfo.LEVEL | LogInfo.MESSAGE)
    analysis_main.add_module(input)

    # Fix MSDT Module
    fix = register_module('B2BIIFixMdst')
    # fix.logging.set_log_level(LogLevel.DEBUG)
    # fix.logging.set_info(LogLevel.DEBUG, LogInfo.LEVEL | LogInfo.MESSAGE)
    analysis_main.add_module(fix)

    if(applyHadronBJSkim):
        emptypath = create_path()
        fix.if_value('<=0', emptypath)  # discard 'bad events' marked by fixmdst

    # Convert MDST Module
    convert = register_module('B2BIIConvertMdst')
    # convert.logging.set_log_level(LogLevel.DEBUG)
    # convert.logging.set_info(LogLevel.DEBUG, LogInfo.LEVEL | LogInfo.MESSAGE)
    analysis_main.add_module(convert)
