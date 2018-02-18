#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import math
import os
from basf2 import *

import ROOT
from ROOT import Belle2


class EventSelectorVXD(Module):

    """
    Python module to select simulated cosmic events
    (which pass VXD volume).
    """

    def __init__(self):
        """ init """
        super(EventSelectorVXD, self).__init__()

    def isOK(self, SimHit):
        """ Events with empty VXD (SVD or PXD) SimHits are removed."""
        nSimHit = SimHit.getEntries()
        if nSimHit == 0:
            return False

        return True

    def event(self):
        """ Return True if event is fine, False otherwise """
        SVDSimHitArray = Belle2.PyStoreArray('SVDSimHits')
        PXDSimHitArray = Belle2.PyStoreArray('PXDSimHits')
        if self.isOK(SVDSimHitArray):
            someOK = True
        elif self.isOK(PXDSimHitArray):
            someOK = True
        else:
            someOK = False

        if someOK:
            EventMetaData = Belle2.PyStoreObj('EventMetaData')
            event = EventMetaData.getEvent()
            print('Event', event, 'has PXD or SVD SimHits. It will be stored')

        super(EventSelectorVXD, self).return_value(someOK)
