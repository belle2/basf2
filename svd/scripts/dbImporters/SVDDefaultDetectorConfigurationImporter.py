#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
SVD Default Detecotr Configuration importer.
"""

import basf2 as b2
from ROOT import Belle2
from basf2 import conditions as b2conditions
import datetime

now = datetime.datetime.now()

# default values
# local config
calibrationTimeUnits = '2'  # in RFC units, it is a string as the xml field where it is written
calibDate = '20190314'
injCharge = 22500

# global config
latency = 158
maskFilter = 7
zeroSuppress = 3
apvClockTimeUnits = '16'  # in RFC units, it is a string as the xml field where it is written
hv = 50
relativeShift = 0
nrFrames = 6


class defaultSVDConfigParametersImporter(b2.Module):
    '''default importer for the detector configuration'''

    def beginRun(self):
        '''begin run'''

        iov = Belle2.IntervalOfValidity.always()

        local_payload = Belle2.SVDDetectorConfiguration.t_svdLocalConfig_payload(
            "LocalConfiguration_default_" + str(now.isoformat()) +
            "_INFO:_injCharge=" + str(injCharge) + "_calTimeUnits=" + str(calibrationTimeUnits))

        global_payload = Belle2.SVDDetectorConfiguration.t_svdGlobalConfig_payload(
            "GlobalConfiguration_default_" + str(now.isoformat()) +
            "_INFO:_latency=" + str(latency) +
            "_maskFilter=" + str(maskFilter) + "_ZS=" + str(zeroSuppress) +
            "_apvClkUnits=" + str(apvClockTimeUnits))

        local_payload.setCalibrationTimeInRFCUnits(calibrationTimeUnits)
        local_payload.setCalibDate(calibDate)
        local_payload.setInjectedCharge(injCharge)
        global_payload.setLatency(latency)
        global_payload.setZeroSuppression(zeroSuppress)
        global_payload.setMaskFilter(maskFilter)
        global_payload.setAPVClockInRFCUnits(apvClockTimeUnits)
        global_payload.setHV(hv)
        global_payload.setRelativeTimeShift(relativeShift)
        global_payload.setNrFrames(nrFrames)

        Belle2.Database.Instance().storeData(Belle2.SVDDetectorConfiguration.svdLocalConfig_name, local_payload, iov)
        Belle2.Database.Instance().storeData(Belle2.SVDDetectorConfiguration.svdGlobalConfig_name, global_payload, iov)


b2conditions.prepend_globaltag("svd_onlySVDinGeoConfiguration")

main = b2.create_path()

# Event info setter - execute single event
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module("Gearbox")
main.add_module("Geometry")

main.add_module(defaultSVDConfigParametersImporter())

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)
