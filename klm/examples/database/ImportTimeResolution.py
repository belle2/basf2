#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Import KLM time resolution payloads.

import sys
from ROOT import Belle2

mc = False
if (len(sys.argv) >= 2):
    if (sys.argv[1] == 'mc'):
        mc = True

timeResolution = Belle2.KLMTimeResolution()
index = Belle2.KLMChannelIndex()
index2 = Belle2.KLMChannelIndex()
while (index != index2.end()):
    channel = index.getKLMChannelNumber()
    timeResolution.setTimeResolution(channel, 3.0)
    index.increment()

db = Belle2.Database.Instance()

if (mc):
    iov = Belle2.IntervalOfValidity(0, 0, 0, -1)
    db.storeData("KLMTimeResolution", timeResolution, iov)

    iov = Belle2.IntervalOfValidity(1002, 0, 1002, -1)
    db.storeData("KLMTimeResolution", timeResolution, iov)

    iov = Belle2.IntervalOfValidity(1003, 0, 1003, -1)
    db.storeData("KLMTimeResolution", timeResolution, iov)

else:
    iov = Belle2.IntervalOfValidity(0, 0, -1, -1)
    db.storeData("KLMTimeResolution", timeResolution, iov)
