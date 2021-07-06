#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Import clocks payloads
# Note: frequencies must be given in GHz (default unit in basf2)

from ROOT import Belle2

clocks = Belle2.HardwareClockSettings()
clocks.setAcceleratorRF(508.876e-3)
clocks.setClockPrescale(Belle2.Const.EDetector.TOP, "sampling", 6)
clocks.setClockPrescale(Belle2.Const.EDetector.SVD, "sampling", 4)
clocks.setClockPrescale(Belle2.Const.EDetector.ECL, "sampling", 3 * 24)
clocks.setClockPrescale(Belle2.Const.EDetector.KLM, "TDC", 0.125)

iov = Belle2.IntervalOfValidity.always()
db = Belle2.Database.Instance()
db.storeData("HardwareClockSettings", clocks, iov)
