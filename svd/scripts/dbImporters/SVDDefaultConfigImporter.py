#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
SVD Default Configuration importer.
t_min = -80,
"""
import basf2
from basf2 import *
from svd import *
import ROOT
from ROOT import Belle2
from ROOT.Belle2 import SVDLocalConfigParameters
from ROOT.Belle2 import SVDGlobalConfigParameters


import os

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


class defaultSVDConfigParametersImporter(basf2.Module):

    def beginRun(self):

        iov = Belle2.IntervalOfValidity.always()

        local_payload = Belle2.SVDDetectorConfiguration.t_svdLocalConfig_payload()
        global_payload = Belle2.SVDDetectorConfiguration.t_svdGlobalConfig_payload()

        local_payload.setCalibrationTimeInRFCUnits(calibrationTimeUnits)
        local_payload.setCalibDate(calibDate)
        local_payload.setInjectedCharge(injCharge)
        global_payload.setLatency(latency)
        global_payload.setZeroSuppression(zeroSuppress)
        global_payload.setMaskFilter(maskFilter)
        global_payload.setAPVClockInRFCUnits(apvClockTimeUnits)

        Belle2.Database.Instance().storeData(Belle2.SVDDetectorConfiguration.svdLocalConfig_name, local_payload, iov)
        Belle2.Database.Instance().storeData(Belle2.SVDDetectorConfiguration.svdGlobalConfig_name, global_payload, iov)


use_database_chain()
use_local_database("localdb_defaultconfig/database.txt", "localdb_defaultconfig", invertLogging=True)

main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module("Gearbox")  # fileName="/geometry/Beast2_phase2.xml")
main.add_module("Geometry", components=['SVD'])  # , useDB = False)

main.add_module(defaultSVDConfigParametersImporter())

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)
