#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import Module
from ROOT import Belle2


class SetMetaTimeModule(Module):

    """
    module which set MetaTime from raw data (SVD and CDC as fallback)

    Author: bjoern.spruck@belle2.org
    """

    meta = None
    svd = None
    cdc = None
    ecl = None

    def initialize(self):
        self.meta = Belle2.PyStoreObj('EventMetaData')
        self.svd = Belle2.PyStoreArray('RawSVDs')
        self.cdc = Belle2.PyStoreArray('RawCDCs')
        self.ecl = Belle2.PyStoreArray('RawECLs')

    def event(self):
        if not self.meta:
            B2FATAL("no EventMetaData object")
            return

        t = self.meta.getTime()  # Check if it already set, t=0 means not set
        if t != 0:
            return  # No need to update

        if t == 0:
            if self.svd:
                # Take the first SVD packet
                for it in self.svd:
                    # t = it.GetTTTimeNs(0)
                    t = it.GetTTUtime(0) * 1000000000 + int(it.GetTTCtime(0) / 0.127216)
                    break

        if t == 0:
            if self.cdc:
                # Take the first CDC packet
                for it in self.cdc:
                    # t = it.GetTTTimeNs(0)
                    t = it.GetTTUtime(0) * 1000000000 + int(it.GetTTCtime(0) / 0.127216)
                    break

        if t == 0:
            if self.ecl:
                # Take the first CDC packet
                for it in self.ecl:
                    # t = it.GetTTTimeNs(0)
                    t = it.GetTTUtime(0) * 1000000000 + int(it.GetTTCtime(0) / 0.127216)
                    break

        if t == 0:
            # could check for other detector raws
            B2FATAL("no raw SVD, CDC, ECL")
            return

        self.meta.setTime(t)
