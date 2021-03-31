#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2

# Set log level.
b2.set_log_level(b2.LogLevel.INFO)

# Create main path.
main = b2.create_path()

# Modules
inroot = b2.register_module('DQMHistAnalysisInputRootFile')
inroot.param('FileList', '/group/belle2/phase3/dqm/dqmsrv1/e0014/dqmhisto/hltdqm_e0014r000822.root')
main.add_module(inroot)

trggdl_analysis = b2.register_module('DQMHistAnalysisTRGGDL')
main.add_module(trggdl_analysis)

output = b2.register_module('DQMHistAnalysisOutputFile')
output.param('histogramDirectoryName', 'analysismodule')
output.param('SaveHistos', False)
output.param('SaveCanvases', True)
main.add_module(output)

# Process mian path.
b2.process(main)

# Statistics.
print(b2.statistics)
