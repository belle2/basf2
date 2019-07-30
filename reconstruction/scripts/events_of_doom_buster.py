#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
from ROOT import Belle2


class EventsOfDoomBuster(basf2.Module):
    """
    Module that flags an event destined for doom at reconstruction,
    based on the size of selected hits/digits containers after the unpacking.

    This is meant to be registered in the path *after* the unpacking, but *before* reconstruction.
    """

    def __init__(self, nCDCHitsMax=int(1e9), nSVDShaperDigitsMax=int(1e9)):
        """
        Module constructor.

        Args:
            nCDCHitsMax (Optional[int]): the max number of CDC hits
                for an event to be kept for reconstruction.
                By default, no events are skipped based upon this requirement.
            nSVDShaperDigitsMax (Optional[int]): the max number of SVD shaper digits
                for an event to be kept for reconstruction.
                By default, no events are skipped based upon this requirement.
        """

        super().__init__()

        self.nCDCHitsMax = nCDCHitsMax
        self.nSVDShaperDigitsMax = nSVDShaperDigitsMax

    def initialize(self):
        """
        Module initializer.
        """

        self.eventinfo = Belle2.PyStoreObj("EventMetaData")
        self.cdchits = Belle2.PyStoreArray("CDCHits")
        self.svdshaperdigits = Belle2.PyStoreArray("SVDShaperDigits")

    def event(self):
        """
        Flag each event.

        Returns:
            bool: True if event exceeds `nCDCHitsMax or nSVDShaperDigitsMax`.
                  In that case, the event should be skipped for reco.
        """

        ncdchits = len(self.cdchits)
        nsvdshaperdigits = len(self.svdshaperdigits)

        basf2.B2DEBUG(20, f"Event: {self.eventinfo.getEvent()} - nCDCHits: {ncdchits}, nSVDShaperDigits: {nsvdshaperdigits}")

        doom_cdc = ncdchits > self.nCDCHitsMax
        doom_svd = nsvdshaperdigits > self.nSVDShaperDigitsMax

        if doom_cdc:
            basf2.B2WARNING("Skip event --> Too much occupancy for reco!",
                            event=self.eventinfo.getEvent(),
                            run=self.eventinfo.getRun(),
                            exp=self.eventinfo.getExperiment(),
                            nCDCHits=ncdchits,
                            nCDCHitsMax=self.nCDCHitsMax)
        if doom_svd:
            basf2.B2WARNING("Skip event --> Too much occupancy for reco!",
                            event=self.eventinfo.getEvent(),
                            run=self.eventinfo.getRun(),
                            exp=self.eventinfo.getExperiment(),
                            nSVDShaperDigits=nsvdshaperdigits,
                            nSVDShaperDigitsMax=self.nSVDShaperDigitsMax)

        self.return_value(doom_cdc or doom_svd)
