#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import sys

# Set log level.
set_log_level(LogLevel.INFO)

# Create main path.
main = create_path()

# Modules
inroot = register_module('DQMHistAnalysisInputRootFile')
inroot.param('FileList', '/group/belle2/phase3/dqm/dqmsrv1/e0014/dqmhisto/hltdqm_e0014r000822.root')
main.add_module(inroot)

trggdl_analysis = register_module('DQMHistAnalysisTRGGDL')
main.add_module(trggdl_analysis)

output = register_module('DQMHistAnalysisOutputFile')
output.param('histogramDirectoryName', 'analysismodule')
output.param('SaveHistos', False)
output.param('SaveCanvases', True)
main.add_module(output)

# Process mian path.
process(main)

# Statistics.
print(statistics)
