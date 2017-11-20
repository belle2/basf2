#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *


def add_cosmic_dqm(path, components=None):
    """
    This function adds DQMs for CRT
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
        bklmdqm = register_module("bklmDQM")
        path.add_module(bklmdqm)
    # ECLTRG
    if components is None or 'TRG' in components:
        trgecldqm = register_module('TRGECLDQM')
        path.add_module(trgecldqm)
