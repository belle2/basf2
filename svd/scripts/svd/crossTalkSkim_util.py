# !/usr/bin/env python
# -*- coding: utf-8 -*-

##
# contributors: James Webb
##

######
# util to skim out cross talk event in the origami sensors
#
# in order to use this function:
# import it:
#     from svd.crossTalkSkim_utils import skimCrossTalkEventsModule
# and then use it:
#     skimCrossTalk = skimCrossTalkEventsModule()
#     main.add_module(skimCrossTalk)
#     emptypath = create_path()
#     skimCrossTalk.if_true(emptypath)
####

import basf2
import ROOT
from ROOT import Belle2


class skimCrossTalkEventsModule(basf2.Module):

    def event(self):

        svdRecoDigits = Belle2.PyStoreArray('SVDRecoDigits')
        eventFoundBool = 0
        for svdRecoDigit in svdRecoDigits:
            if svdRecoDigit.isCrossTalkEvent():
                self.return_value(1)
                eventFoundBool = 1
                break
        if eventFoundBool == 0:
            self.return_value(0)
