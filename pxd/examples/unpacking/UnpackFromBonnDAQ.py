#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import shutil
from basf2 import *

from basf2 import Module

main = create_path()

rawinput = register_module('PXDReadRawBonnDAQ')
rawinput.param('FileName', 'map.dat')

main.add_module(rawinput)


# main.add_module("PXDRawDumper")

unpacker = register_module('PXDUnpacker')
unpacker.param('FormatBonnDAQ', True)

main.add_module(unpacker)

histoman = register_module('HistoManager')
histoman.param('histoFileName', 'map_HISTO.root')

# main.add_module(histoman)

# main.add_module(register_module('PXDRawDQM'))
# main.add_module(register_module('PXDROIDQM'))
# main.add_module(register_module('Progress'))

simpleoutput = register_module('RootOutput')
simpleoutput.param('outputFileName', 'map_RAW.root')
simpleoutput.param('compressionLevel', 0)

main.add_module(simpleoutput)

process(main)
