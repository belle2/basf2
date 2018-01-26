#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *


def add_common_dqm(path, components=None):
    """
    This function adds DQMs which are common for Cosmic runs and Collion runs
    """
    # CDC
    if components is None or 'CDC' in components:
        cdcdqm = register_module('cdcDQM7')
        path.add_module(cdcdqm)
    # ECL
    if components is None or 'ECL' in components:
        ecldqm = register_module('ECLDQM')
        path.add_module(ecldqm)
    # TOP
    if components is None or 'TOP' in components:
        topdqm = register_module('TOPDataQualityOnline')
        path.add_module(topdqm)
    # BKLM
    if components is None or 'BKLM' in components:
        bklmdqm = register_module("BKLMDQM")
        path.add_module(bklmdqm)
    # EKLM
    if components is None or 'EKLM' in components:
        eklmdqm = register_module('EKLMDQM')
        path.add_module(eklmdqm)
    # ECLTRG
    if components is None or 'TRG' in components:
        trgecldqm = register_module('TRGECLDQM')
        path.add_module(trgecldqm)
