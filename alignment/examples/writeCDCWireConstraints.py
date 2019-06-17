#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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


class WriteConstraints(Module):

    def __init__(self):
        """ init """
        super(WriteConstraints, self).__init__()
        self.consts = []

    def event(self):
        """ Return True if event should be selected, False otherwise """
        if self.consts:
            return

        for layer in range(0, 56):
            self.consts.append('Constraint 0. ! sum of wire X (BWD) in layer=0')
            for wire in range(0, wires_in_layer[layer]):
                wireid = Belle2.WireID(layer, wire).getEWire()

                label = Belle2.GlobalLabel()

                label.construct(Belle2.CDCAlignment.getGlobalUniqueID(), wireid, Belle2.CDCAlignment.wireBwdX)
                self.consts.append('{} 1.'.format(str(label.label())))

        for layer in range(0, 56):
            self.consts.append('Constraint 0. ! sum of wire Y (BWD) in layer=0')
            for wire in range(0, wires_in_layer[layer]):
                wireid = Belle2.WireID(layer, wire).getEWire()

                label = Belle2.GlobalLabel()

                label.construct(Belle2.CDCAlignment.getGlobalUniqueID(), wireid, Belle2.CDCAlignment.wireBwdY)
                self.consts.append('{} 1.'.format(str(label.label())))

        for layer in range(0, 56):
            self.consts.append('Constraint 0. ! sum of wire rotations (BWD) in layer=0')
            for wire in range(0, wires_in_layer[layer]):
                wireid = Belle2.WireID(layer, wire).getEWire()
                wid = Belle2.WireID(layer, wire)
                wh = Belle2.TrackFindingCDC.CDCWire.getInstance(wid)

                wirePhi = wh.getBackwardPos3D().phi()

                label = Belle2.GlobalLabel()
                import math
                label.construct(Belle2.CDCAlignment.getGlobalUniqueID(), wireid, Belle2.CDCAlignment.wireBwdX)
                self.consts.append('{} {}.'.format(str(label.label()), -math.sin(wirePhi)))

                label = Belle2.GlobalLabel()

                label.construct(Belle2.CDCAlignment.getGlobalUniqueID(), wireid, Belle2.CDCAlignment.wireBwdY)
                self.consts.append('{} {}.'.format(str(label.label()), math.cos(wirePhi)))

        for layer in range(0, 56):
            self.consts.append('Constraint 0. ! sum of wire X (FWD) in layer=0')
            for wire in range(0, wires_in_layer[layer]):
                wireid = Belle2.WireID(layer, wire).getEWire()

                label = Belle2.GlobalLabel()

                label.construct(Belle2.CDCAlignment.getGlobalUniqueID(), wireid, Belle2.CDCAlignment.wireFwdX)
                self.consts.append('{} 1.'.format(str(label.label())))

        for layer in range(0, 56):
            self.consts.append('Constraint 0. ! sum of wire Y (FWD) in layer=0')
            for wire in range(0, wires_in_layer[layer]):
                wireid = Belle2.WireID(layer, wire).getEWire()

                label = Belle2.GlobalLabel()

                label.construct(Belle2.CDCAlignment.getGlobalUniqueID(), wireid, Belle2.CDCAlignment.wireFwdY)
                self.consts.append('{} 1.'.format(str(label.label())))

        for layer in range(0, 56):
            self.consts.append('Constraint 0. ! sum of wire rotations (FWD) in layer=0')
            for wire in range(0, wires_in_layer[layer]):
                wireid = Belle2.WireID(layer, wire).getEWire()
                wid = Belle2.WireID(layer, wire)
                wh = Belle2.TrackFindingCDC.CDCWire.getInstance(wid)

                wirePhi = wh.getBackwardPos3D().phi()

                label = Belle2.GlobalLabel()
                import math
                label.construct(Belle2.CDCAlignment.getGlobalUniqueID(), wireid, Belle2.CDCAlignment.wireFwdX)
                self.consts.append('{} {}.'.format(str(label.label()), -math.sin(wirePhi)))

                label = Belle2.GlobalLabel()

                label.construct(Belle2.CDCAlignment.getGlobalUniqueID(), wireid, Belle2.CDCAlignment.wireFwdY)
                self.consts.append('{} {}.'.format(str(label.label()), math.cos(wirePhi)))

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
