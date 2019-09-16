# !/usr/bin/env python
# -*- coding: utf-8 -*-

##
# contributors: James Webb
##

######
# util to skim out cross talk event in the origami sensors
#
# requires svdCrossTalkFinderModule to have flagged events in
# SVDEventInfo
#
# in order to use this function:
# import it:
#     from svd.crossTalkSkim_utils import skimCrossTalkEventsModule
# and then use it:
#     emptypath = create_path()
#     skimCrossTalk.if_true(emptypath)
####

import basf2
import ROOT
from ROOT import Belle2


class skimCrossTalkEventsModule(basf2.Module):

    def event(self):

        svdEventInfo = Belle2.PyStoreObj('SVDEventInfo')

        if svdEventInfo.isCrossTalkEvent():

            self.return_value(1)

        else:

            self.return_value(0)
