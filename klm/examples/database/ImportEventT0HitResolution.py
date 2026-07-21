#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Import KLM EventT0 hit resolution payloads.
# Default values use the same conservative fallbacks as KLMTimeAlgorithm
# (10 ns per hit) when no calibration data is available.

import sys
from ROOT import Belle2

mc = False
if (len(sys.argv) >= 2):
    if (sys.argv[1] == 'mc'):
        mc = True

hitResolution = Belle2.KLMEventT0HitResolution()
hitResolution.setSigmaBKLMScint(10.0, 1.0)
hitResolution.setSigmaRPC(10.0, 1.0)
hitResolution.setSigmaRPCPhi(10.0, 1.0)
hitResolution.setSigmaRPCZ(10.0, 1.0)
hitResolution.setSigmaEKLMScint(10.0, 1.0)

db = Belle2.Database.Instance()

if (mc):
    iov = Belle2.IntervalOfValidity(0, 0, 0, -1)
    db.storeData("KLMEventT0HitResolution", hitResolution, iov)

    iov = Belle2.IntervalOfValidity(1002, 0, 1002, -1)
    db.storeData("KLMEventT0HitResolution", hitResolution, iov)

    iov = Belle2.IntervalOfValidity(1003, 0, 1003, -1)
    db.storeData("KLMEventT0HitResolution", hitResolution, iov)

else:
    iov = Belle2.IntervalOfValidity(0, 0, -1, -1)
    db.storeData("KLMEventT0HitResolution", hitResolution, iov)
