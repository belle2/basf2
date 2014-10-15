#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10])
eventinfoprinter = register_module('EventInfoPrinter')

# register topcaf modules
itopeventconverter = register_module('Test')

main = create_path()
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(itopeventconverter)
process(main)

