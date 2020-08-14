#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Importer of the SVD Reconstruction Configuration
"""
import basf2
from basf2 import *
from svd import *
import ROOT
from ROOT import Belle2
from ROOT.Belle2 import SVDRecoConfiguration
import datetime
import os

now = datetime.datetime.now()

algorithmFor_timeRecoWith6Samples = "CoG6"
algorithmFor_timeRecoWith3Samples = "CoG3"
algorithmFor_positionRecoWith6Samples = ""
algorithmFor_positionRecoWith3Samples = ""
algorithmFor_chargeRecoWith6Samples = ""
algorithmFor_chargeRecoWith3Samples = ""


class recoConfigurationImporter(basf2.Module):
    ''' importer of the SVD Reconstruction Configuration'''

    def beginRun(self):
        '''begin run'''

        iov = Belle2.IntervalOfValidity.always()

        payload = Belle2.SVDRecoConfiguration("SVDRecoConfiguration_TEST_" +
                                              str(now.isoformat()) + "_INFO:_timeOnly")

        payload.setTimeRecoWith6Samples(algorithmFor_timeRecoWith6Samples)
        payload.setTimeRecoWith3Samples(algorithmFor_timeRecoWith3Samples)
#        payload.setPositionRecoWith6Samples("")
#        payload.setPositionRecoWith3Samples("")
#        payload.setChargeRecoWith6Samples("")
#        payload.setChargeRecoWith3Samples("")

        Belle2.Database.Instance().storeData(Belle2.SVDRecoConfiguration.name, payload, iov)


main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

# main.add_module("Gearbox")
# main.add_module("Geometry")

main.add_module(recoConfigurationImporter())

# Process events
process(main)
