#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Import KLM time cable delay payloads.

from ROOT import Belle2

timeCableDelay = Belle2.KLMTimeCableDelay()
index = Belle2.KLMChannelIndex()
index2 = Belle2.KLMChannelIndex()
while (index != index2.end()):
    channel = index.getKLMChannelNumber()
    timeCableDelay.setTimeDelay(channel, 0.0)
    index.increment()

db = Belle2.Database.Instance()

iov = Belle2.IntervalOfValidity(0, 0, 0, -1)
db.storeData("KLMTimeCableDelay", timeCableDelay, iov)

iov = Belle2.IntervalOfValidity(1002, 0, 1002, -1)
db.storeData("KLMTimeCableDelay", timeCableDelay, iov)

iov = Belle2.IntervalOfValidity(1003, 0, 1003, -1)
db.storeData("KLMTimeCableDelay", timeCableDelay, iov)
