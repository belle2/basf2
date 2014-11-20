#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [900000001])
eventinfosetter.param('evtNumList', [1000])
eventinfoprinter = register_module('EventInfoPrinter')

output = register_module('RootOutput')

# register topcaf modules
itopeventconverter = register_module('iTopRawConverter')
evtconvDict = {'InputDirectory': '/srv/itop_data/jun2013leps/20130609/data/',
               'InputFileName': 'topcrt-pedfront-e000002r000571-f000.dat'}
itopeventconverter.param(evtconvDict)

pedmodule = register_module('Pedestal')
pedestalDict = {
    'InputFileName': 'test.root',
    'Mode': 1,
    'WriteFile': 0,
    'Conditions': 1,
    }
pedmodule.param(pedestalDict)

mergemodule = register_module('WaveMerging')

main = create_path()
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(itopeventconverter)
main.add_module(pedmodule)
main.add_module(mergemodule)
main.add_module(output)

process(main)

