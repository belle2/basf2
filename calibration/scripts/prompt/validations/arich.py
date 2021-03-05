# -*- coding: utf-8 -*-

'''
Validation of ARICH channel masking calibration.
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
                              expert_config={"chunk_size": 100})


def run_validation(job_path, input_data_path, requested_iov, expert_config, **kwargs):
    '''
    Run the validation.

    '''

    # TODO: replace it with an expert dictionary when it will be possible.
    chunk_size = 100

    # Ignore the ROOT command line options.
    ROOT.PyConfig.IgnoreCommandLineOptions = True  # noqa

    # Run ROOT in batch mode.
    ROOT.gROOT.SetBatch(True)
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

    # Run the ARICHCalibrationChecker class.
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
            run_file_names = [
                f'channel_mask_exp{exp}_run{run}.root' for run in run_list[chunk * chunk_size:(chunk + 1) * chunk_size]]
            subprocess.run(['hadd', '-f', file_name] + run_file_names, check=True)
            input_file = ROOT.TFile(f'{file_name}')
            output_file = ROOT.TFile(f'histograms_{file_name}', 'recreate')
            output_file.cd()
            tree = input_file.Get('arich_masked')
            assert isinstance(tree, ROOT.TTree) == 1
            canvas = ROOT.TCanvas(f'canvas_exp{exp}_chunk{chunk}', 'canvas', 800, 500)
            canvas.cd()
            graphs = []
            mg = ROOT.TMultiGraph("mg", "ARICH masked channels")
            leg = ROOT.TLegend(0.9, 0.5, 0.98, 0.9)

            # draw graphs for each sector
            for i in range(0, 6):
                n = tree.Draw(f'frac_masked_sector[{i}]:run', "", "goff")
                graphs.append(ROOT.TGraph(n, tree.GetV2(), tree.GetV1()))
                graphs[i].SetMarkerStyle(7)
                graphs[i].SetMarkerColor(i + 1)
                graphs[i].SetLineWidth(0)
                graphs[i].SetTitle(f'sector {i+1}')
                leg.AddEntry(graphs[i])
                mg.Add(graphs[i])

            # draw graph for total
            n = tree.Draw(f'frac_masked:run', "", "goff")
            graphs.append(ROOT.TGraph(n, tree.GetV2(), tree.GetV1()))
            graphs[6].SetMarkerStyle(20)
            graphs[6].SetMarkerColor(6 + 1)
            graphs[6].SetLineWidth(0)
            graphs[6].SetTitle('total')
            leg.AddEntry(graphs[6])
            mg.Add(graphs[6])

            mg.Draw("ap")
            mg.GetXaxis().SetTitle('Exp. 12 -- Run number')
            mg.GetYaxis().SetTitle('fraction of masked channels')
            leg.Draw()
            ROOT.gPad.SetGridx()
            ROOT.gPad.SetGridy()
            canvas.Write()
            canvas.SaveAs(file_name.replace("root", "pdf"))

            input_file.Close()
            output_file.Close()
            # delete run files
            subprocess.run(['rm', '-rf'] + run_file_names, check=True)


if __name__ == "__main__":
    run_validation(*sys.argv[1:])
