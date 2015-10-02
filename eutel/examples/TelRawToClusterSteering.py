#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

# Set the log level to show only warning, error and, fatal messages
# otherwise there's gonna be a segfault when python exits
set_log_level(LogLevel.WARNING)

# Load parameters from xml
gearbox = register_module('Gearbox')
# VXD (no Telescopes), and the real PCMAG magnetic field
gearbox.param('fileName', 'testbeam/vxd/FullTelescopeVXDTB_v2.xml')

# Create geometry
geometry = register_module('Geometry')
# No magnetic field for this test,
geometry.param('components', ['TB'])

# data reader
dataReader = register_module('TelDataReader')
# use 4394, 313 or, 3
p_dataReader = {'inputFileName': 'put_your_path_here/run000005.raw',
                'maxNumEntries': 10000,
                'eutelPlaneNrs': [4, 3, 2, 0, 1, 5]}
dataReader.param(p_dataReader)

# Histogram manager immediately after master module
histo = register_module('HistoManager')
histo.param('histoFileName', 'DQM-VXD-histo.root')  # File to save histograms

progress = register_module('Progress')

telClusterizer = register_module('TelClusterizer')

# telescope DQM
telDQM = register_module("TelDQM")

# data writer
dataWriter = register_module('RootOutput')
dataWriter.param('outputFileName', 'temp.root')

main = create_path()
main.add_module(dataReader)
main.add_module(histo)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(telClusterizer)
main.add_module(telDQM)
main.add_module(dataWriter)

process(main)
