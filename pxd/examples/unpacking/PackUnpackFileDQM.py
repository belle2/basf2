#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2

# suppress messages and warnings during processing:
b2.set_log_level(b2.LogLevel.INFO)

input = b2.register_module('RootInput')
input.param('inputFileName', 'PXDRawHit.root')

input.param('branchNames', ['EventMetaData', 'PXDDigits', 'PXDDigit'])
#     'RawPXDs',

# to run the framework the used modules need to be registered
# eventinfosetter = register_module('EventInfoSetter')
# Setting the option for all non-hepevt reader modules:
# eventinfosetter.param('evtNumList', [10])  # we want to process 100 events
# eventinfosetter.param('runList', [1])  # from run number 1
# eventinfosetter.param('expList', [1])  # and experiment number 1

histoman = b2.register_module('HistoManager')
histoman.param('histoFileName', 'your_histo_file.root')

packer = b2.register_module('PXDPacker')
# [[dhhc1, dhh1, dhh2, dhh3, dhh4, dhh5] [ ... ]]
# -1 is disable port
packer.param('dhe_to_dhc', [
    [0, 2, 4, 34, 36, 38],
    [1, 6, 8, 40, 42, 44],
    [2, 10, 12, 46, 48, 50],
    [3, 14, 16, 52, 54, 56],
    [4, 3, 5, 35, 37, 39],
    [5, 7, 9, 41, 43, 45],
    [6, 11, 13, 47, 49, 51],
    [7, 15, 17, 53, 55, 57],
])

# packer.param('dhh_to_dhhc', [
#    [    1,    1,    2,    3,    -1,    5,    ],
#    [    2,    10,    11,    12,    13,    14,    ],
#    [    3, -1, 16    ],
#    [    4    ],
#    [    5, 35    ]
# ])

unpacker = b2.register_module('PXDUnpacker')

simpleoutput = b2.register_module('RootOutput')
simpleoutput.param('outputFileName', 'Output.root')

# Load parameters
gearbox = b2.register_module('Gearbox')

# Create geometry
geometry = b2.register_module('Geometry')

# Select subdetectors to be built
# geometry.param('Components', ['PXD','SVD'])
geometry.param('components', ['PXD'])


# creating the path for the processing
main = b2.create_path()
main.add_module(input)
main.add_module(gearbox)
main.add_module(geometry)
# main.add_module(eventinfosetter)
# main.add_module(paramloader)
# main.add_module(geobuilder)
main.add_module(histoman)
main.add_module(packer)
main.add_module(unpacker)
main.add_module('PXDDAQDQM')
# main.add_module('PXDRawDQM')
# main.add_module('PXDROIDQM')
main.add_module('Progress')
main.add_module(simpleoutput)

# Process the events
b2.process(main)
# if there are less events in the input file the processing will be stopped at
# EOF.
