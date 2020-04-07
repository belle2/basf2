# !/usr/bin/env python
# -*- coding: utf-8 -*-

##
# contributors: G. Casarosa, J. Bennett
##

######
# util to skim out the event triggered with random trigger
#
# in order to use this function:
# import it:
#     from svd.skim_utils import skimOutRNDTrgModule
# and then use it:
#     skimRNDtrg = skimOutRNDTrgModule()
#     main.add_module(skimRNDtrg)
#     emptypath = create_path()
#     skimRNDtrg .if_false(emptypath)
####

import basf2
import ROOT
from ROOT import Belle2
from basf2 import *


class skimOutRNDTrgModule(basf2.Module):
    """
    returns True if the event is NOT a random triggered event
    """

    def event(self):

        rawFTSW = Belle2.PyStoreArray('RawFTSWs')

        if not rawFTSW.isValid():
            B2WARNING('No RawFTSW available - event ignored')
            self.return_value(0)

            return

        # unknown meaning of this number
        unknownInt = 0
        if rawFTSW[0].GetTRGType(unknownInt) != Belle2.TRGSummary.TTYP_RAND:
            self.return_value(1)
        else:
            self.return_value(0)


class skimSVDBurstEventsModule(basf2.Module):
    """
    returns True if the event is a Burst event (number of clusters > max number of clusters)
    use set_nMaxStrips(nMaxStrips) to set the max number of strips iof a non-burst event, default is nMaxStrips=2000
    """

    def __init__(self):
        """constructor"""

        super().__init__()
        self.nMaxStrips = 2000

    def set_nMaxStrips(user_nMaxStrips):
        "set the max strips, otherwise 2000"

        self.nMaxStrips = user_nMaxStrips

    def event(self):

        strips = Belle2.PyStoreArray('SVDShaperDigits')

        if not strips.isValid():
            B2WARNING('No SVDShaperDigits - event ignored')
            self.return_value(0)

            return

        # unknown meaning of this number
        if strips.getEntries() > self.nMaxStrips:
            self.return_value(1)
        else:
            self.return_value(0)


class skimSVDTriggerBinEventsModule(basf2.Module):
    """
    returns True if TriggerBin of the event is the selected one
    use set_tb(tb) to set the value of the selected TriggerBin (0,1,2,3)
    """

    def __init__(self):
        """constructor"""

        super().__init__()
        self.tb = 99  # no selection of TB
        """ trigger bin to select"""

    def set_tb(self, user_tb):
        "set the trigger bin, otherwise 99, i.e. no selection"

        self.tb = user_tb

    def event(self):

        if int(self.tb) == 99:
            self.return_value(1)

        eventInfo = Belle2.PyStoreObj('SVDEventInfo')

        if not eventInfo.isValid():
            B2ERROR('No SVDEventInfo - event ignored')
            self.return_value(0)

            return

        if ord(eventInfo.getModeByte().getTriggerBin()) == int(self.tb):
            self.return_value(1)
        else:
            self.return_value(0)
