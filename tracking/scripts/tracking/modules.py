#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
import ROOT
from ROOT import Belle2


class registerEventTrackingInfo(basf2.Module):
    def __init__(self):
        super(registerEventTrackingInfo, self).__init__()

    def initialize(self):
        self.m_eventLevelTrackingInfo = Belle2.PyStoreObj(Belle2.EventLevelTrackingInfo.Class())
        self.m_eventLevelTrackingInfo.registerInDataStore()

    def event(self):
        self.m_eventLevelTrackingInfo.create()
