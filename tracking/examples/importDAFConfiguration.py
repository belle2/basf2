#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Import DAFparameters payloads
from ROOT import Belle2  # noqa: make the Belle2 namespace available
from ROOT.Belle2 import Database, IntervalOfValidity, DAFParameters, DAFConfiguration

# Optimisation 2023 #
parameters = DAFParameters()
parameters.setDeltaPValue(1)
parameters.setDeltaWeight(0.001)
parameters.setProbabilityCut(0.001)
parameters.setAnnealingScheme(100, 0.1, 5)
parameters.setMinimumIterations(5)
parameters.setMaximumIterations(9)
parameters.setMinimumIterationsForPVal(5)
parameters.setMaximumFailedHits(5)
parameters.setMinimumPValue(0.)

configuration = DAFConfiguration()
parameters.setDAFParameters(DAFConfiguration.c_Default, parameters)
parameters.setDAFParameters(DAFConfiguration.c_Cosmics, parameters)
parameters.setDAFParameters(DAFConfiguration.c_CDConly, parameters)

database = Database.Instance()

iov = IntervalOfValidity(0, 0, -1, -1)  # 1st exp, 1st run, final exp, final run
database.storeData('DAFConfiguration', parameters, iov)
