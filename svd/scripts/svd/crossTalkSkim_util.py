##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
# !/usr/bin/env python
# -*- coding: utf-8 -*-

#
# contributors: James Webb
#

#
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
#

import basf2
from ROOT import Belle2


class skimCrossTalkEventsModule(basf2.Module):
    """
    a basf2 pythin module to skim SVD xTalk events
    """

    def event(self):
        """
        check if the xTalk flag is set and return 1 in this case
        """

        svdEventInfo = Belle2.PyStoreObj('SVDEventInfo')

        if svdEventInfo.isCrossTalkEvent():

            self.return_value(1)

        else:

            self.return_value(0)
