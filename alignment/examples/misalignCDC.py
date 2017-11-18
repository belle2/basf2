#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2

payload = Belle2.CDCAlignment()

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
x = 1
y = 2
phi = 6

layer = 1

for wire in range(0, wires_in_layer[layer]):
    wireid = Belle2.WireID(layer, wire).getEWire()
    payload.setGlobalParam(0.01, wireid, y)

iov = Belle2.IntervalOfValidity(0, 0, -1, -1)
Belle2.Database.Instance().storeData('CDCAlignment', payload, iov)
