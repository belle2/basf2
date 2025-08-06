#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2

# Set log level.
basf2.set_log_level(basf2.LogLevel.INFO)
useful_tags = ["online"]  # add whatever is relevant for you
for tag in useful_tags:
    basf2.conditions.append_globaltag(tag)

# Create main path.
main = basf2.create_path()

# random run with reasonable statistics (check rundb.belle2.org)
exp_num = 35
run_num = 2291
exp = "0"*(4-len(str(exp_num)))+str(exp_num)
run = "0"*(6-len(str(run_num)))+str(run_num)

# downloaded from dqm.belle2.org
dqm_histo_file = f"hlt_dqm_histo_e{exp}r{run}.root"

# Input module.
main.add_module('DQMHistAnalysisInputRootFile',
                Experiment=exp_num,
                RunList=[run_num],
                FileList=[dqm_histo_file],
                SelectHistograms=['KLM/*', 'KLMEfficiencyDQM/*', 'DAQ/*', 'DQMInfo/*'])

# Plot some canvases from Input
main.add_module("DQMHistAutoCanvas",
                IncludeFolders=["KLM", "KLMEfficiencyDQM"])

# used by DQMHistAnalysisKLM2 for calculating eff. ratios
reference = basf2.register_module('DQMHistReference')
reference.param('ReferenceFile', 'refHisto.root')  # also obtained from dqm.belle2.org
main.add_module(reference)

# DQM's KLM analysis module.
main.add_module('DQMHistAnalysisKLM')  # occupancies and timing related
main.add_module('DQMHistAnalysisKLM2')  # layer eff. measurement via muon tracks

# Output module.
main.add_module('DQMHistAnalysisOutputFile',
                OutputFolder="./",
                Filename=f'KLMDQMAnalysed_hlt_{exp}_{run}.root',
                SaveHistos=False,
                SaveCanvases=True)

# Process main path.
basf2.process(main, calculateStatistics=True)

# Print modules' statistics.
print(basf2.statistics)
