#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##############################################################################
#
# This is an example steering file to run the SVD Unpacke.
# It decodes a RawSVD object (from COPPER) and produce a list of SVDDigit
# according to the xml map of the sensors (currently missing)
#
##############################################################################

from basf2 import *

numEvents = 1

# show warnings during processing
set_log_level(LogLevel.DEBUG)

# Register modules

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = register_module('EventInfoPrinter')
gearbox = register_module('Gearbox')

geometry = register_module('Geometry')
geometry.param('Components', ['MagneticField', 'PXD', 'SVD'])

svdUnpacker = register_module('SVDUnpacker')
svdUnpacker.param('rawSVDListName', 'inputList')
svdUnpacker.param('svdDigitListName', 'outputList')
svdUnpacker.param('GenerateShaperDigits', True)
# svdUnpacker.param('dataFileName','RUN19880101063811.dat')
# svdUnpacker.param('dataFileName','coded')

# ============================================================================
# Do the simulation

main = create_path()
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(svdUnpacker)

# Process events
process(main)

# Print call statistics
print(statistics)
