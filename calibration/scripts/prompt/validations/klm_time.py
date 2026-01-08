##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Validation of KLM time calibration (cable delay and time constants).
'''


import basf2
from prompt import ValidationSettings
import ROOT
from ROOT.Belle2 import KLMCalibrationChecker
import sys
import subprocess
import math
import os
import json
from ROOT import TH1F, TH2F, TCanvas, TFile, gStyle

#: Tells the automated system some details of this script
settings = ValidationSettings(name='KLM time',
                              description=__doc__,
                              download_files=['stdout'],
                              expert_config={
                                  "chunk_size": 100,
                                  "LowerTimeBoundaryRPC": -800.0,
                                  "UpperTimeBoundaryRPC": -600.0,
                                  "LowerTimeBoundaryScintillatorsBKLM": -4800.0,
                                  "UpperTimeBoundaryScintillatorsBKLM": -4600.0,
                                  "LowerTimeBoundaryScintillatorsEKLM": -4850.0,
                                  "UpperTimeBoundaryScintillatorsEKLM": -4600.0,
                              })


def run_validation(job_path, input_data_path, requested_iov, expert_config):
    '''
    Run the validation.
    Note:
      - job_path will be replaced with path/to/calibration_results
      - input_data_path will be replaced with path/to/data_path used for calibration, e.g. /group/belle2/dataprod/Data/PromptSkim/
    '''

    # Grab the expert configurations.
    expert_config = json.loads(expert_config)
    print(expert_config)
    chunk_size = expert_config['chunk_size']
    LowerTimeBoundaryRPC = expert_config['LowerTimeBoundaryRPC']
    UpperTimeBoundaryRPC = expert_config['UpperTimeBoundaryRPC']
    LowerTimeBoundaryScintillatorsBKLM = expert_config['LowerTimeBoundaryScintillatorsBKLM']
    UpperTimeBoundaryScintillatorsBKLM = expert_config['UpperTimeBoundaryScintillatorsBKLM']
    LowerTimeBoundaryScintillatorsEKLM = expert_config['LowerTimeBoundaryScintillatorsEKLM']
    UpperTimeBoundaryScintillatorsEKLM = expert_config['UpperTimeBoundaryScintillatorsEKLM']

    # Ignore the ROOT command line options.
    ROOT.PyConfig.IgnoreCommandLineOptions = True  # noqa
    # Run ROOT in batch mode.
    ROOT.gROOT.SetBatch(True)
    # Set the Belle II style.
    ROOT.gROOT.SetStyle("BELLE2")
    # And unset the stat box.
    ROOT.gStyle.SetOptStat(0)

    # Path to the database.txt file.
    database_file = os.path.join(f'{job_path}', 'KLMTime', 'outputdb', 'database.txt')

    # Check the list of runs from the file database.txt for both payloads.
    exp_run_dict_cabledelay = {}
    exp_run_dict_constants = {}
    previous_exp_cabledelay = -666
    previous_exp_constants = -666

    with open(database_file) as f:
        for line in f:
            fields = line.split(' ')
            if fields[0] == 'dbstore/KLMTimeCableDelay':
                iov = fields[2].split(',')
                exp = int(iov[0])
                run = int(iov[1])
                if exp != previous_exp_cabledelay:
                    exp_run_dict_cabledelay[exp] = [run]
                    previous_exp_cabledelay = exp
                else:
                    exp_run_dict_cabledelay[exp].append(run)
            elif fields[0] == 'dbstore/KLMTimeConstants':
                iov = fields[2].split(',')
                exp = int(iov[0])
                run = int(iov[1])
                if exp != previous_exp_constants:
                    exp_run_dict_constants[exp] = [run]
                    previous_exp_constants = exp
                else:
                    exp_run_dict_constants[exp].append(run)

    # Tweak the IoV range if the first run is 0.
    # This is needed for display purposes.
    for exp, run_list in exp_run_dict_cabledelay.items():
        run_list.sort()
        if len(run_list) > 1:
            if run_list[0] == 0 and run_list[1] > 5:
                run_list[0] = run_list[1] - 5

    for exp, run_list in exp_run_dict_constants.items():
        run_list.sort()
        if len(run_list) > 1:
            if run_list[0] == 0 and run_list[1] > 5:
                run_list[0] = run_list[1] - 5

    # Run the KLMCalibrationChecker class for cable delay.
    for exp, run_list in exp_run_dict_cabledelay.items():
        for run in run_list:
            checker = KLMCalibrationChecker()
            checker.setExperimentRun(exp, run)
            checker.setTestingPayload(database_file)
            basf2.B2INFO(f'Creating time cable delay results tree for experiment {exp}, run {run}.')
            checker.setTimeCableDelayResultsFile(f'time_cabledelay_exp{exp}_run{run}.root')
            checker.checkTimeCableDelay()

    # Run the KLMCalibrationChecker class for time constants.
    for exp, run_list in exp_run_dict_constants.items():
        for run in run_list:
            checker = KLMCalibrationChecker()
            checker.setExperimentRun(exp, run)
            checker.setTestingPayload(database_file)
            basf2.B2INFO(f'Creating time constants results tree for experiment {exp}, run {run}.')
            checker.setTimeConstantsResultsFile(f'time_constants_exp{exp}_run{run}.root')
            checker.checkTimeConstants()

    # Run the validation for cable delay.
    for exp, run_list in exp_run_dict_cabledelay.items():
        # For each experiment, merge the files in chunks of some runs.
        chunks = math.ceil(len(run_list) / chunk_size)
        chunk_files = []
        for chunk in range(chunks):
            file_name = f'time_cabledelay_exp{exp}_chunk{chunk}.root'
            run_files = [
                f'time_cabledelay_exp{exp}_run{run}.root' for run in run_list[chunk * chunk_size:(chunk + 1) * chunk_size]]
            subprocess.run(['hadd', '-f', file_name] + run_files, check=True)
            chunk_files.append(file_name)

            # Let's delete the files for single IoVs.
            for run_file in run_files:
                try:
                    os.remove(run_file)
                except OSError as e:
                    basf2.B2ERROR(f'The file {run_file} can not be removed: {e.strerror}')

        # Now merge all chunks into one final file for plotting
        final_file_name = f'time_cabledelay_exp{exp}_all.root'
        subprocess.run(['hadd', '-f', final_file_name] + chunk_files, check=True)
        input_file = ROOT.TFile(final_file_name)

        gStyle.SetOptStat(1111111)

        barrel_RPC = TH1F("barrel_RPC", "time cable delay for Barrel RPC", 100, LowerTimeBoundaryRPC, UpperTimeBoundaryRPC)
        barrel_scintillator = TH1F("barrel_scintillator", "time cable delay for Barrel scintillator",
                                   100, LowerTimeBoundaryScintillatorsBKLM, UpperTimeBoundaryScintillatorsBKLM)
        endcap_scintillator = TH1F("endcap_scintillator", "time cable delay for endcap scintillator",
                                   100, LowerTimeBoundaryScintillatorsEKLM, UpperTimeBoundaryScintillatorsEKLM)

        tree = input_file.Get("cabledelay")
        assert isinstance(tree, ROOT.TTree) == 1
        myC = TCanvas("myC")

        tree.Draw("timeDelay>>barrel_RPC", "subdetector==1 & layer>=3")
        barrel_RPC.GetXaxis().SetTitle("T_{cable} (ns)")
        barrel_RPC.GetYaxis().SetTitle("Entries")
        myC.Print("barrel_RPC.png")

        tree.Draw("timeDelay>>barrel_scintillator", "subdetector==1 & layer<3")
        barrel_scintillator.GetXaxis().SetTitle("T_{cable} (ns)")
        barrel_scintillator.GetYaxis().SetTitle("Entries")
        myC.Print("barrel_scintillator.png")

        tree.Draw("timeDelay>>endcap_scintillator", "subdetector==2")
        endcap_scintillator.GetXaxis().SetTitle("T_{cable} (ns)")
        endcap_scintillator.GetYaxis().SetTitle("Entries")
        myC.Print("endcap_scintillator.png")

        # Write out histograms
        fout = TFile("KLMTimeCableDelay.root", "recreate")
        barrel_RPC.Write()
        barrel_scintillator.Write()
        endcap_scintillator.Write()
        input_file.Close()
        fout.Close()

        # Delete the chunk files
        for chunk_file in chunk_files:
            try:
                os.remove(chunk_file)
            except OSError as e:
                basf2.B2ERROR(f'The file {chunk_file} can not be removed: {e.strerror}')

    # Run the validation for time constants.
    for exp, run_list in exp_run_dict_constants.items():
        # For each experiment, merge the files in chunks of some runs.
        chunks = math.ceil(len(run_list) / chunk_size)
        chunk_files = []
        for chunk in range(chunks):
            file_name = f'time_constants_exp{exp}_chunk{chunk}.root'
            run_files = [
                f'time_constants_exp{exp}_run{run}.root' for run in run_list[chunk * chunk_size:(chunk + 1) * chunk_size]]
            subprocess.run(['hadd', '-f', file_name] + run_files, check=True)
            chunk_files.append(file_name)

            # Let's delete the files for single IoVs.
            for run_file in run_files:
                try:
                    os.remove(run_file)
                except OSError as e:
                    basf2.B2ERROR(f'The file {run_file} can not be removed: {e.strerror}')

        # Now merge all chunks into one final file for plotting
        final_file_name = f'time_constants_exp{exp}_all.root'
        subprocess.run(['hadd', '-f', final_file_name] + chunk_files, check=True)
        input_file = ROOT.TFile(final_file_name)
        gStyle.SetOptStat(1111111)

        barrel_RPCPhi = TH2F("barrel_RPCPhi",
                             "time constants for Barrel RPC phi readout", 100, 30000, 65000, 100, 0.004, 0.015)
        barrel_RPCZ = TH2F("barrel_RPCZ",
                           "time constants for Barrel RPC Z readout", 100, 30000, 65000, 100, 0.0, 0.0025)
        barrel_scintillator_constants = TH2F("barrel_scintillator_constants",
                                             "time constants for Barrel scintillator", 100, 30000, 65000, 100, 0.075, 0.09)
        endcap_scintillator_constants = TH2F("endcap_scintillator_constants",
                                             "time constants for endcap scintillator", 100, 0, 16000, 100, 0.060, 0.075)

        tree = input_file.Get("constants")
        assert isinstance(tree, ROOT.TTree) == 1
        myC = TCanvas("myC")

        tree.Draw("delayRPCPhi:channelNumber>>barrel_RPCPhi", "subdetector==1 & layer>=3", "colz")
        barrel_RPCPhi.GetXaxis().SetTitle("Channel number")
        barrel_RPCPhi.GetYaxis().SetTitle("Time Delay constants")
        myC.Print("barrel_RPCPhi.png")

        tree.Draw("delayRPCZ:channelNumber>>barrel_RPCZ", "subdetector==1 & layer>=3", "colz")
        barrel_RPCZ.GetXaxis().SetTitle("Channel number")
        barrel_RPCZ.GetYaxis().SetTitle("Time Delay constants")
        myC.Print("barrel_RPCZ.png")

        tree.Draw("delayBKLM:channelNumber>>barrel_scintillator_constants", "subdetector==1 & layer<3", "colz")
        barrel_scintillator_constants.GetXaxis().SetTitle("Channel number")
        barrel_scintillator_constants.GetYaxis().SetTitle("Time Delay constants")
        myC.Print("barrel_scintillator_constants.png")

        tree.Draw("delayEKLM:channelNumber>>endcap_scintillator_constants", "subdetector==2", "colz")
        endcap_scintillator_constants.GetXaxis().SetTitle("Channel number")
        endcap_scintillator_constants.GetYaxis().SetTitle("Time Delay constants")
        myC.Print("endcap_scintillator_constants.png")

        fout = TFile("KLMTimeConstants.root", "recreate")
        barrel_RPCPhi.Write()
        barrel_RPCZ.Write()
        barrel_scintillator_constants.Write()
        endcap_scintillator_constants.Write()
        input_file.Close()
        fout.Close()

        # Delete the chunk files
        for chunk_file in chunk_files:
            try:
                os.remove(chunk_file)
            except OSError as e:
                basf2.B2ERROR(f'The file {chunk_file} can not be removed: {e.strerror}')


if __name__ == "__main__":
    run_validation(*sys.argv[1:])
