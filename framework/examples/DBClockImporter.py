#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Import clocks payloads
"""

from ROOT import Belle2

clocks = Belle2.HardwareClockSettings()
clocks.setAcceleratorRF(508.887)
clocks.setClockPrescale(Belle2.Const.EDetector.TOP, "sampling", 6)
clocks.setClockPrescale(Belle2.Const.EDetector.SVD, "sampling", 4)
clocks.setClockPrescale(Belle2.Const.EDetector.ECL, "sampling", 3 * 24)

iov = Belle2.IntervalOfValidity.always()
db = Belle2.Database.Instance()
db.storeData("HardwareClockSettings", clocks, iov)
