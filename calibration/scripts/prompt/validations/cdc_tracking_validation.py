# -*- coding: utf-8 -*-
# """
# CDC tracking validation
# """

import basf2
from prompt import ValidationSettings
import sys
import os

#: Tells the automated system some details of this script
settings = ValidationSettings(name='CDC Tracking',
                              description=__doc__,
                              download_files=[],
                              expert_config=None)


def findLastIteration(job_path, algorithm):
    root = f"{job_path}/{algorithm}/"
    list_dirs = [item for item in os.listdir(root) if item != 'outputdb']
    list_dirs.sort(reverse=True)
    print(f"The following iterations are available for {algorithm} algorithm: {list_dirs}")
    print(f"--> {list_dirs[0]} will be used for generating the plots")
    return list_dirs[0]


def run_validation(job_path, input_data_path, requested_iov, expert_config, **kwargs):
    # job_path will be replaced with path/to/calibration_results
    # input_data_path will be replaced with path/to/data_path used for calibration, e.g. /group/belle2/dataprod/Data/PromptSkim/

    # expert_config = kwargs.get("expert_config")
    # file_extension = expert_config["file_extension"]

    file_extension = 'pdf'

    import os
    import ROOT

    # Run ROOT in batch mode.
    ROOT.gROOT.SetBatch(True)
    ROOT.gStyle.SetOptStat(0)

    plot_directory = "plots"
    if not os.path.exists(plot_directory):
        os.makedirs(plot_directory)

    ##########################################################
    print("****   T0 validation plots   ****")

    algo = 'tz2'
    lastIt = findLastIteration(job_path, algo)
    histT0_tz2 = f'{job_path}/{algo}/{lastIt}/algorithm_output/histT0_{algo}.root'
    f_histT0_tz2 = ROOT.TFile(histT0_tz2)

    # plot total histograms of the fitted means and sigmas for each wire
    hm_All = f_histT0_tz2.Get("hm_All")
    can1 = ROOT.TCanvas()
    hm_All.GetXaxis().SetRangeUser(-1, 0)
    hm_All.GetXaxis().SetTitle(r"<\Delta t>  [ns]")
    hm_All.Draw("pe")
    can1.Draw()
    can1.SaveAs(f"{plot_directory}/tz2_hm_All.{file_extension}")

    hs_All = f_histT0_tz2.Get("hs_All")
    can2 = ROOT.TCanvas()
    hs_All.GetXaxis().SetRangeUser(2, 8)
    hs_All.GetXaxis().SetTitle(r"\sigma(\Delta t)  [ns]")
    hs_All.Draw("pe")
    can2.Draw()
    can2.SaveAs(f"{plot_directory}/tz2_hs_All.{file_extension}")

    # plot total histogram of the fitted means for each wire + fit
    hTotal = f_histT0_tz2.Get("hTotal")
    can3 = ROOT.TCanvas()
    hTotal.GetXaxis().SetTitle(r"<\Delta t>   [ns]")
    hTotal.Draw("pe")
    can3.Draw()
    can3.SaveAs(f"{plot_directory}/tz2_hTotal.{file_extension}")

    # plot histograms of the fitted means per layer
    gr1 = [f_histT0_tz2.Get(f'DeltaT0/lay{i}') for i in range(56)]
    cs = [ROOT.TCanvas(f'cs{i}', '', 1200, 750) for i in range(7)]
    for i in range(7):
        cs[i].Divide(4, 2)
    for j in range(7):
        pad = [cs[j].GetPrimitive(f'cs{j}_{i + 1}') for i in range(8)]
        cs[0].cd(j + 1)
        for i in range(8):
            pad[i].cd()
            pad[i].SetGrid()
            gid = 8 * j + i
            if gr1[gid]:
                gr1[gid].Draw("AP")
    for i in range(7):
        cs[i].Draw()
        cs[i].SaveAs(f"{plot_directory}/{algo}_layers-{i}.{file_extension}")

    ##########################################################
    print("****   TW validation plots   ****")

    algo = 'tw0'
    lastIt = findLastIteration(job_path, algo)
    histTW_tw0 = f'{job_path}/{algo}/{lastIt}/algorithm_output/histTW_{algo}.root'
    f_histTW_tw0 = ROOT.TFile(histTW_tw0)

    # plot histograms for each board
    rangeBorad = range(1, 301)
    board_1D = [f_histTW_tw0.Get(f'h1D/board_{boardID}_1') for boardID in rangeBorad]
    can = [ROOT.TCanvas(f'c{c}', f'c{c}', 2000, 1500) for c in range(12)]
    for c in range(12):
        ni, nj = 5, 5
        can[c].Divide(ni, nj)
        for j in range(ni * nj):
            can[c].cd(j + 1)
            boardNumber = j + c * (ni * nj)
            if board_1D[boardNumber]:
                board_1D[boardNumber].SetMarkerStyle(2)
                board_1D[boardNumber].SetTitle(f'Board number {boardNumber}')
                board_1D[boardNumber].GetXaxis().SetTitle("ADC count")
                board_1D[boardNumber].GetYaxis().SetTitle(r"$\Delta t$   [ns]")
                board_1D[boardNumber].Draw()
        can[c].Draw()
        can[c].SaveAs(f"{plot_directory}/{algo}_boards-{c}.{file_extension}")

    ##########################################################
    print("**** sigma res validation plots ****")

    algo = 'sr0'
    lastIt = findLastIteration(job_path, algo)
    histsr_sr0 = f'{job_path}/{algo}/{lastIt}/algorithm_output/histSigma_{algo}.root'
    f_histsr_sr0 = ROOT.TFile(histsr_sr0)
    print(f"Plots from {algo}/{lastIt}/algorithm_output/histSigma_{algo}.root")

    alpha = 4
    theta = 3
    for LR in [0, 1]:
        histograms = [f_histsr_sr0.Get(f'lay_{ilay}/sigma2_lay{ilay}_lr{LR}_al{alpha}_th{theta}') for ilay in range(56)]
        ncols = 5
        count_h = 0
        for h in histograms:
            if h:
                count_h += 1
        div, mod = count_h // ncols, count_h % ncols
        nrows = div
        if mod != 0:
            nrows += 1

        print(f"number of valid histograms = {count_h} => canvas layout = ({ncols}, {nrows})")

        c2 = ROOT.TCanvas('c2', '', ncols * 700, nrows * 300)
        c2.Divide(ncols, nrows)
        j = 0
        for h in histograms:
            if h:
                j = j + 1
                c2.cd(j)
                h.GetXaxis().SetTitle("drift lenght  [cm]")
                h.GetYaxis().SetTitle("#sigma_{r}^{2} = #sigma_{u}.#sigma_{d}")
                h.Draw("AP")
                h.Draw()
        c2.Draw()
        c2.SaveAs(f"{plot_directory}/{algo}_lr{LR}_al{alpha}_th{theta}.{file_extension}")

    ##########################################################
    print("****   XT validation plots  ****")

    algo = 'xt0'
    lastIt = findLastIteration(job_path, algo)
    histXT_xt0 = f'{job_path}/{algo}/{lastIt}/algorithm_output/histXT_{algo}.root'
    f_histXT_xt0 = ROOT.TFile(histXT_xt0)

    # get histograms for all layers for a specific incident angles
    thetaID = 1
    alphaID = 3
    for LR in [0, 1]:
        histograms = [f_histXT_xt0.Get(f'lay_{layer}/m_hProf{layer}_{LR}_{alphaID}_{thetaID}') for layer in range(56)]
        # get the number of plots in the canvas
        ncols = 3
        count_h = 0
        for h in histograms:
            if h:
                count_h += 1
        div, mod = count_h // ncols, count_h % ncols
        nrows = div
        if mod != 0:
            nrows += 1

        c2 = ROOT.TCanvas('c2', '', ncols * 700, nrows * 300)
        c2.Divide(ncols, nrows)
        j = 0
        for histo in histograms:
            if histo:
                j = j + 1
                c2.cd(j)
                histo.GetYaxis().SetRangeUser(-1.2, 1.2)
                histo.SetMarkerStyle(2)
                histo.Draw()
        c2.Draw()
        c2.SaveAs(f"{plot_directory}/{algo}_lr{LR}_al{alphaID}_th{thetaID}.{file_extension}")


if __name__ == "__main__":
    run_validation(*sys.argv[1:])
