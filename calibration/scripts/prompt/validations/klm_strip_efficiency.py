# -*- coding: utf-8 -*-
"""
Checks the KLM Strip Efficiency Payloads by:
    - Reading a local database (database.txt) in order to extract the experiment and run numbers
    - For each run, it executes a dedicated basf2 class that has a direct interface with the database in order to read
      the payloads and store some values in a .root file (one file per run)
    - It merges all the .root files into a single one
    - It draws the histograms and saves them as .pdf files and as a .root file.
"""

import basf2
from prompt import ValidationSettings

##############################
# REQUIRED VARIABLE #
##############################
# Will be used to construct the validation in the automated system, as well as set up the submission web forms.

#: Tells the automated system some details of this script
settings = ValidationSettings(name='KLM strip efficiency',
                              description=__doc__,
                              download_files=['stdout'],
                              expert_config={'hello': 'world'})


def run_validation(job_path, input_data_path, requested_iov, **kwargs):
    # job_path will be replaced with path/to/calibration_results
    # input_data_path will be replaced with path/to/data_path used for calibration, e.g. /group/belle2/dataprod/Data/PromptSkim/

    import os
    import basf2
    import ROOT
    from ROOT.Belle2 import KLMCalibrationChecker

    # Run ROOT in batch mode.
    ROOT.gROOT.SetBatch(True)
    ROOT.gStyle.SetOptStat(0)

    database_file = f'{job_path}/KLMStripEfficiency/outputdb/database.txt'

    # Check the list of runs from the file database.txt.
    exp_run_list = []
    with open(database_file) as f:
        for line in f:
            fields = line.split(' ')
            if (fields[0] == 'dbstore/KLMStripEfficiency'):
                iov = fields[2].split(',')
                exp_run_list.append([int(iov[0]), int(iov[1])])

    # Run the KLMCalibrationChecker class.
    for exp_run in exp_run_list:
        exp = exp_run[0]
        run = exp_run[1]
        checker = KLMCalibrationChecker()
        checker.setExperimentRun(exp, run)
        checker.setTestingPayload(database_file)
        basf2.B2INFO(f'Creating strip efficiency results tree for experiment {exp}, run {run}.')
        checker.setStripEfficiencyResultsFile(f'strip_efficiency_exp{exp}_run{run}.root')
        checker.checkStripEfficiency()

    # Merge the .root files into a single one and draw the histograms.
    file_name = f'strip_efficiency_exp{exp_run_list[0][0]}.root'
    command = f'hadd -f {file_name} ' \
        f'strip_efficiency_exp{exp_run_list[0][0]}_run*.root'
    if (os.system(command) != 0):
        basf2.B2FATAL(f'The command "{command}" aborted during its execution.')
    input_file = ROOT.TFile(f'{file_name}')
    output_file = ROOT.TFile(f'histograms_{file_name}', 'recreate')
    output_file.cd()
    tree = input_file.Get('efficiency')
    canvas = ROOT.TCanvas('canvas', 'canvas', 800, 400)
    canvas.cd()
    tree.Draw('efficiency:run', 'subdetector==1 && layer>=3')
    canvas.SaveAs('efficiency_barrel_RPCs.pdf')
    graph = ROOT.gPad.GetPrimitive('Graph')
    graph.SetName('barrel_rpcs')
    graph.Write()
    tree.Draw('efficiency:run', 'subdetector==1 && layer<3')
    canvas.SaveAs('efficiency_barrel_scintillators.pdf')
    graph = ROOT.gPad.GetPrimitive('Graph')
    graph.SetName('barrel_scintillators')
    graph.Write()
    tree.Draw('efficiency:run', 'subdetector==2')
    canvas.SaveAs('efficiency_endcap_scintillators.pdf')
    graph = ROOT.gPad.GetPrimitive('Graph')
    graph.SetName('endcap_scintillators')
    graph.Write()
    input_file.Close()
    output_file.Close()
