#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
SVD Default 3-sample CoG Time Calibration importer.
alfa = 1 and beta = 0 for all sensors and sides
Script to Import Calibrations into a local DB
"""
import basf2 as b2
from ROOT import Belle2
from ROOT.Belle2 import SVDCoGCalibrationFunction
from basf2 import conditions as b2conditions
import datetime

now = datetime.datetime.now()


class defaultCoGTimeCalibrationImporter_pol1TBdep(b2.Module):
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


class defaultCoGTimeCalibrationImporter_pol3TBindep(b2.Module):
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

main = b2.create_path()

# Event info setter - execute single event
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module("Gearbox")
main.add_module("Geometry")

# main.add_module(defaultCoGTimeCalibrationImporter_pol1TBdep())
main.add_module(defaultCoGTimeCalibrationImporter_pol3TBindep())
#
# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)
