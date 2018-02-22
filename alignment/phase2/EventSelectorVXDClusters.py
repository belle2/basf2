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
    Python module to search PXD of SVD clusters
    (which pass VXD volume).
    """

    def __init__(self):
        """ init """
        super(EventSelectorVXDClusters, self).__init__()

    def event(self):
        """ Return True if event is fine, False otherwise """

        someOK = False

        SVDClusters = Belle2.PyStoreArray('SVDClusters')
        nSVDClusters = SVDClusters.getEntries()
        if nSVDClusters != 0:
            someOK = True

        PXDClusters = Belle2.PyStoreArray('PXDClusters')
        nPXDClusters = PXDClusters.getEntries()
        if nPXDClusters != 0:
            someOK = True

        if someOK:
            EventMetaData = Belle2.PyStoreObj('EventMetaData')
            event = EventMetaData.getEvent()
            print('Event', event, 'has PXD or SVD Clusters. It will be stored')

        super(EventSelectorVXDClusters, self).return_value(someOK)
