#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Import CkfToPXDFiltersSetting payloads

import ROOT  # noqa
from ROOT.Belle2 import CkfToPXDFiltersSetting, Database, IntervalOfValidity

ckfToPXDFiltersSetting_sensor = CkfToPXDFiltersSetting()
ckfToPXDFiltersSetting_sensor.setHitFilterName("sensor")
ckfToPXDFiltersSetting_sensor.setSeedFilterName("sensor")

ckfToPXDFiltersSetting_intercept = CkfToPXDFiltersSetting()
ckfToPXDFiltersSetting_intercept.setHitFilterName("intercept")
ckfToPXDFiltersSetting_intercept.setSeedFilterName("intercept")
ckfToPXDFiltersSetting_intercept.setPtThresholdTrackToHitCut(0.5)
ckfToPXDFiltersSetting_intercept.setPhiInterceptToHitCut(0.2)
ckfToPXDFiltersSetting_intercept.setEtaInterceptToHitCut(0.2)
ckfToPXDFiltersSetting_intercept.setPhiRecoTrackToHitCut(0.5)
ckfToPXDFiltersSetting_intercept.setEtaRecoTrackToHitCut(0.4)
ckfToPXDFiltersSetting_intercept.setPhiHitHitCut(0.8)
ckfToPXDFiltersSetting_intercept.setEtaHitHitCut(0.8)
ckfToPXDFiltersSetting_intercept.setPhiOverlapHitHitCut(0.15)
ckfToPXDFiltersSetting_intercept.setEtaOverlapHitHitCut(0.15)
ckfToPXDFiltersSetting_intercept.setPXDInterceptsName("PXDIntercepts")

database = Database.Instance()

payloadname = "ckf_ToPXD_FiltersSetting"
print("Storing payload under name: " + payloadname)

iov = IntervalOfValidity(0, 0, 0, -1)
database.storeData(payloadname, ckfToPXDFiltersSetting_sensor, iov)
iov = IntervalOfValidity(1003, 0, 1003, -1)
database.storeData(payloadname, ckfToPXDFiltersSetting_sensor, iov)

iov = IntervalOfValidity(1004, 0, 1004, -1)
database.storeData(payloadname, ckfToPXDFiltersSetting_intercept, iov)
