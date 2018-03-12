#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# to run the framework the used modules need to be registered
eventinfosetter = register_module('EventInfoSetter')
# Setting the option for all non-hepevt reader modules:
eventinfosetter.param('evtNumList', [30])  # we want to process nr defined error events
eventinfosetter.param('runList', [1])  # from run number 1
eventinfosetter.param('expList', [1])  # and experiment number 1

histoman = register_module('HistoManager')
histoman.param('histoFileName', 'your_histo_file.root')

packer = register_module('PXDPackerErr')
# [[dhhc1, dhh1, dhh2, dhh3, dhh4, dhh5] [ ... ]]
# -1 is disable port
packer.param('dhe_to_dhc', [
    [0, 2]
])
# packer.param('dhe_to_dhc', [
# [0,  2,  4, 34, 36, 38],
# [1,  6,  8, 40, 42, 44],
# [2, 10, 12, 46, 48, 50],
# [3, 14, 16, 52, 54, 56],
# [4,  3,  5, 35, 37, 39],
# [5,  7,  9, 41, 43, 45],
# [6, 11, 13, 47, 49, 51],
# [7, 15, 17, 53, 55, 57],
# ])

unpacker = register_module('PXDUnpacker')

packercheck = register_module('PXDPackerErr')
packercheck.param('dhe_to_dhc', [
    [0, 2]
])

simpleoutput = register_module('RootOutput')
simpleoutput.param('outputFileName', 'Output.root')

# Load parameters
gearbox = register_module('Gearbox')

# Create geometry
geometry = register_module('Geometry')

# Select subdetectors to be built
# geometry.param('Components', ['PXD','SVD'])
geometry.param('components', ['PXD'])

# creating the path for the processing
main = create_path()
main.add_module(eventinfosetter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(histoman)
main.add_module(packer)
main.add_module(unpacker)
packercheck.set_log_level(LogLevel.INFO)
main.add_module(packercheck, Check=True)
main.add_module(register_module('PXDDAQDQM'))
# main.add_module(register_module('PXDRawDQM'))
# main.add_module(register_module('PXDROIDQM'))
main.add_module(register_module('Progress'))
main.add_module(simpleoutput)

# Process the events
process(main)
# if there are less events in the input file the processing will be stopped at
# EOF.
