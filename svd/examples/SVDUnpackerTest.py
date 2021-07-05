#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##############################################################################
#
# This is an example steering file to run the SVD Unpacke.
# It decodes a RawSVD object (from COPPER) and produce a list of SVDDigit
# according to the xml map of the sensors (currently missing)
#
##############################################################################

import basf2 as b2

numEvents = 1

# show warnings during processing
b2.set_log_level(b2.LogLevel.DEBUG)

# Register modules

eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = b2.register_module('EventInfoPrinter')
gearbox = b2.register_module('Gearbox')

geometry = b2.register_module('Geometry')
geometry.param('components', ['MagneticField', 'PXD', 'SVD'])

svdUnpacker = b2.register_module('SVDUnpacker')
svdUnpacker.param('rawSVDListName', 'inputList')
svdUnpacker.param('svdShaperDigitListName', 'outputList')
# svdUnpacker.param('dataFileName','RUN19880101063811.dat')
# svdUnpacker.param('dataFileName','coded')

# ============================================================================
# Do the simulation

main = b2.create_path()
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(svdUnpacker)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
