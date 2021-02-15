#!/usr/bin/env python

"""
Import the default bunch structure payload for master GT
"""

from ROOT import Belle2

bunches = Belle2.BunchStructure()

iov = Belle2.IntervalOfValidity.always()
db = Belle2.Database.Instance()
db.storeData("BunchStructure", bunches, iov)
