#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Import HLTprefilterParameters payloads

from ROOT import Belle2  # noqa: make Belle2 namespace available
from ROOT.Belle2 import HLTprefilterParameters, Database, IntervalOfValidity

parameters = HLTprefilterParameters()

parameters.setLERtimeSinceLastInjectionMin(5000)
parameters.setLERtimeSinceLastInjectionMax(20000)
parameters.setHERtimeSinceLastInjectionMin(600)
parameters.setHERtimeSinceLastInjectionMax(20000)
parameters.setLERtimeInBeamCycleMin(1.25)
parameters.setLERtimeInBeamCycleMax(1.55)
parameters.setHERtimeInBeamCycleMin(2.2)
parameters.setHERtimeInBeamCycleMax(2.33)

parameters.setECLDigitsMax(3000)
parameters.setCDCHitsMax(2500)

parameters.setHLTprefilterMode(0)

database = Database.Instance()

iov = IntervalOfValidity(0, 0, -1, -1)
database.storeData('HLTprefilterParameters', parameters, iov)
