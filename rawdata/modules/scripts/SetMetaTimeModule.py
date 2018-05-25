#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import Module
from ROOT import Belle2
from basf2 import B2FATAL, B2ERROR, B2WARNING, B2INFO, B2DEBUG


class SetMetaTimeModule(Module):

    """
    module which set MetaTime from raw data (SVD and CDC as fallback)

    Author: bjoern.spruck@belle2.org
    """

    #: pointer to event meta data
    meta = None
    #: array of pointers to the detector raw data arrays
    raws = []

    def initialize(self):
        """initialize"""
        self.meta = Belle2.PyStoreObj('EventMetaData')
        if not self.meta:
            B2FATAL("no EventMetaData object")
            return

        self.raws = [Belle2.PyStoreArray(f"Raw{e}s") for e in "SVD CDC ECL TOP".split()]

    def calc_time(self, it):
        """Calculate the time in ns since epoch from UnixTime and Accelerator Clock count"""
        # t = it.GetTTTimeNs(0)
        return it.GetTTUtime(0) * 1000000000 + int(it.GetTTCtime(0) / 0.127216)

    def event(self):
        """event"""
        if not self.meta.isValid():
            B2FATAL("no valid EventMetaData object in this event")
            return

        t = self.meta.getTime()  # Check if it already set, t=0 means not set
        if t != 0:
            return  # No need to update

        for det in self.raws:
            if det:
                for it in det:
                    t = self.calc_time(it)
                    self.meta.setTime(t)
                    return

        B2DEBUG(1, "No Time for EventMetaData extracted as no raw SVD, CDC, ECL entries available")
