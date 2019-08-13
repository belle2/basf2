#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Write .txt file with constraint for T0's
# Sum(all T0's) = 0.

from basf2 import *
from ROOT import Belle2


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

label = Belle2.GlobalLabel()

with open('cdc-t0-constraints.txt', 'w') as file:
    file.write('Constraint 0. ! Constraint to set sum(T0 - all wires) = 0.\n')
    for layer in range(0, 56):
        for wire in range(0, wires_in_layer[layer]):
            label.construct(Belle2.CDCTimeZeros.getGlobalUniqueID(), Belle2.WireID(layer, wire).getEWire(), 0)
            file.write(str(label.label()) + ' 1.\n')
