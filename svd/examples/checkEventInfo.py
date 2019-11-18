#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from basf2 import *
from ROOT import Belle2

####################################################################################
# simple scripts that sets data in SVDEventInfo object, and then reads its content
####################################################################################


# register modules
eventinfosetter = register_module("EventInfoSetter")
eventinfoprinter = register_module("EventInfoPrinter")

# set parameters in modules
param_eventinfosetter = {'expList': [1],
                         'runList': [1],
                         'evtNumList': [1]}


class Check_info(Module):
    def event(self):
        eventInfo = Belle2.PyStoreObj('SVDEventInfo')
        B2INFO(eventInfo.toString())


eventinfosetter.param(param_eventinfosetter)

# create path and add modules
main = create_path()
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)

setInfo = register_module('SVDEventInfoSetter')
# exemplary settings that overwrite default ones
# setInfo.param('runType', 1) #transparent
# setInfo.param('eventType', 1) #local mode
# setInfo.param('daqMode', 1) #3 samples
# setInfo.param('triggerBin', 3) #last bin
# setInfo.param('triggerType', 1) #ecl trigger
# setInfo.param('crossTalk', True) #cross-talk event

main.add_module(setInfo)

checkInfo = register_module(Check_info())
main.add_module(checkInfo)

# process
process(main)
