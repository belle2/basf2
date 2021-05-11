#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Import EventsOfDoomParameters payloads

from ROOT.Belle2 import EventsOfDoomParameters, Database, IntervalOfValidity

parameters = EventsOfDoomParameters()
parameters.setNCDCHitsMax(6000)
parameters.setNSVDShaperDigitsMax(70000)

database = Database.Instance()

iov = IntervalOfValidity(0, 0, 0, -1)
database.storeData('EventsOfDoomParameters', parameters, iov)
iov = IntervalOfValidity(1002, 0, 1002, -1)
database.storeData('EventsOfDoomParameters', parameters, iov)
iov = IntervalOfValidity(1003, 0, 1003, -1)
database.storeData('EventsOfDoomParameters', parameters, iov)
