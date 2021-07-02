#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

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
