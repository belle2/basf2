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
from ROOT.Belle2 import Database, IntervalOfValidity, WireHitFilterSettings

# Use combined
settings = WireHitFilterSettings()
settings.setFilterName('combined')
settings.setMVASwitchSuperLayer(2)
database = Database.Instance()

iov = IntervalOfValidity(0, 0, -1, -1)  # 1st exp, 1st run, final exp, final run
database.storeData('WireHitFilterSettings', settings, iov)
