#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import shutil
from basf2 import *

from basf2 import Module

input = register_module('RootInput')

unpacker = register_module('PXDUnpacker')
# unpacker.param('DoNotStore',True);
# unpacker.param('HeaderEndianSwap', False)

histoman = register_module('HistoManager')
histoman.param('histoFileName', 'your_histo_file.root')

simpleoutput = register_module('RootOutput')
simpleoutput.param('outputFileName', 'PXDRawHit.root')
simpleoutput.param('compressionLevel', 0)

main = create_path()

main.add_module(input)
main.add_module(histoman)
main.add_module(unpacker)
main.add_module(register_module('PXDRawDQM'))
main.add_module(register_module('PXDROIDQM'))
main.add_module(register_module('Progress'))
main.add_module(simpleoutput)

process(main)
