#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Import KLM scintillator firmware payloads.

import sys
import basf2
from ROOT.Belle2 import Database, IntervalOfValidity, KLMScintillatorFirmware

basf2.set_log_level(basf2.LogLevel.INFO)

mc = False
if (len(sys.argv) >= 2):
    if (sys.argv[1] == 'mc'):
        mc = True


db = Database.Instance()

if (mc):
    db.storeData(
        "KLMScintillatorFirmware", KLMScintillatorFirmware(
            KLMScintillatorFirmware.c_Run2), IntervalOfValidity(
            0, 0, 0, -1))
    db.storeData(
        "KLMScintillatorFirmware", KLMScintillatorFirmware(
            KLMScintillatorFirmware.c_Phase2), IntervalOfValidity(
            1002, 0, 1002, -1))
    db.storeData(
        "KLMScintillatorFirmware", KLMScintillatorFirmware(
            KLMScintillatorFirmware.c_Run1), IntervalOfValidity(
            1003, 0, 1003, -1))
    db.storeData(
        "KLMScintillatorFirmware", KLMScintillatorFirmware(
            KLMScintillatorFirmware.c_Run2), IntervalOfValidity(
            1004, 0, 1004, -1))

else:
    db.storeData(
        "KLMScintillatorFirmware", KLMScintillatorFirmware(
            KLMScintillatorFirmware.c_Phase2), IntervalOfValidity(
            1, 0, 3, -1))
    db.storeData(
        "KLMScintillatorFirmware", KLMScintillatorFirmware(
            KLMScintillatorFirmware.c_Run1), IntervalOfValidity(
            4, 0, 27, -1))
    db.storeData(
        "KLMScintillatorFirmware", KLMScintillatorFirmware(
            KLMScintillatorFirmware.c_Run2), IntervalOfValidity(
            28, 0, -1, -1))
