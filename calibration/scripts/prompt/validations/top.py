#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Validation plots for TOP calibration.
'''

from prompt import ValidationSettings
from ROOT import TFile, TH1F, TGraph, TCanvas, TLegend, gROOT, gStyle, PyConfig
from basf2 import B2ERROR
import sys

#: Tells the automated system some details of this script
settings = ValidationSettings(name='TOP post-tracking calibration',
                              description=__doc__,
                              download_files=[],
                              expert_config=None)


def run_validation(job_path, input_data_path, requested_iov, expert_config):
    '''
    Makes validation plots
    :job_path: path to top posttracking calibration output
    :input_data_path: required argument but not used
    :requested_iov: required argument but not used
    :expert_config: required argument but not used
    '''

    # input/output file names

    inputFileName = job_path + '/TOP_validation/0/algorithm_output/TOPCalValidation.root'
    outputFileName = 'TOPValidation.pdf'

    # configuration

    PyConfig.IgnoreCommandLineOptions = True
    gROOT.SetBatch(True)
    gROOT.ForceStyle()
    gStyle.SetOptStat(0)
    gStyle.SetOptFit(1)
    gStyle.SetHistMinimumZero(True)
    gStyle.SetLabelSize(0.05, "X")
    gStyle.SetLabelSize(0.05, "Y")
    gStyle.SetTitleSize(0.06, "X")
    gStyle.SetTitleSize(0.06, "Y")
    gStyle.SetTitleOffset(0.7, "X")
    gStyle.SetTitleOffset(0.8, "Y")
    gStyle.SetGridColor(11)

    # open input file and create the canvas

    file_in = TFile.Open(inputFileName)
    if (not file_in):
        B2ERROR(inputFileName + ": file not found")
        return
    canvas = TCanvas("c1", "c1", 1000, 750)

    # prepare and fit histograms for channel T0

    channelT0s = []
    residuals = []
    residuals_all = TH1F("residuals_all", "ChannelT0 residuals; residuals [ns]", 200, -1.0, 1.0)
    h_rms = TH1F("h_rms", "Width of residuals (rms); slot; r.m.s [ps]", 16, 0.5, 16.5)
    h_core = TH1F("h_core", "Core width of residuals; slot; sigma [ps]", 16, 0.5, 16.5)
    h_out = TH1F("h_out", "Outlayers (more than 3 sigma); slot; fraction [%]", 16, 0.5, 16.5)
    for slot in range(1, 17):
        name = "slot" + '{:0=2d}'.format(slot)
        h = file_in.Get("channelT0_" + name)
        channelT0s.append(h)
        r = TH1F("residuals_" + name, "ChannelT0, " + name + "; residuals [ns]", 200, -1.0, 1.0)
        residuals.append(r)
        if not h:
            continue
        for i in range(1, h.GetNbinsX() + 1):
            if h.GetBinError(i) > 0:
                r.Fill(h.GetBinContent(i))
                residuals_all.Fill(h.GetBinContent(i))
        r.Fit("gaus", "Q")
        residuals_all.Fit("gaus", "Q")
        h_rms.SetBinContent(slot, r.GetRMS() * 1000)
        fun = r.GetFunction("gaus")
        if not fun:
            continue
        fun.SetLineWidth(1)
        h_core.SetBinContent(slot, fun.GetParameter(2) * 1000)
        mean = fun.GetParameter(1)
        wid = fun.GetParameter(2)
        n_all = 0
        n_in = 0
        for i in range(1, h.GetNbinsX() + 1):
            if h.GetBinError(i) == 0:
                continue
            n_all += 1
            if abs(h.GetBinContent(i) - mean) < 3 * wid:
                n_in += 1
        if n_all > 0:
            h_out.SetBinContent(slot, (1 - n_in / n_all) * 100)
    residuals_all.Fit("gaus", "Q")

    # open pdf file

    canvas.Print(outputFileName + "[")

    # make plots of channelT0 residuals vs. channel

    canvas.Clear()
    canvas.Divide(4, 4)
    graphs = []
    for i, h in enumerate(channelT0s):
        canvas.cd(i + 1)
        if h:
            h.SetMinimum(-1.0)
            h.SetMaximum(1.0)
            h.Draw()
            g_over = TGraph()
            g_under = TGraph()
            for ibin in range(1, h.GetNbinsX() + 1):
                if h.GetBinContent(ibin) > 1.0:
                    g_over.SetPoint(g_over.GetN(), h.GetBinCenter(ibin), 0.95)
                elif h.GetBinContent(ibin) < -1.0:
                    g_under.SetPoint(g_under.GetN(), h.GetBinCenter(ibin), -0.95)
            if g_over.GetN() > 0:
                g_over.SetMarkerStyle(22)
                g_over.SetMarkerColor(2)
                g_over.Draw("P")
                graphs.append(g_over)
            if g_under.GetN() > 0:
                g_under.SetMarkerStyle(23)
                g_under.SetMarkerColor(2)
                g_under.Draw("P")
                graphs.append(g_under)
    canvas.Print(outputFileName)

    # make plots of channelT0 residual distirbutions

    canvas.Clear()
    canvas.Divide(4, 4)
    gStyle.SetOptFit(0)
    gStyle.SetOptStat("oue")
    W = gStyle.GetStatW()
    H = gStyle.GetStatH()
    gStyle.SetStatW(0.4)
    gStyle.SetStatH(0.3)
    for i, h in enumerate(residuals):
        canvas.cd(i + 1)
        h.Draw()
    canvas.Print(outputFileName)
    gStyle.SetStatW(W)
    gStyle.SetStatH(H)

    # make summary plots for channelT0

    canvas.Clear()
    canvas.Divide(2, 2)
    gStyle.SetOptFit(111)
    gStyle.SetOptStat(0)
    histos = [residuals_all, h_rms, h_core, h_out]
    for i, h in enumerate(histos):
        canvas.cd(i + 1)
        h.Draw()
    canvas.Print(outputFileName)

    # prepare commonT0 histogram

    commonT0 = file_in.Get("commonT0")
    if commonT0:
        commonT0.Scale(1000)
        commonT0.SetYTitle(commonT0.GetYaxis().GetTitle().replace('[ns]', '[ps]'))
        commonT0.SetMinimum(-100)
        commonT0.SetMaximum(100)
        commonT0.Fit("pol0", "Q")

    # make plots for commonT0

    canvas.Clear()
    canvas.Divide(1, 2)
    canvas.GetPad(2).SetGrid()
    gStyle.SetHistMinimumZero(False)
    histos = [commonT0, file_in.Get("runIndex")]
    for i, h in enumerate(histos):
        canvas.cd(i + 1)
        if h:
            h.SetMarkerStyle(20)
            h.SetMarkerSize(0.7)
            h.Draw()
    canvas.Print(outputFileName)
    gStyle.SetHistMinimumZero(True)

    # make plots for moduleT0

    canvas.Clear()
    canvas.Divide(4, 4)
    histos = []
    for slot in range(1, 17):
        canvas.cd(slot)
        h = file_in.Get("moduleT0_slot" + '{:0=2d}'.format(slot))
        if h:
            h.Scale(1000)
            h.SetYTitle(h.GetYaxis().GetTitle().replace('[ns]', '[ps]'))
            h.SetMinimum(-100)
            h.SetMaximum(100)
            h.Fit("pol0", "Q")
            histos.append(h)
    canvas.Print(outputFileName)

    # prepare moduleT0 summary histograms

    h_offset = TH1F("h_offset", "ModuleT0: fitted offset; slot; p0 [ps]", 16, 0.5, 16.5)
    h_chi = TH1F("h_chi", "ModuleT0: chi2/ndf; slot; #chi^{2}/ndf", 16, 0.5, 16.5)
    h_res = TH1F("h_res", "ModuleT0: residuals; residual - p0 [ps]", 200, -100, 100)
    h_pul = TH1F("h_pul", "ModuleT0: pulls; (residual - p0) / error", 200, -10, 10)
    for i, h in enumerate(histos):
        if not h:
            continue
        fun = h.GetFunction("pol0")
        if not fun:
            continue
        p0 = fun.GetParameter(0)
        err = fun.GetParError(0)
        h_offset.SetBinContent(i + 1, p0)
        h_offset.SetBinError(i + 1, err)
        h_chi.SetBinContent(i + 1, fun.GetChisquare() / max(fun.GetNDF(), 1))
        for ibin in range(1, h.GetNbinsX() + 1):
            err = h.GetBinError(ibin)
            if err > 0:
                h_res.Fill(h.GetBinContent(ibin) - p0)
                h_pul.Fill((h.GetBinContent(ibin) - p0) / err)

    # plot moduleT0 summary histograms

    canvas.Clear()
    canvas.Divide(2, 2)
    histos = [h_offset, h_chi, h_res, h_pul]
    h_offset.SetStats(False)
    h_chi.SetStats(False)
    gStyle.SetOptStat("rme")
    for i, h in enumerate(histos):
        canvas.cd(i + 1)
        h.Draw()
    canvas.Print(outputFileName)
    gStyle.SetOptStat(0)

    # plot fractions of active and active-and-calibrated channels

    canvas.Clear()
    canvas.Divide(4, 4)
    legends = []
    for slot in range(1, 17):
        canvas.cd(slot)
        name = "slot" + '{:0=2d}'.format(slot)
        histos = [file_in.Get("numActive_" + name), file_in.Get("numActiveCalibrated_" + name)]
        legend = TLegend(0.1, 0.1, 0.7, 0.3)
        legends.append(legend)
        opt = ''
        for i, h in enumerate(histos):
            if h:
                h.SetMinimum(0.0)
                h.SetMaximum(1.05)
                text = h.GetTitle().split(',')[0]
                h.SetTitle(name)
                h.SetLineColor(i+1)
                h.Draw(opt)
                opt = 'same'
                legend.AddEntry(h, text)
        if legend.GetNRows() > 0:
            legend.Draw("same")
    canvas.Print(outputFileName)

    # plot fractions of time base and channelT0 calibrated channels

    canvas.Clear()
    canvas.Divide(4, 4)
    legends = []
    for slot in range(1, 17):
        canvas.cd(slot)
        name = "slot" + '{:0=2d}'.format(slot)
        histos = [file_in.Get("numTBCalibrated_" + name), file_in.Get("numT0Calibrated_" + name)]
        legend = TLegend(0.1, 0.1, 0.7, 0.3)
        legends.append(legend)
        opt = ''
        for i, h in enumerate(histos):
            if h:
                h.SetMinimum(0.0)
                h.SetMaximum(1.05)
                text = h.GetTitle().split(',')[0]
                h.SetTitle(name)
                h.SetLineColor(i+1)
                h.Draw(opt)
                opt = 'same'
                legend.AddEntry(h, text)
        if legend.GetNRows() > 0:
            legend.Draw("same")
    canvas.Print(outputFileName)

    # plot threshold efficiencies

    canvas.Clear()
    canvas.Divide(4, 4)
    legends = []
    for slot in range(1, 17):
        canvas.cd(slot)
        name = "slot" + '{:0=2d}'.format(slot)
        h = file_in.Get("thrEffi_slot" + '{:0=2d}'.format(slot))
        legend = TLegend(0.1, 0.1, 0.7, 0.3)
        legends.append(legend)
        if h:
            h.SetMinimum(0.0)
            h.SetMaximum(1.0)
            text = h.GetTitle().split(',')[0]
            h.SetTitle(name)
            h.Draw()
            legend.AddEntry(h, text)
            legend.Draw("same")
    canvas.Print(outputFileName)

    # plot BS13d asic shifts

    canvas.Clear()
    canvas.Divide(2, 2)
    for cb in range(4):
        canvas.cd(cb + 1)
        canvas.GetPad(cb + 1).SetGrid()
        h = file_in.Get("asicShifts_" + str(cb))
        if h:
            h.SetMinimum(-50.0)
            h.SetMaximum(50.0)
            h.Draw()
    canvas.Print(outputFileName)

    # plot offsets

    canvas.Clear()
    canvas.Divide(2, 2)
    offsets = [file_in.Get("svdOffset"), file_in.Get("cdcOffset")]
    sigmas = [file_in.Get("svdSigma"), file_in.Get("cdcSigma")]
    components = ['SVD', 'CDC']
    colors = [4, 2]

    canvas.cd(1)
    hmin = 0
    hmax = 0
    for h in offsets:
        if h:
            hmin = min(hmin, h.GetMinimum())
            hmax = max(hmax, h.GetMaximum())
    hmin = round(hmin, 0) - 1
    hmax = round(hmax, 0) + 1
    for i, h in enumerate(offsets):
        if h:
            h.SetMinimum(hmin)
            h.SetMaximum(hmax)
            h.SetMarkerStyle(24)
            h.SetMarkerColor(colors[i])
            h.SetLineColor(colors[i])
    first = True
    legend1 = TLegend()
    for i, h in enumerate(offsets):
        if not h or not sigmas[i] or sigmas[i].Integral() == 0:
            continue
        if first:
            h.Draw()
            first = False
        else:
            h.Draw("same")
        legend1.AddEntry(h, components[i])
    legend1.Draw("same")

    canvas.cd(2)
    hmax = 0
    for h in sigmas:
        if h:
            hmax = max(hmax, h.GetMaximum())
    for i, h in enumerate(sigmas):
        if h:
            h.SetMinimum(0)
            h.SetMaximum(hmax * 1.1)
            h.SetMarkerStyle(24)
            h.SetMarkerColor(colors[i])
            h.SetLineColor(colors[i])
    first = True
    legend2 = TLegend()
    for i, h in enumerate(sigmas):
        if not h or h.Integral() == 0:
            continue
        if first:
            h.Draw()
            first = False
        else:
            h.Draw("same")
        legend2.AddEntry(h, components[i])
    legend2.Draw("same")

    fillPatt = [file_in.Get("fillPatternOffset"), file_in.Get("fillPatternFract")]
    for i, h in enumerate(fillPatt):
        canvas.cd(3 + i)
        if h:
            if i == 1:
                for k in range(h.GetNbinsX()):
                    h.SetBinError(k + 1, h.GetBinContent(k + 1) / 1000)
            h.Draw()

    canvas.Print(outputFileName)

    # close pdf file

    canvas.Print(outputFileName + "]")

    # plot photon hits, use png format to shorten file size

    canvas.Clear()
    canvas.Divide(4, 4)
    for slot in range(1, 17):
        canvas.cd(slot)
        h = file_in.Get("hits_slot" + '{:0=2d}'.format(slot))
        if h:
            h.Draw("colz")
    canvas.SaveAs(outputFileName.replace('.pdf', 'Hits.png'))


if __name__ == "__main__":
    run_validation(*sys.argv[1:])
