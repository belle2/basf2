#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Import KLM time constants payloads.

from ROOT import Belle2

basf2.set_log_level(basf2.LogLevel.INFO)

timeConstants = Belle2.KLMTimeConstants()
timeConstants.setEffLightSpeed(0.5 * Belle2.Const.speedOfLight, Belle2.KLMTimeConstants.c_RPC)
timeConstants.setEffLightSpeed(0.5671 * Belle2.Const.speedOfLight, Belle2.KLMTimeConstants.c_BKLM)
timeConstants.setEffLightSpeed(0.5671 * Belle2.Const.speedOfLight, Belle2.KLMTimeConstants.c_EKLM)

db = Belle2.Database.Instance()

iov = Belle2.IntervalOfValidity(0, 0, 0, -1)
db.storeData("KLMTimeConstants", timeConstants, iov)

iov = Belle2.IntervalOfValidity(1002, 0, 1002, -1)
db.storeData("KLMTimeConstants", timeConstants, iov)

iov = Belle2.IntervalOfValidity(1003, 0, 1003, -1)
db.storeData("KLMTimeConstants", timeConstants, iov)
