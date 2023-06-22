#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2


class svdSpacePointSelectionFunctionImporter(basf2.Module):

    def initialize(self):
        from ROOT import Belle2

        payload = Belle2.SVDSpacePointSelectionFunction()
        payload.setFraction0Max(0.45)
        payload.setFraction1Min(0.30)
        payload.setFraction0MaxForFraction1Cut(0.10)

        iov = Belle2.IntervalOfValidity.always()
        Belle2.Database.Instance().storeData("SVDSpacePointSelectionFunction", payload, iov)


main = basf2.create_path()

# Event info setter - execute single event
eventinfosetter = basf2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module(svdSpacePointSelectionFunctionImporter())

# Process events
basf2.process(main)
