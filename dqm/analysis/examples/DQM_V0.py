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


# v0a = register_module('DQMHistAnalysisV0')
# v0a.param('OverlayPath', Belle2.FileSystem.findFile('/dqm/analysis/CAD'))
# main.add_module(v0a)

epicsarray = b2.register_module('DQMHistOutputToEPICS')
epicsarray.param('HistoList', [
    ['V0Objects/xvsz', 'DQM:Beam:V0:XZ', 'DQM:Beam:V0:XZ:Last'],
    ['V0Objects/xvsy[0]', 'DQM:Beam:V0:XY0', 'DQM:Beam:V0:XY0:Last'],
    ['V0Objects/xvsy[1]', 'DQM:Beam:V0:XY1', 'DQM:Beam:V0:XY1:Last'],
    ['V0Objects/xvsy[2]', 'DQM:Beam:V0:XY2', 'DQM:Beam:V0:XY2:Last'],
    ['V0Objects/xvsy[3]', 'DQM:Beam:V0:XY3', 'DQM:Beam:V0:XY3:Last'],
    ['V0Objects/xvsy[4]', 'DQM:Beam:V0:XY4', 'DQM:Beam:V0:XY4:Last'],
    ['V0Objects/xvsy[5]', 'DQM:Beam:V0:XY5', 'DQM:Beam:V0:XY5:Last'],
    ['V0Objects/xvsy[6]', 'DQM:Beam:V0:XY6', 'DQM:Beam:V0:XY6:Last'],
    ['V0Objects/xvsy[7]', 'DQM:Beam:V0:XY7', 'DQM:Beam:V0:XY7:Last'],
    ['V0Objects/xvsy[8]', 'DQM:Beam:V0:XY8', 'DQM:Beam:V0:XY8:Last'],
    ['V0Objects/xvsy[9]', 'DQM:Beam:V0:XY9', 'DQM:Beam:V0:XY9:Last'],
    ['V0Objects/xvsy[10]', 'DQM:Beam:V0:XY10', 'DQM:Beam:V0:XY10:Last'],
    ['V0Objects/xvsy[11]', 'DQM:Beam:V0:XY11', 'DQM:Beam:V0:XY11:Last'],
    ['V0Objects/xvsy[12]', 'DQM:Beam:V0:XY12', 'DQM:Beam:V0:XY12:Last'],
    ['V0Objects/xvsy[13]', 'DQM:Beam:V0:XY13', 'DQM:Beam:V0:XY13:Last'],
    ['V0Objects/xvsy[14]', 'DQM:Beam:V0:XY14', 'DQM:Beam:V0:XY14:Last'],
    ['V0Objects/xvsy[15]', 'DQM:Beam:V0:XY15', 'DQM:Beam:V0:XY15:Last'],
    ['V0Objects/xvsy[16]', 'DQM:Beam:V0:XY16', 'DQM:Beam:V0:XY16:Last'],
    ['V0Objects/xvsy[17]', 'DQM:Beam:V0:XY17', 'DQM:Beam:V0:XY17:Last'],
    ['V0Objects/xvsy[18]', 'DQM:Beam:V0:XY18', 'DQM:Beam:V0:XY18:Last'],
    ['V0Objects/xvsy[19]', 'DQM:Beam:V0:XY19', 'DQM:Beam:V0:XY19:Last'],
    ['V0Objects/xvsy[20]', 'DQM:Beam:V0:XY20', 'DQM:Beam:V0:XY20:Last'],
    ['V0Objects/xvsy[21]', 'DQM:Beam:V0:XY21', 'DQM:Beam:V0:XY21:Last'],
    ['V0Objects/xvsy[22]', 'DQM:Beam:V0:XY22', 'DQM:Beam:V0:XY22:Last'],
    ['V0Objects/xvsy[23]', 'DQM:Beam:V0:XY23', 'DQM:Beam:V0:XY23:Last'],
    ['V0Objects/xvsy[24]', 'DQM:Beam:V0:XY24', 'DQM:Beam:V0:XY24:Last'],
    ['V0Objects/xvsy[25]', 'DQM:Beam:V0:XY25', 'DQM:Beam:V0:XY25:Last'],
    ['V0Objects/xvsy[26]', 'DQM:Beam:V0:XY26', 'DQM:Beam:V0:XY26:Last'],
    ['V0Objects/xvsy[27]', 'DQM:Beam:V0:XY27', 'DQM:Beam:V0:XY27:Last'],
    ['V0Objects/xvsy[28]', 'DQM:Beam:V0:XY28', 'DQM:Beam:V0:XY28:Last'],
    ['V0Objects/xvsy[29]', 'DQM:Beam:V0:XY29', 'DQM:Beam:V0:XY29:Last'],
    ['V0Objects/xvsy[30]', 'DQM:Beam:V0:XY30', 'DQM:Beam:V0:XY30:Last'],
    ['V0Objects/xvsy[31]', 'DQM:Beam:V0:XY31', 'DQM:Beam:V0:XY31:Last'],
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
