#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Import DAFparameters payloads
from ROOT.Belle2 import Database, IntervalOfValidity, DAFparameters

# Optimisation 2023 #
parameters = DAFparameters()
parameters.setDeltaPValue(0.001)
parameters.setDeltaWeight(0.06)
parameters.setProbabilityCut(0.001)
parameters.setAnnealingScheme(2, 0.01, 5)
parameters.setMinimumIterations(1)
parameters.setMaximumIterations(9)
parameters.setMinimumIterationsForPVal(1)
parameters.setMaximumFailedHits(5)

database = Database.Instance()

iov = IntervalOfValidity(0, 0, -1, -1)  # 1st exp, 1st run, final exp, final run
database.storeData('DAFparameters', parameters, iov)
