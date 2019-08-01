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

# fix wire in layer-range:
first_layer = 0
last_layer = 55


class FixWires(Module):

    def __init__(self):
        """ init """
        super(FixWires, self).__init__()
        self.consts = []

    def event(self):
        """ Return True if event should be selected, False otherwise """
        if self.consts:
            return

        self.consts.append('Parameters')
        for layer in range(first_layer, last_layer + 1):
            for wire in range(0, wires_in_layer[layer]):
                # Unique id of CDCAlignment db object
                cdcid = Belle2.CDCAlignment.getGlobalUniqueID()

                wireid = Belle2.WireID(layer, wire).getEWire()
                label = Belle2.GlobalLabel()

                label.construct(cdcid, wireid, Belle2.CDCAlignment.wireBwdX)
                self.consts.append('{} 0. -1.'.format(str(label.label())))

                label.construct(cdcid, wireid, Belle2.CDCAlignment.wireFwdX)
                self.consts.append('{} 0. -1.'.format(str(label.label())))

                label.construct(cdcid, wireid, Belle2.CDCAlignment.wireBwdY)
                self.consts.append('{} 0. -1.'.format(str(label.label())))

                label.construct(cdcid, wireid, Belle2.CDCAlignment.wireFwdY)
                self.consts.append('{} 0. -1.'.format(str(label.label())))

    def terminate(self):
        with open('fix-cdc-wires.txt', 'w') as txt:
            for line in self.consts:
                txt.write(str(line) + '\n')


main = create_path()
main.add_module('EventInfoSetter')
main.add_module('Gearbox')
main.add_module('Geometry')
main.add_module(FixWires())
process(main)
