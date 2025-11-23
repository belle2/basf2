#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2

# Set log level.
b2.set_log_level(b2.LogLevel.DEBUG)

# Create main path.
main = b2.create_path()

# Modules
inroot = b2.register_module('DQMHistAnalysisInputRootFile')
inroot.param('FileList', '/home/t2k/taichiro/public/erecodqm_e0026r001675.root')
main.add_module(inroot)
# main.add_module("DQMHistAutoCanvas") # Plot all Histo from Input not needed

trg_analysis = b2.register_module('DQMHistAnalysisTRG')
main.add_module(trg_analysis)

output = b2.register_module('DQMHistAnalysisOutputFile')
output.param('histogramDirectoryName', 'analysismodule')
output.param('SaveHistos', False)
output.param('SaveCanvases', True)
main.add_module(output)

# Process main path.
b2.process(main)

# Statistics.
print(b2.statistics)
