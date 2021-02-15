#!/usr/bin/env python

"""
Import clocks payloads
Note: frequencies must be given in GHz (default unit in basf2)
"""

from ROOT import Belle2

clocks = Belle2.HardwareClockSettings()
clocks.setAcceleratorRF(508.887e-3)
clocks.setClockPrescale(Belle2.Const.EDetector.TOP, "sampling", 6)
clocks.setClockPrescale(Belle2.Const.EDetector.SVD, "sampling", 4)
clocks.setClockPrescale(Belle2.Const.EDetector.ECL, "sampling", 3 * 24)

iov = Belle2.IntervalOfValidity.always()
db = Belle2.Database.Instance()
db.storeData("HardwareClockSettings", clocks, iov)
