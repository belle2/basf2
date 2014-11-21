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
               'InputFileName': 'topcrt-pulser-e000002r000616-f000.dat'}
#
#               'InputFileName': 'topcrt-pedfront-e000002r000571-f000.dat'}
itopeventconverter.param(evtconvDict)

camacconverter = register_module('Camac')
camacDict = {  # leps june 2013 itop test beam
               # leps june 2013 itop test beam
               # leps june 2013 itop test beam
    'InputFilename': '/srv/itop_data/jun2013leps/20130609/data/topcrt-pulser-e000002r000616-f000.cmc',
    'CrateID': 13369927,
    'FTSWslot': 7,
    'FTSWword': 3,
    }
camacconverter.param(camacDict)

pedmodule = register_module('Pedestal')
pedestalDict = {
    'InputFileName': 'test.root',
    'Mode': 1,
    'WriteFile': 0,
    'Conditions': 1,
    }
pedmodule.param(pedestalDict)

mergemodule = register_module('WaveMerging')
timemodule = register_module('WaveTiming')

main = create_path()
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(itopeventconverter)
main.add_module(camacconverter)
main.add_module(pedmodule)
main.add_module(mergemodule)
main.add_module(timemodule)
main.add_module(output)

process(main)

