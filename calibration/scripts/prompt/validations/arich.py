# -*- coding: utf-8 -*-

'''
Validation of KLM strip efficiency calibration.
'''


import basf2
from prompt import ValidationSettings
import ROOT
from ROOT.Belle2 import ARICHCalibrationChecker
import sys
import subprocess
import math


#: Tells the automated system some details of this script
settings = ValidationSettings(name='ARICH channel mask',
                              description=__doc__,
                              download_files=['stdout'],
                              expert_config={"max_events_per_file": 100})


def save_graph_to_root(graph_name):
    '''
    Save a TGraph in a ROOT file.
    '''
    graph = ROOT.gPad.GetPrimitive('Graph')
    assert isinstance(graph, ROOT.TGraph) == 1
    graph.SetName(graph_name)
    graph.Write()


def save_graph_to_pdf(canvas, root_file, graph_name, exp, chunk):
    '''
    Save a drawn TGraph in a PDF file.
    '''
    graph = root_file.Get(graph_name)
    assert isinstance(graph, ROOT.TGraph) == 1
    graph.SetMarkerStyle(ROOT.EMarkerStyle.kFullDotSmall)
    graph.SetMarkerColor(ROOT.EColor.kRed + 1)
    graph.GetXaxis().SetTitle(f'Exp. {exp} -- Run number')
    graph.GetYaxis().SetTitle('# of masked channels')
    graph.SetMinimum(0.)
    graph.SetMaximum(1.)
    graph.Draw('AP')
    ROOT.gPad.SetGridy()
    canvas.SaveAs(f'channel_mask_exp{exp}_chunk{chunk}_{graph_name}.pdf')


def run_validation(job_path, input_data_path, requested_iov, expert_config, **kwargs):
    '''
    Run the validation.
    Nota bene:
      - job_path will be replaced with path/to/calibration_results
      - input_data_path will be replaced with path/to/data_path used for calibration, e.g. /group/belle2/dataprod/Data/PromptSkim/
    '''

    # TODO: replace it with an expert dictionary when it will be possible.
    chunk_size = 100

    # Ignore the ROOT command line options.
    ROOT.PyConfig.IgnoreCommandLineOptions = True  # noqa
    # Run ROOT in batch mode.
    ROOT.gROOT.SetBatch(True)
    # Set the Belle II style.
    ROOT.gROOT.SetStyle("BELLE2")
    # And unset the stat box.
    ROOT.gStyle.SetOptStat(0)

    # Path to the database.txt file.
    database_file = f'{job_path}/ARICHChannelMasks/outputdb/database.txt'

    # Check the list of runs from the file database.txt.
    exp_run_dict = {}
    previous_exp = -666
    with open(database_file) as f:
        for line in f:
            fields = line.split(' ')
            if (fields[0] == 'dbstore/ARICHChannelMask'):
                iov = fields[2].split(',')
                exp = int(iov[0])
                run = int(iov[1])
                if (exp != previous_exp):
                    exp_run_dict[exp] = [run]
                    previous_exp = exp
                else:
                    exp_run_dict[exp].append(run)

    # Tweak the IoV range if the first run is 0.
    # This is needed for display purposes.
    for exp, run_list in exp_run_dict.items():
        run_list.sort()
        if len(run_list) > 1:
            if run_list[0] == 0 and run_list[1] > 5:
                run_list[0] = run_list[1] - 5

    # Run the KLMCalibrationChecker class.
    for exp, run_list in exp_run_dict.items():
        for run in run_list:
            checker = ARICHCalibrationChecker()
            checker.setExperimentRun(exp, run)
            checker.setTestingPayload(database_file)
            basf2.B2INFO(f'Creating arich channel mask results tree for experiment {exp}, run {run}.')
            checker.setChannelMaskResultsFile(f'channel_mask_exp{exp}_run{run}.root')
            checker.checkChannelMask()

    # Run the validation.
    for exp, run_list in exp_run_dict.items():
        # For each experiment, merge the files in chunks of some runs.
        chunks = math.ceil(len(run_list) / chunk_size)
        for chunk in range(chunks):
            file_name = f'channel_mask_exp{exp}_chunk{chunk}.root'
            run_file_names = [f'channel_mask_exp{exp}_run{run}.root' for run in run_list[chunk:(chunk + chunk_size)]]
            subprocess.run(['hadd', '-f', file_name] + run_file_names, check=True)
            input_file = ROOT.TFile(f'{file_name}')
            output_file = ROOT.TFile(f'histograms_{file_name}', 'recreate')
            output_file.cd()
            tree = input_file.Get('arich_masked')
            assert isinstance(tree, ROOT.TTree) == 1
            canvas = ROOT.TCanvas(f'canvas_exp{exp}_chunk{chunk}', 'canvas', 800, 500)
            canvas.cd()
            tree.Draw('frac_masked:run')
            save_graph_to_root("channel_mask")
            save_graph_to_pdf(canvas, output_file, "channel_mask", exp, chunk)
            input_file.Close()
            output_file.Close()


if __name__ == "__main__":
    run_validation(*sys.argv[1:])
