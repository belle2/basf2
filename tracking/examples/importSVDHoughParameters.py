#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Import SVDHoughParameters payloads
from ROOT import Belle2  # noqa: make the Belle2 namespace available
from ROOT.Belle2 import Database, IntervalOfValidity, SVDHoughParameters

# initial values, to be optimized #
parameters = SVDHoughParameters()
parameters.setFinalOverlapResolverMinActivityState(0.7)
parameters.setMaxRelations(100)
parameters.setMinQualitiyIndicatorSize3(0.5)
parameters.setMinQualitiyIndicatorSize4(0.5)
parameters.setMinQualitiyIndicatorSize5(0.5)
parameters.setRefinerOverlapResolverMinActivityState(0.7)

database = Database.Instance()

iov = IntervalOfValidity(0, 0, -1, -1)  # 1st exp, 1st run, final exp, final run
database.storeData('SVDHoughParameters', parameters, iov)
