#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Import KLMTriggerParameters payloads
# Usage: basf2 ImportKLMTriggerParameters.py [mc]

import sys
from ROOT.Belle2 import KLMTriggerParameters, Database, IntervalOfValidity

mc = False
if (len(sys.argv) >= 2):
    if (sys.argv[1] == 'mc'):
        mc = True

parameters = KLMTriggerParameters()
parameters.setWhichLayers('0:16')

database = Database.Instance()

if mc:
    parameters.setNLayers(4)
    iov = IntervalOfValidity(0, 0, 0, -1)
    database.storeData('KLMTriggerParameters', parameters, iov)
    iov = IntervalOfValidity(1002, 0, 1002, -1)
    database.storeData('KLMTriggerParameters', parameters, iov)
    iov = IntervalOfValidity(1003, 0, 1003, -1)
    database.storeData('KLMTriggerParameters', parameters, iov)
else:
    parameters.setNLayers(7)
    iov = IntervalOfValidity(1, 0, 16, 1131)
    database.storeData('KLMTriggerParameters', parameters, iov)
    parameters.setNLayers(4)
    iov = IntervalOfValidity(16, 1132, 16, 1132)
    database.storeData('KLMTriggerParameters', parameters, iov)
    parameters.setNLayers(7)
    iov = IntervalOfValidity(16, 1133, 17, 169)
    database.storeData('KLMTriggerParameters', parameters, iov)
    parameters.setNLayers(4)
    iov = IntervalOfValidity(17, 170, -1, -1)
    database.storeData('KLMTriggerParameters', parameters, iov)
