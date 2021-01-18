# -*- coding: utf-8 -*-
# """
# CDC tracking validation
# """

import basf2
from prompt import ValidationSettings
import sys

##############################
# REQUIRED VARIABLE #
##############################
# Will be used to construct the validation in the automated system, as well as set up the submission web forms.

#: Tells the automated system some details of this script
settings = ValidationSettings(name='CDC Tracking',
                              description=__doc__,
                              download_files=[],
                              expert_config=None)


def run_validation(job_path, input_data_path, requested_iov, **kwargs):
    # job_path will be replaced with path/to/calibration_results
    # input_data_path will be replaced with path/to/data_path used for calibration, e.g. /group/belle2/dataprod/Data/PromptSkim/

    import os
    import ROOT

    # Run ROOT in batch mode.
    ROOT.gROOT.SetBatch(True)
    ROOT.gStyle.SetOptStat(0)

    # TODO:
    #   - look for the last iteration (it can be 1, 2, 3...)
    #   - add axis labels (+ units!)
    #   - improve algo for T0 layers
    #   - join pdfs that need multiple pages

    plot_directory = "plots"
    if not os.path.exists(plot_directory):
        os.makedirs(plot_directory)

    ##########################################################
    print("******************************")
    print("****    T0 validation     ****")
    print("******************************")

    # get the rootfile
    histT0_tz2 = f'{job_path}/tz2/0/algorithm_output/histT0_tz2.root'
    f_histT0_tz2 = ROOT.TFile(histT0_tz2)

    # plot total histograms of the fitted means and sigmas for each wire
    hm_All = f_histT0_tz2.Get("hm_All")
    can1 = ROOT.TCanvas()
    hm_All.GetXaxis().SetRangeUser(-1, 0)
    hm_All.Draw("pe")
    can1.Draw()
    can1.SaveAs(f"{plot_directory}/tz2_hm_All.pdf")

    hs_All = f_histT0_tz2.Get("hs_All")
    can2 = ROOT.TCanvas()
    hs_All.GetXaxis().SetRangeUser(2, 8)
    hs_All.Draw("pe")
    can2.Draw()
    can2.SaveAs(f"{plot_directory}/tz2_hs_All.pdf")

    # plot total histogram of the fitted means for each wire + fit
    hTotal = f_histT0_tz2.Get("hTotal")
    can3 = ROOT.TCanvas()
    hTotal.Draw("pe")
    can3.Draw()
    can3.SaveAs(f"{plot_directory}/tz2_hTotal.pdf")

    # plot histograms of the fitted means per layer
    gr1 = [f_histT0_tz2.Get(f'DeltaT0/lay{i}') for i in range(56)]
    cs = [ROOT.TCanvas(f'cs{i}', '', 1200, 750) for i in range(7)]
    for i in range(7):
        cs[i].Divide(4, 2)
    for j in range(7):
        pad = [cs[j].GetPrimitive(f'cs{j}_{i+1}') for i in range(8)]
        cs[0].cd(j + 1)
        for i in range(8):
            pad[i].cd()
            pad[i].SetGrid()
            gid = 8*j+i
            if gr1[gid] is not None:
                gr1[gid].Draw("AP")
    for i in range(7):
        cs[i].Draw()
        cs[i].SaveAs(f"{plot_directory}/tz2_layers-{i}.pdf")

    ##########################################################
    print("******************************")
    print("****    TW validation     ****")
    print("******************************")

    # get the rootfile
    histT0_tw0 = f'{job_path}/tw0/1/algorithm_output/histTW_tw0.root'
    f_histT0_tw0 = ROOT.TFile(histT0_tw0)

    # plot histograms for each board
    rangeBorad = range(1, 301)
    board_1D = [f_histT0_tw0.Get(f'h1D/board_{boardID}_1') for boardID in rangeBorad]
    can = [ROOT.TCanvas(f'c{c}', f'c{c}', 1200, 3000) for c in range(12)]
    for c in range(12):
        ni, nj = 5, 5
        can[c].Divide(ni, nj)
        for j in range(ni*nj):
            can[c].cd(j+1)
            boardNumber = j+c*(ni*nj)
            if board_1D[boardNumber]:
                board_1D[boardNumber].SetMarkerStyle(2)
                board_1D[boardNumber].SetTitle(f'Board number {boardNumber}')
                board_1D[boardNumber].Draw()
        can[c].Draw()
        can[c].SaveAs(f"{plot_directory}/tw0_boards-{c}.pdf")

    ##########################################################
    print("******************************")
    print("**** sigma res validation ****")
    print("******************************")

    ##########################################################
    print("******************************")
    print("****    XT validation     ****")
    print("******************************")


if __name__ == "__main__":
    run_validation(*sys.argv[1:])
