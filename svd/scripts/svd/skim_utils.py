# !/usr/bin/env python
# -*- coding: utf-8 -*-

##
# contributors: G. Casarosa, J. Bennett
##

######
# util to skim the events
#
# in order to use this function:
# import it:
#     from svd.skim_utils import skimOutRNDTrgModule
# and then use it:
#     skimRNDtrg = skimOutRNDTrgModule()
#     main.add_module(skimRNDtrg)
#     emptypath = create_path()
#     skimRNDtrg.if_false(emptypath)
####

from ROOT import Belle2
import basf2 as b2


class skimOutRNDTrgModule(b2.Module):
    """
    returns True if the event is NOT a random triggered event
    """

    def __init__(self):
        """constructor"""

        super().__init__()

    def event(self):
        """
        check RawFTSW to get the event type
        """
        #: StoreArray of RawFTSW
        rawFTSW = Belle2.PyStoreArray('RawFTSWs')

        if not rawFTSW.isValid():
            b2.B2WARNING('No RawFTSW available - event ignored')
            self.return_value(0)

            return

        # unknown meaning of this number
        unknownInt = 0
        if rawFTSW[0].GetTRGType(unknownInt) != Belle2.TRGSummary.TTYP_RAND:
            self.return_value(1)
        else:
            self.return_value(0)


class skimSVDBurstEventsModule(b2.Module):
    """
    returns True if the event is a Burst event (number of strips > max number of strips)
    use :code:`set_nMaxStrips(nMaxStrips)` to set the max number of strips iof a non-burst event, default is :code:`nMaxStrips=5000`
    """

    def __init__(self):
        """constructor"""

        super().__init__()

        #: max number of strips for NON-BURST events
        self.nMaxStrips = 5000

    def set_nMaxStrips(self, user_nMaxStrips):
        """set the max strips, otherwise 5000"""

        self.nMaxStrips = user_nMaxStrips

    def event(self):
        '''event'''

        strips = Belle2.PyStoreArray('SVDShaperDigits')

        if not strips.isValid():
            b2.B2WARNING('No SVDShaperDigits - event ignored')
            self.return_value(0)

            return

        if strips.getEntries() > self.nMaxStrips:
            self.return_value(1)
        else:
            self.return_value(0)


class skim6SampleEventsPyModule(b2.Module):
    """
    returns True if the event is acquired with 6 samples
    """

    def __init__(self):
        """constructor"""

        super().__init__()

    def event(self):
        '''event'''

        # take SVDEventInfo or SVDvVentInfoSim
        eventInfo = Belle2.PyStoreObj('SVDEventInfo')

        if not eventInfo.isValid():
            eventInfo = Belle2.PyStoreObjPtr('SVDEventInfoSim')

        if not eventInfo.isValid():
            b2.B2WARNING('No SVDEventInfo/SVDEventInfoSim - event ignored')
            self.return_value(0)

            return

        # check if we acquired 6-sample strips in this event
        if eventInfo.getNSamples() == 6:
            self.return_value(1)
        else:
            self.return_value(0)


class skimSVDTriggerBinEventsPyModule(b2.Module):
    """
    returns True if TriggerBin of the event is the selected one,
    use :code:`set_tb(tb)` to set the value of the selected TriggerBin (0,1,2,3)
    """

    def __init__(self):
        """constructor"""

        super().__init__()

        #: trigger bin
        self.tb = 99  # no selection of TB

    def set_tb(self, user_tb):
        '''set the trigger bin, otherwise 99, i.e. no selection'''

        self.tb = user_tb

    def event(self):
        '''event'''

        if int(self.tb) == 99:
            self.return_value(1)

        eventInfo = Belle2.PyStoreObj('SVDEventInfo')

        if not eventInfo.isValid():
            b2.B2ERROR('No SVDEventInfo - event ignored')
            self.return_value(0)

            return

        if ord(eventInfo.getModeByte().getTriggerBin()) == int(self.tb):
            self.return_value(1)
        else:
            self.return_value(0)


class skimFineTRGEventsPyModule(b2.Module):
    """
    returns True if the event has a fine trigger from TRGSummary
    """

    def __init__(self):
        """constructor"""

        super().__init__()

    def event(self):
        '''event'''

        trgQuality = Belle2.PyStoreObj('TRGSummary')

        if not trgQuality.isValid():
            b2.B2WARNING('No TRGSummary - event ignored')
            self.return_value(0)

            return

        # check trigger quality
        if trgQuality.getTimQuality() == 2:
            self.return_value(1)
        else:
            self.return_value(0)


class skimLowEventT0EventsPyModule(b2.Module):
    """
    returns True if :code:`abs(EventT0)` is smaller than a selected value
    that can be set with :code:`set_maxEventT0(evtT0)`
    """

    def __init__(self):
        """constructor"""

        super().__init__()

        #: maximum value of EventT0 allowed
        self.maxEvtT0 = 10  # ns

    def set_maxEventT0(self, user_maxEvtT0):
        '''set the maximum EventT0, otherwise 10 ns'''

        self.maxEvtT0 = user_maxEvtT0

    def event(self):
        '''event'''

        eventT0 = Belle2.PyStoreObj('EventT0')

        if not eventT0.isValid():
            b2.B2ERROR('No EventT0 - event ignored')
            self.return_value(0)

            return

        if abs(eventT0.getEventT0()) < self.maxEvtT0:
            self.return_value(1)
        else:
            self.return_value(0)
