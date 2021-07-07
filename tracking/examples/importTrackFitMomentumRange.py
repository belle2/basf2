#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Import math Library
import math

# Import TrackFitMomentumRange payloads
from ROOT.Belle2 import TrackFitMomentumRange, Database, IntervalOfValidity

parameters = TrackFitMomentumRange()
parameters.setMomentumRange(11, math.inf)
parameters.setMomentumRange(13, math.inf)
parameters.setMomentumRange(211, math.inf)
parameters.setMomentumRange(321, math.inf)
parameters.setMomentumRange(2212, math.inf)
parameters.setMomentumRange(1000010020, math.inf)

database = Database.Instance()

iov = IntervalOfValidity(0, 0, 0, -1)
database.storeData('TrackFitMomentumRange', parameters, iov)
iov = IntervalOfValidity(1002, 0, 1002, -1)
database.storeData('TrackFitMomentumRange', parameters, iov)
iov = IntervalOfValidity(1003, 0, 1003, -1)
database.storeData('TrackFitMomentumRange', parameters, iov)
