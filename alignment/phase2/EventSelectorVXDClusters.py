#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import math
import os
from basf2 import *

import ROOT
from ROOT import Belle2


class EventSelectorVXDClusters(Module):

    """
    Python module to select cosmic events
    (which pass VXD volume).
    """

    def __init__(self):
        """ init """
        super(EventSelectorVXDClusters, self).__init__()

    def isOK(self, RecoHit):
        """ Events with empty VXD (SVD or PXD) SimHits are removed."""
        nRecoHit = RecoHit.getEntries()
        if nRecoHit == 0:
            return False

        return True

        return True

    def event(self):
        """ Return True if event is fine, False otherwise """

        SVDRecoHitArray = Belle2.PyStoreArray('SVDClusters')
        PXDRecoHitArray = Belle2.PyStoreArray('PXDClusters')

        if self.isOK(SVDRecoHitArray):
            someOK = True
        elif self.isOK(PXDRecoHitArray):
            someOK = True
        else:
            someOK = False

        if someOK:
            EventMetaData = Belle2.PyStoreObj('EventMetaData')
            event = EventMetaData.getEvent()
            print('Event', event, 'has PXD or SVD RecoHits. It will be stored')

        super(EventSelectorVXDClusters, self).return_value(someOK)
