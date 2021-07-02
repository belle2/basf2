#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import sys
argv = sys.argv

# Set the log level to show only error and fatal messages
b2.set_log_level(b2.LogLevel.ERROR)
# set_log_level(LogLevel.INFO)
# set_log_level(LogLevel.DEBUG)

# globalTag = 'vxd_commissioning_20181030'
globalTag = 'data_reprocessing_prompt'

b2.conditions.prepend_globaltag(globalTag)
b2.conditions.prepend_testing_payloads("localdb/database.txt")


gearbox = b2.register_module('Gearbox')
geometry = b2.register_module('Geometry')


# Create main path
main = b2.create_path()

# Modules

input = b2.register_module('DQMHistAnalysisInput')
input.param('HistMemoryPath', argv[1])
input.param('AutoCanvas', False)
input.param('AutoCanvasFolders', [])
main.add_module(input)

# input = register_module('DQMHistAnalysisInputRootFile')
# input.param('InputRootFile', argv[1])
# main.add_module(input)

main.add_module(gearbox, fileName="/geometry/Belle2_earlyPhase3.xml")
main.add_module(geometry)  # , useDB=True)

pxd_charge = b2.register_module('DQMHistAnalysisPXDCharge')
pxd_charge.param('histogramDirectoryName', 'PXDER')
main.add_module(pxd_charge)

pxd_trackcharge = b2.register_module('DQMHistAnalysisPXDTrackCharge')
pxd_trackcharge.param('histogramDirectoryName', 'PXDER')
main.add_module(pxd_trackcharge)

pxd_inj = b2.register_module('DQMHistInjection')
main.add_module(pxd_inj)

pxd_commode = b2.register_module('DQMHistAnalysisPXDCM')
pxd_commode.param('histogramDirectoryName', 'PXDDAQ')
main.add_module(pxd_commode)

pxd_effi = b2.register_module('DQMHistAnalysisPXDEff')
pxd_effi.param('histogramDirectoryName', 'PXDEFF')
pxd_effi.param("binsU", 16)
pxd_effi.param("binsV", 24)
pxd_effi.param("singleHists", False)
main.add_module(pxd_effi)

pxd_redu = b2.register_module('DQMHistAnalysisPXDReduction')
pxd_redu.param('histogramDirectoryName', 'PXDDAQ')
main.add_module(pxd_redu)

# output = register_module('DQMHistAnalysisOutputFile')
# output.param('SaveHistos', False)  # don't save histograms
# output.param('SaveCanvases', True)  # save canvases
# main.add_module(output)

output = b2.register_module('DQMHistAnalysisOutputRelayMsg')
# check that port fit your root canvas server
output.param('Port', int(argv[2]))
main.add_module(output)

# Process all events
b2.process(main)
