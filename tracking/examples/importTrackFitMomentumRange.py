#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Import TrackFitMomentumRange payloads

from ROOT.Belle2 import TrackFitMomentumRange, Database, IntervalOfValidity

parameters = TrackFitMomentumRange()
parameters.setMomentumRange(211, 100.)
parameters.setMomentumRange(321, 100.)
parameters.setMomentumRange(2212, 100.)

database = Database.Instance()

iov = IntervalOfValidity(0, 0, -1, -1)
database.storeData('TrackFitMomentumRange', parameters, iov)
