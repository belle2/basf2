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
import sys


##############################
# REQUIRED VARIABLE #
##############################
# Will be used to construct the validation in the automated system, as well as set up the submission web forms.

#: Tells the automated system some details of this script
settings = ValidationSettings(name='KLM strip efficiency',
                              description=__doc__,
                              download_files=['stdout'],
                              expert_config=None)


def save_graph_to_root(graph_name):
    '''
    Save a TGraph in a ROOT file.
    '''
    graph = ROOT.gPad.GetPrimitive('Graph')
    assert isinstance(graph, ROOT.TGraph) == 1
    graph.SetName(graph_name)
    graph.Write()


def save_graph_to_pdf(root_file, graph_name):
    '''
    Save a drawn TGraph in a PDF file.
    '''
    graph = root_file.Get(graph_name)
    assert isinstance(graph, ROOT.TGraph) == 1
    graph.SetMarkerStyle(ROOT.EMarkerStyle.kFullDotSmall)
    graph.SetMarkerColor(ROOT.EColor.kCyan + 2)
    graph.GetXaxis().SetTitle("Run number")
    graph.GetYaxis().SetTitle("Plane efficiency")
    graph.SetMinimum(0.)
    graph.SetMaximum(1.)
    graph.Draw("AP")
    ROOT.gPad.SetGridy()
    canvas.SaveAs(f'efficiency_{graph_name}.pdf')


def run_validation(job_path, input_data_path, requested_iov, expert_config, **kwargs):
    '''
    Run the validation.
    Nota bene:
      - job_path will be replaced with path/to/calibration_results
      - input_data_path will be replaced with path/to/data_path used for calibration, e.g. /group/belle2/dataprod/Data/PromptSkim/
    '''

    import os
    import ROOT
    from ROOT.Belle2 import BKLMElementNumbers, KLMCalibrationChecker, KLMElementNumbers

    # Ignore the ROOT command line options.
    ROOT.PyConfig.IgnoreCommandLineOptions = True  # noqa
    # Run ROOT in batch mode.
    ROOT.gROOT.SetBatch(True)
    # Set the Belle II style.
    ROOT.gROOT.SetStyle("BELLE2")
    # And unset the stat box.
    ROOT.gStyle.SetOptStat(0)

    # Path to the database.txt file.
    database_file = f'{job_path}/KLMStripEfficiency/outputdb/database.txt'

    # Dictionary with the definition of the validation plots.
    bklm = KLMElementNumbers.c_BKLM
    eklm = KLMElementNumbers.c_EKLM
    first_rpc = BKLMElementNumbers.c_FirstRPCLayer
    graph_dictionary = {'barrel_rpcs': f'subdetector=={bklm} && layer>={first_rpc}',
                        'barrel_scintillators': f'subdetector=={bklm} && layer<{first_rpc}',
                        'endcap_scintillators': f'subdetector=={eklm}'}

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
    assert isinstance(tree, ROOT.TTree) == 1
    canvas = ROOT.TCanvas('canvas', 'canvas', 800, 500)
    canvas.cd()
    for name, cut in graph_dictionary.items():
        tree.Draw('efficiency:run', cut)
        save_graph_to_root(name)
        save_graph_to_pdf(output_file, name)
    input_file.Close()
    output_file.Close()


if __name__ == "__main__":
    run_validation(*sys.argv[1:])
