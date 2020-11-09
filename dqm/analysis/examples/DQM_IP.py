#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
import sys
argv = sys.argv


# Set the log level to show only error and fatal messages
# set_log_level(LogLevel.ERROR)
b2.set_log_level(b2.LogLevel.INFO)
# set_log_level(LogLevel.DEBUG)
# set_debug_level(1000)

# Create main path
main = b2.create_path()

# Modules
inroot = b2.register_module('DQMHistAnalysisInputRootFile')
inroot.param('InputRootFile', sys.argv[1])
main.add_module(inroot)

# input = register_module('DQMHistAnalysisInput')
# input.param('HistMemoryPath', argv[1])
# input.param('AutoCanvas', False)
# input.param('AutoCanvasFolders', [])
# main.add_module(input)

ip_x = b2.register_module('DQMHistAnalysisIP')
ip_x.param("HistoName", "IPMonitoring/Y4S_Vertex.X")
ip_x.param('PVName', 'DQM:Beam:IP:X:')
main.add_module(ip_x)

ip_y = b2.register_module('DQMHistAnalysisIP')
ip_y.param("HistoName", "IPMonitoring/Y4S_Vertex.Y")
ip_y.param('PVName', 'DQM:Beam:IP:Y:')
main.add_module(ip_y)

ip_z = b2.register_module('DQMHistAnalysisIP')
ip_z.param("HistoName", "IPMonitoring/Y4S_Vertex.Z")
ip_z.param('PVName', 'DQM:Beam:IP:Z:')
main.add_module(ip_z)


epicsarray = b2.register_module('DQMHistOutputToEPICS')
epicsarray.param('HistoList', [
    ['IPMonitoring/Y4S_Vertex.X', 'DQM:Beam:IP:X:Proj', 'DQM:Beam:IP:X:Proj:Last'],
    ['IPMonitoring/Y4S_Vertex.Y', 'DQM:Beam:IP:Y:Proj', 'DQM:Beam:IP:Y:Proj:Last'],
    ['IPMonitoring/Y4S_Vertex.Z', 'DQM:Beam:IP:Z:Proj', 'DQM:Beam:IP:Z:Proj:Last'],
])

main.add_module(epicsarray)

# outroot = register_module('DQMHistAnalysisOutputFile')
# outroot.param('SaveHistos', False)  # don't save histograms
# outroot.param('SaveCanvases', True)  # save canvases
# main.add_module(outroot)

output = b2.register_module('DQMHistAnalysisOutputRelayMsg')
# check that port fit your root canvas server
output.param('Port', int(argv[2]))
main.add_module(output)


b2.print_path(main)
# Process all events
b2.process(main)
