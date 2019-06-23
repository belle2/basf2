#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from basf2 import *
from ROOT import Belle2

import math

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

layers = [l for l in range(0, 56)]


def get_consts_line(layer, wire, parameter, coefficient):
    wireid = Belle2.WireID(layer, wire).getEWire()
    label = Belle2.GlobalLabel()
    label.construct(Belle2.CDCAlignment.getGlobalUniqueID(), wireid, parameter)
    return '{} {}'.format(str(label.label()), coefficient)


class WriteConstraints(Module):
    def __init__(self):
        """ init """
        super(WriteConstraints, self).__init__()
        self.consts = []

    def add(self, text):
        self.consts.append(text)

    def event(self):
        if self.consts:
            return

        for layer in layers:
            self.add('Constraint 0. ! sum of wire X (BWD) in layer {} = 0'.format(layer))
            for wire in range(0, wires_in_layer[layer]):
                self.add(get_consts_line(layer, wire, Belle2.CDCAlignment.wireBwdX, 1.))

        for layer in layers:
            self.add('Constraint 0. ! sum of wire Y (BWD) in layer {} = 0'.format(layer))
            for wire in range(0, wires_in_layer[layer]):
                self.add(get_consts_line(layer, wire, Belle2.CDCAlignment.wireBwdY, 1.))

        for layer in layers:
            self.add('Constraint 0. ! sum of wire rotations (BWD) in layer {} = 0'.format(layer))
            for wire in range(0, wires_in_layer[layer]):

                wirePhi = Belle2.TrackFindingCDC.CDCWire.getInstance(Belle2.WireID(layer, wire)).getBackwardPos3D().phi()

                self.add(get_consts_line(layer, wire, Belle2.CDCAlignment.wireBwdX, -math.sin(wirePhi)))
                self.add(get_consts_line(layer, wire, Belle2.CDCAlignment.wireBwdY, +math.cos(wirePhi)))

        for layer in layers:
            self.add('Constraint 0. ! sum of wire X (FWD) in layer {} = 0'.format(layer))
            for wire in range(0, wires_in_layer[layer]):
                self.add(get_consts_line(layer, wire, Belle2.CDCAlignment.wireFwdX, 1.))

        for layer in layers:
            self.add('Constraint 0. ! sum of wire Y (FWD) in layer {} = 0'.format(layer))
            for wire in range(0, wires_in_layer[layer]):
                self.add(get_consts_line(layer, wire, Belle2.CDCAlignment.wireFwdY, 1.))

        for layer in layers:
            self.add('Constraint 0. ! sum of wire rotations (FWD) in layer {} = 0'.format(layer))
            for wire in range(0, wires_in_layer[layer]):

                wirePhi = Belle2.TrackFindingCDC.CDCWire.getInstance(Belle2.WireID(layer, wire)).getForwardPos3D().phi()

                self.add(get_consts_line(layer, wire, Belle2.CDCAlignment.wireFwdX, -math.sin(wirePhi)))
                self.add(get_consts_line(layer, wire, Belle2.CDCAlignment.wireFwdY, +math.cos(wirePhi)))

    def terminate(self):
        with open('cdc-wire-constraints.txt', 'w') as txt:
            for line in self.consts:
                txt.write(str(line) + '\n')


main = create_path()
main.add_module('EventInfoSetter')
main.add_module('Gearbox')
main.add_module('Geometry')
main.add_module(WriteConstraints())
process(main)
