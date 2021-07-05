#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import basf2 as b2
from ROOT import Belle2

####################################################################################
# simple scripts that sets data in SVDEventInfo object, and then reads its content
####################################################################################


# register modules
eventinfosetter = b2.register_module("EventInfoSetter")
eventinfoprinter = b2.register_module("EventInfoPrinter")

# set parameters in modules
param_eventinfosetter = {'expList': [1],
                         'runList': [1],
                         'evtNumList': [1]}


class Check_info(b2.Module):
    ''' check the SVDEventInfo object'''

    def event(self):
        '''print the SVDEventInfo to string in each event'''
        eventInfo = Belle2.PyStoreObj('SVDEventInfo')
        b2.B2INFO(eventInfo.toString())


eventinfosetter.param(param_eventinfosetter)

# create path and add modules
main = b2.create_path()
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)

setInfo = b2.register_module('SVDEventInfoSetter')
# exemplary settings that overwrite default ones
# setInfo.param('runType', 1) #transparent
# setInfo.param('eventType', 1) #local mode
# setInfo.param('daqMode', 1) #3 samples
# setInfo.param('triggerBin', 3) #last bin
# setInfo.param('triggerType', 1) #ecl trigger
# setInfo.param('crossTalk', True) #cross-talk event

main.add_module(setInfo)

checkInfo = b2.register_module(Check_info())
main.add_module(checkInfo)

# process
b2.process(main)
