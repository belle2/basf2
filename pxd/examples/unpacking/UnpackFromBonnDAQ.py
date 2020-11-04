#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import shutil
import basf2 as b2

from basf2 import Module

main = b2.create_path()

rawinput = b2.register_module('PXDReadRawBonnDAQ')

rawinput.param('FileName', 'map.dat')
rawinput.param('ExpNr', 0)
rawinput.param('RunNr', 14)
# rawinput.param('RunNr', 182)

main.add_module(rawinput)


main.add_module("PXDRawDumper")

unpacker = b2.register_module('PXDUnpacker')
unpacker.param('FormatBonnDAQ', True)

main.add_module(unpacker)

histoman = b2.register_module('HistoManager')
histoman.param('histoFileName', 'map_HISTO.root')

# main.add_module(histoman)

# main.add_module(register_module('PXDRawDQM'))
# main.add_module(register_module('PXDROIDQM'))
# main.add_module(register_module('Progress'))

simpleoutput = b2.register_module('RootOutput')
simpleoutput.param('outputFileName', 'map_RAW.root')
simpleoutput.param('compressionLevel', 0)

main.add_module(simpleoutput)

b2.process(main)
