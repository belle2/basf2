#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2

payload = Belle2.CDCTimeZeros()

wires_in_layer = [
    160, 160, 160, 160, 160, 160, 160, 160,
    160, 160, 160, 160, 160, 160,
    192, 192, 192, 192, 192, 192,
    224, 224, 224, 224, 224, 224,
    256, 256, 256, 256, 256, 256,
    288, 288, 288, 288, 288, 288,
    320, 320, 320, 320, 320, 320,
    352, 352, 352, 352, 352, 352,
    384, 384, 384, 384, 384, 384]

for layer in range(0, 56):
    for wire in range(0, wires_in_layer[layer]):
        wireid = Belle2.WireID(layer, wire).getEWire()

        deltaT0 = 0  # T0 miscalibration in ns

        # only miscalibrate each second wire
        if wire % 2 == 1:
            deltaT0 = 2  # ns

        # We have to use global param access, because TimeZeros do
        # not have a real setter
        payload.setT0(layer, wire, 4900 + deltaT0)

iov = Belle2.IntervalOfValidity(0, 0, -1, -1)
Belle2.Database.Instance().storeData('CDCTimeZeros', payload, iov)
