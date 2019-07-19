#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Default CoG Time Calibration importer.
alfa = 1 and beta = 0 for all sensors and sides
Script to Import Calibrations into a local DB
"""
import basf2
from basf2 import *
from svd import *
import ROOT
from ROOT import Belle2
from ROOT.Belle2 import SVDCoGCalibrationFunction
from ROOT.Belle2 import SVDCoGTimeCalibrations
import datetime
import os

now = datetime.datetime.now()


class defaultCoGTimeCalibrationImporter(basf2.Module):

    def beginRun(self):

        iov = Belle2.IntervalOfValidity.always()

        timeCal = SVDCoGCalibrationFunction()
        timeCal.set_bias(0., 0., 0., 0.)
        timeCal.set_scale(1., 1., 1., 1.)

        payload = Belle2.SVDCoGTimeCalibrations.t_payload(
            timeCal, "CoGTimeCalibrations_default_" + str(now.isoformat()) + "_INFO:_alpha=1_beta=0")

        Belle2.Database.Instance().storeData(Belle2.SVDCoGTimeCalibrations.name, payload, iov)


use_database_chain()
use_central_database("svd_onlySVDinGeoConfiguration")
use_local_database("localDB_noCoGcalibration/database.txt", "localDB_noCoGcalibration")

main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module("Gearbox")
main.add_module("Geometry", components=['SVD'], useDB=True)

main.add_module(defaultCoGTimeCalibrationImporter())

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)
