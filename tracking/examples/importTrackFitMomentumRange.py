#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Import TrackFitMomentumRange payloads

from ROOT.Belle2 import TrackFitMomentumRange, Database, IntervalOfValidity

parameters = TrackFitMomentumRange()
parameters.setMomentumRange(211, 100.)
parameters.setMomentumRange(321, 100.)
parameters.setMomentumRange(2212, 100.)

database = Database.Instance()

iov = IntervalOfValidity(0, 0, 0, -1)
database.storeData('TrackFitMomentumRange', parameters, iov)
iov = IntervalOfValidity(1002, 0, 1002, -1)
database.storeData('TrackFitMomentumRange', parameters, iov)
iov = IntervalOfValidity(1003, 0, 1003, -1)
database.storeData('TrackFitMomentumRange', parameters, iov)
