#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Default 3-sample CoG Time Calibration importer.
alfa = 1 and beta = 0 for all sensors and sides
Script to Import Calibrations into a local DB
"""
import basf2
from basf2 import *
from svd import *
import ROOT
from ROOT import Belle2
from ROOT.Belle2 import SVDCoGCalibrationFunction
from ROOT.Belle2 import SVD3SampleCoGTimeCalibrations
from basf2 import conditions as b2conditions
import datetime
import os

now = datetime.datetime.now()


class defaultCoGTimeCalibrationImporter_pol1TBdep(basf2.Module):
    """ default importer for the first order polynomial, TB dependent
    for the 3-sample CoG calibration payload
    """

    def beginRun(self):
        """do everything here"""

        iov = Belle2.IntervalOfValidity.always()

        timeCal = SVDCoGCalibrationFunction()
        timeCal.set_bias(0., 0., 0., 0.)
        timeCal.set_scale(1., 1., 1., 1.)
        timeCal.set_current(0)

        # take the base class payload which is the same for all calibrations:
        payload = Belle2.SVDCoGTimeCalibrations.t_payload(
            timeCal, "3SampleCoGTimeCalibrations_default_" + str(now.isoformat()) + "_INFO:_1stOrderPol__alpha=1_beta=0")

        Belle2.Database.Instance().storeData(Belle2.SVD3SampleCoGTimeCalibrations.name, payload, iov)


class defaultCoGTimeCalibrationImporter_pol3TBindep(basf2.Module):
    """ default importer for the third order polynomial, TB independent
    for the 3-sample CoG calibration payload
    """

    def beginRun(self):
        """ do everything here"""

        iov = Belle2.IntervalOfValidity.always()

        timeCal = SVDCoGCalibrationFunction()
        timeCal.set_pol3parameters(0., 1., 0., 0.)
        timeCal.set_current(1)

        # take the base class payload which is the same for all calibrations:
        payload = Belle2.SVDCoGTimeCalibrations.t_payload(
            timeCal, "3SampleCoGTimeCalibrations_default_" + str(now.isoformat()) + "_INFO:_3rdOrderPol_a=0_b=1_c=0_d=0")

        Belle2.Database.Instance().storeData(Belle2.SVD3SampleCoGTimeCalibrations.name, payload, iov)


b2conditions.prepend_globaltag("svd_onlySVDinGeoConfiguration")

main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module("Gearbox")
main.add_module("Geometry")

# main.add_module(defaultCoGTimeCalibrationImporter_pol1TBdep())
main.add_module(defaultCoGTimeCalibrationImporter_pol3TBindep())
#
# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)
