#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Import ckf_ToPXD_FiltersName payloads

from ROOT.Belle2 import ckf_ToPXD_FiltersName, Database, IntervalOfValidity

ckfToPXDFiltersName_sensor = ckf_ToPXD_FiltersName()
ckfToPXDFiltersName_sensor.setHitFilterName("sensor")
ckfToPXDFiltersName_sensor.setSeedFilterName("sensor")

ckfToPXDFiltersName_intercept = ckf_ToPXD_FiltersName()
ckfToPXDFiltersName_intercept.setHitFilterName("intercept")
ckfToPXDFiltersName_intercept.setSeedFilterName("intercept")

database = Database.Instance()

iov = IntervalOfValidity(0, 0, 0, -1)
database.storeData('ckf_toPXD_FiltersName', ckfToPXDFiltersName_sensor, iov)
iov = IntervalOfValidity(1003, 0, 1003, -1)
database.storeData('ckf_toPXD_FiltersName', ckfToPXDFiltersName_sensor, iov)

iov = IntervalOfValidity(1004, 0, 1004, -1)
database.storeData('ckf_toPXD_FiltersName', ckfToPXDFiltersName_intercept, iov)
