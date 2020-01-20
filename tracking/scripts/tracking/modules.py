#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
import ROOT
from ROOT import Belle2


class RegisterEventLevelTrackingInfo(basf2.Module):
    """Small module that registers the EventLevelTrackingInfo that is used to set general tracking-related flags"""

    def __init__(self, m_eventLevelTrackingInfoName="EventLevelTrackingInfo"):
        """Contructor"""
        super(RegisterEventLevelTrackingInfo, self).__init__()
        self.set_property_flags(basf2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED)
        #: name for the StoreObject to access the event level tracking information
        self.m_eventLevelTrackingInfoName = m_eventLevelTrackingInfoName

    def initialize(self):
        """ Register the StoreObj on the DataStore"""
        #: StoreObject to access the event level tracking information
        self.m_eventLevelTrackingInfo = Belle2.PyStoreObj(Belle2.EventLevelTrackingInfo.Class(), self.m_eventLevelTrackingInfoName)
        self.m_eventLevelTrackingInfo.registerInDataStore()

    def event(self):
        """ ... and create the StoreObj """
        self.m_eventLevelTrackingInfo.create()
