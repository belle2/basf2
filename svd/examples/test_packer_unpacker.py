#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2

from simulation import add_simulation

numEvents = 10
nodeid = 0
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

evtgeninput = b2.register_module('EvtGenInput')

rootoutput = b2.register_module('RootOutput')

Packer = b2.register_module('SVDPacker')
Packer.param('NodeID', nodeid)
Packer.param('svdShaperDigitListName', 'SVDShaperDigits')
Packer.param('rawSVDListName', 'SVDRaw')

unPacker = b2.register_module('SVDUnpacker')
unPacker.param('rawSVDListName', 'SVDRaw')
unPacker.param('svdShaperDigitListName', 'newSVDShaperDigits')


main = b2.create_path()

main.add_module(eventinfosetter)
main.add_module(evtgeninput)
add_simulation(main)
# add_reconstruction(main)
# main.add_module(rootoutput)
main.add_module(Packer)
main.add_module(unPacker)

b2.process(main)
