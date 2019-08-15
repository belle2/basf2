#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
import sys

# Set log level.
basf2.set_log_level(basf2.LogLevel.INFO)

# Create main path.
main = basf2.create_path()

# Modules
input = basf2.register_module('DQMHistAnalysisInputRootFile', Experiment=0, RunNr=0)
input.param('SelectFolders', ['KLM', 'BKLM', 'EKLM'])
input.param('InputRootFile', 'KLMDQM.root')
main.add_module(input)

klm_analysis = basf2.register_module('DQMHistAnalysisKLM')
main.add_module(klm_analysis)

output = basf2.register_module('DQMHistAnalysisOutputFile')
output.param('HistoFile', 'KLMDQMAnalysed.root')
output.param('SaveHistos', False)
output.param('SaveCanvases', True)
main.add_module(output)

# Process mian path.
basf2.process(main)

# Statistics.
print(basf2.statistics)
