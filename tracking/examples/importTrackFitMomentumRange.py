#!/usr/bin/env python
# -*- coding: utf-8 -*-

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
