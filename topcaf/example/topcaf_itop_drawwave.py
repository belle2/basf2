#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

# InputDirectory = "/srv/itop_data/jun2013leps/20130609/data/"
# InputFileRoot = "topcrt-pulser-e000002r000616-f000"

InputDirectory = '/srv/itop_data/crt/'
InputFileRoot = 'topcrt-pulser-e000000r002284-f000'

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [900000001])
eventinfosetter.param('evtNumList', [1000])
eventinfoprinter = register_module('EventInfoPrinter')

output = register_module('RootOutput')

# register topcaf modules
itopeventconverter = register_module('iTopRawConverter')
evtconvDict = {'InputDirectory': InputDirectory,
               'InputFileName': InputFileRoot + '.dat'}
itopeventconverter.param(evtconvDict)

pedmodule = register_module('Pedestal')
pedestalDict = {
    'InputFileName': 'topcrt-ped-e000000r002713-f000.dat_pedestals.root',
    'Mode': 1,
    'WriteFile': 0,
    'Conditions': 0,
    }
pedmodule.param(pedestalDict)

camacconverter = register_module('Camac')
camacDict = {  #             'CrateID'            : 13369927,  # leps june 2013 itop test beam
               #             'FTSWslot'           : 7,  # leps june 2013 itop test beam
               #             'FTSWword'           : 3  # leps june 2013 itop test beam
               # 2014 itop crt
               # 2014 itop crt
               # 2014 itop crt
    'InputFilename': InputDirectory + InputFileRoot + '.cmc',
    'CrateID': 0x00cc0200,
    'FTSWslot': 6,
    'FTSWword': 5,
    }

camacconverter.param(camacDict)

mergemodule = register_module('WaveMerging')

timemodule = register_module('WaveTiming')
timeDict = {'Time2TDC': 40.0}
timemodule.param(timeDict)

sampletimemodule = register_module('SampleTimeCalibration')
sampletimeDict = {
    'InputFileName': '/home/jhall/release/test1.root',
    'Mode': 1,
    'WriteFile': 0,
    'Conditions': 0,
    }
sampletimemodule.param(sampletimeDict)

main = create_path()
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(itopeventconverter)
main.add_module(pedmodule)
main.add_module(camacconverter)
main.add_module(mergemodule)
main.add_module(timemodule)
# main.add_module(sampletimemodule)
main.add_module(output)

process(main)

