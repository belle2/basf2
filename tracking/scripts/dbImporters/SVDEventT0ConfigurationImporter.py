#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Importer of the SVD EventT0 Configuration
"""
import basf2
from ROOT import Belle2
import datetime

now = datetime.datetime.now()

selectTracksFromIP = True
minimumPtSelection = 0.25
absPzSelection = 0.0
absD0Selection = 0.5
absZ0Selection = 2


class svdEventT0ConfigurationImporter(basf2.Module):
    ''' Importer of the SVD EventT0 Configuration'''

    def beginRun(self):
        '''begin run'''

        iov = Belle2.IntervalOfValidity.always()

        uniqueID = f"SVDEventT0Configuration_default_{now.isoformat()}_INFO:min-pt={minimumPtSelection}_pz={absPzSelection}"

        svdEventT0ConfigurationSpecificString = ""
        if selectTracksFromIP:
            svdEventT0ConfigurationSpecificString = f"_selectTracksFromIP_d0={absD0Selection}_z0={absZ0Selection}"

        uniqueID += svdEventT0ConfigurationSpecificString

        payload = Belle2.SVDEventT0Configuration(uniqueID)

        payload.setSelectTracksFromIP(selectTracksFromIP)
        payload.setMinimumPtSelection(minimumPtSelection)
        payload.setAbsPzSelection(absPzSelection)
        payload.setAbsD0Selection(absD0Selection)
        payload.setAbsZ0Selection(absZ0Selection)

        Belle2.Database.Instance().storeData(payload.get_name(), payload, iov)


main = basf2.create_path()

# Event info setter - execute single event
eventinfosetter = basf2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module(svdEventT0ConfigurationImporter())

# Process events
basf2.process(main)
