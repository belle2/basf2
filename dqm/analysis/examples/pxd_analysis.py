#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import sys
argv = sys.argv

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.ERROR)
# set_log_level(LogLevel.INFO)
# set_log_level(LogLevel.DEBUG)

# globalTag = 'vxd_commissioning_20181030'
globalTag = 'data_reprocessing_prompt'

reset_database()
use_database_chain()
use_central_database(globalTag, LogLevel.WARNING)
use_local_database("localdb/database.txt", "localdb")


gearbox = register_module('Gearbox')
geometry = register_module('Geometry')


# Create main path
main = create_path()

# Modules

input = register_module('DQMHistAnalysisInput')
input.param('HistMemoryPath', argv[1])
input.param('AutoCanvas', False)
input.param('AutoCanvasFolders', [])
main.add_module(input)

# input = register_module('DQMHistAnalysisInputRootFile')
# input.param('InputRootFile', argv[1])
# main.add_module(input)

main.add_module(gearbox, fileName="/geometry/Belle2_earlyPhase3.xml")
main.add_module(geometry)  # , useDB=True)

pxd_charge = register_module('DQMHistAnalysisPXDCharge')
pxd_charge.param('histogramDirectoryName', 'PXDER')
main.add_module(pxd_charge)

pxd_commode = register_module('DQMHistAnalysisPXDCM')
pxd_commode.param('histogramDirectoryName', 'PXDDAQ')

main.add_module(pxd_commode)

pxd_effi = register_module('DQMHistAnalysisPXDEff')
pxd_effi.param('histogramDirectoryName', 'PXDEFF')
pxd_effi.param("binsU", 16)
pxd_effi.param("binsV", 24)
pxd_effi.param("singleHists", False)
main.add_module(pxd_effi)

pxd_redu = register_module('DQMHistAnalysisPXDReduction')
pxd_redu.param('histogramDirectoryName', 'PXDDAQ')
main.add_module(pxd_redu)

# output = register_module('DQMHistAnalysisOutputFile')
# output.param('SaveHistos', False)  # don't save histograms
# output.param('SaveCanvases', True)  # save canvases
# main.add_module(output)

output = register_module('DQMHistAnalysisOutputRelayMsg')
# check that port fit your root canvas server
output.param('Port', int(argv[2]))
main.add_module(output)

# Process all events
process(main)
