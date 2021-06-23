#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Importer of the SVD Reconstruction Configuration
"""
import basf2
from ROOT import Belle2
import datetime

now = datetime.datetime.now()

# cluster reconstruction algorithms:
algorithmFor_timeRecoWith6Samples = "CoG3"
algorithmFor_timeRecoWith3Samples = "CoG3"
algorithmFor_positionRecoWith6Samples = "CoGOnly"
algorithmFor_positionRecoWith3Samples = "CoGOnly"
algorithmFor_chargeRecoWith6Samples = "MaxSample"
algorithmFor_chargeRecoWith3Samples = "MaxSample"
# strip reconstruction algorithms:
algorithmFor_stripTimeRecoWith6Samples = "dontdo"
algorithmFor_stripTimeRecoWith3Samples = "dontdo"
algorithmFor_stripChargeRecoWith6Samples = "MaxSample"
algorithmFor_stripChargeRecoWith3Samples = "MaxSample"


class recoConfigurationImporter(basf2.Module):
    ''' importer of the SVD Reconstruction Configuration'''

    def beginRun(self):
        '''begin run'''

        iov = Belle2.IntervalOfValidity.always()

        payload = Belle2.SVDRecoConfiguration("SVDRecoConfiguration_default_3=6_" +
                                              str(now.isoformat()) + "_INFO:" +
                                              "_" + str(algorithmFor_timeRecoWith6Samples) + "Time" +
                                              "_" + str(algorithmFor_chargeRecoWith6Samples) + "Charge" +
                                              "_" + str(algorithmFor_positionRecoWith6Samples) + "Position")

        # cluster time
        payload.setTimeRecoWith6Samples(algorithmFor_timeRecoWith6Samples)
        payload.setTimeRecoWith3Samples(algorithmFor_timeRecoWith3Samples)
        # cluster position
        payload.setPositionRecoWith6Samples(algorithmFor_positionRecoWith6Samples)
        payload.setPositionRecoWith3Samples(algorithmFor_positionRecoWith3Samples)
        # cluster charge
        payload.setChargeRecoWith6Samples(algorithmFor_chargeRecoWith6Samples)
        payload.setChargeRecoWith3Samples(algorithmFor_chargeRecoWith3Samples)
        # strip time
        payload.setStripTimeRecoWith6Samples(algorithmFor_stripTimeRecoWith6Samples)
        payload.setStripTimeRecoWith3Samples(algorithmFor_stripTimeRecoWith3Samples)
        # strip charge
        payload.setStripChargeRecoWith6Samples(algorithmFor_stripChargeRecoWith6Samples)
        payload.setStripChargeRecoWith3Samples(algorithmFor_stripChargeRecoWith3Samples)

        Belle2.Database.Instance().storeData(Belle2.SVDRecoConfiguration.name, payload, iov)


main = basf2.create_path()

# Event info setter - execute single event
eventinfosetter = basf2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module(recoConfigurationImporter())

# Process events
basf2.process(main)
