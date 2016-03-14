#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
eventinfoprinter = register_module('EventInfoPrinter')

# register topcaf modules
itopeventconverter = register_module('iTopRawConverter')
itopeventconverter.param('inputFileName',
                         '/home/schr476/belle2/basf2/release/topcaf/data/topcrt-pedfront-e000001r000035-f000.dat'
                         )
main = create_path()
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(itopeventconverter)
process(main)
