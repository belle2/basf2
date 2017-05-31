#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

numEvents = 1
nodeid = 0
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

evtgeninput = register_module('EvtGenInput')

from simulation import add_simulation
from reconstruction import add_reconstruction

rootoutput = register_module('RootOutput')

Packer = register_module('SVDPacker')
Packer.param('NodeID', nodeid)
Packer.param('svdDigitListName', 'SVDDigits')
Packer.param('rawSVDListName', 'SVDRaw')
Packer.param('xmlMapFileName', '../data/svd_mapping.xml')

unPacker = register_module('SVDUnpacker')
unPacker.param('rawSVDListName', 'SVDRaw')
unPacker.param('svdDigitListName', 'newSVDDigits')
unPacker.param('xmlMapFileName', '../data/svd_mapping.xml')


main = create_path()

main.add_module(eventinfosetter)
main.add_module(evtgeninput)
add_simulation(main)
# add_reconstruction(main)
# main.add_module(rootoutput)
main.add_module(Packer)
main.add_module(unPacker)

process(main)
