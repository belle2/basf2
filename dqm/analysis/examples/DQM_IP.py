#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
import sys
argv = sys.argv


# Set the log level to show only error and fatal messages
# set_log_level(LogLevel.ERROR)
set_log_level(LogLevel.INFO)
# set_log_level(LogLevel.DEBUG)
# set_debug_level(1000)

# Create main path
main = create_path()

# Modules
inroot = register_module('DQMHistAnalysisInputRootFile')
inroot.param('InputRootFile', sys.argv[1])
main.add_module(inroot)

# input = register_module('DQMHistAnalysisInput')
# input.param('HistMemoryPath', argv[1])
# input.param('AutoCanvas', False)
# input.param('AutoCanvasFolders', [])
# main.add_module(input)


epicsarray = register_module('DQMHistOutputToEPICS')
epicsarray.param('HistoList', [
    ['IPMonitoring/Y4S_Vertex.X', 'DQM:Beam:IP:X:Proj', 'DQM:Beam:IP:X:Proj:Last'],
    ['IPMonitoring/Y4S_Vertex.Y', 'DQM:Beam:IP:Y:Proj:Last'],
    ['IPMonitoring/Y4S_Vertex.Z', 'DQM:Beam:IP:Z:Proj', 'DQM:Beam:IP:Z:Proj:Last'],
])

main.add_module(epicsarray)

# outroot = register_module('DQMHistAnalysisOutputFile')
# outroot.param('SaveHistos', False)  # don't save histograms
# outroot.param('SaveCanvases', True)  # save canvases
# main.add_module(outroot)

output = register_module('DQMHistAnalysisOutputRelayMsg')
# check that port fit your root canvas server
output.param('Port', int(argv[2]))
main.add_module(output)


print_path(main)
# Process all events
process(main)
