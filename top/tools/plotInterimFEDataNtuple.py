#!/usr/bin/env python
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# draw summary plots from output of makeInterimFEDataNtuple module (run******_slot**_ana.root)
# Contibutors: Maeda Yosuke and Umberto Tamponi
#
# Usage: basf2 plotInterimFEDataNtuple <output root file of makeInterimFEDataNtuple.py> (to be called in this steering file)
# ---------------------------------------------------------------------------------------

import sys
import time
from basf2 import *
from ROOT import gROOT, gStyle, gPad, TFile, TTree, TStyle, TCanvas, TH1F, TH2F, TF1, TLegend, TAxis, TPaveStats, TLatex


def plotInterimFEDataNtupleSummary(root_output, FitWidth=5):

    pdf_output = root_output[0:root_output.rfind(".root")] + "_Summary.pdf"

    print(root_output + " --> " + pdf_output)

    gStyle.SetFrameFillStyle(0)
    gStyle.SetFillStyle(0)
    gStyle.SetStatX(0.9)
    gStyle.SetStatY(0.9)
    gStyle.SetStatStyle(0)
    gStyle.SetOptStat(112210)
    gStyle.SetOptFit(1110)
    canvas = TCanvas("cv1", "cv1", 800, 800)
    canvas.SetFillStyle(0)

    f = TFile(root_output)
    tr = f.Get("tree")

    canvas.Divide(2, 3)
    canvas.cd(1)
    gPad.SetFrameFillStyle(0)
    gPad.SetFillStyle(0)
    tr.Draw("TMath::FloorNint((pixelId-1)/64+1):TMath::FloorNint((pixelId-1)%64+1)>>hCalMap(64,0.5,64.5,8,0.5,8.5)",
            "isCalCh", "colz")
    hCalMap = gROOT.FindObject("hCalMap")
    hCalMap.SetStats(False)
    hCalMap.Scale(1. / tr.GetEntries())
    hCalMap.SetTitle(root_output + " - occupancy map only in cal. ch. (hits/evt)")

    canvas.cd(2)
    gPad.SetFrameFillStyle(0)
    gPad.SetFillStyle(0)
    tr.Draw("Sum$(isCalCh)>>hCalEff(65,-0.5,64.5)")
    hCalEff = gROOT.FindObject("hCalEff")
    hCalEff.GetXaxis().SetTitle("# of hits identified in cal. ch.")
    hCalEff.SetTitle("# of hits in cal. ch. for each event")
    gPad.Update()
    pave = hCalEff.FindObject("stats")
    pave.SetX1NDC(0.15)
    pave.SetX2NDC(0.45)
    pave.SetY1NDC(0.875)
    pave.SetY2NDC(0.625)

    canvas.cd(4)
    gPad.SetFrameFillStyle(0)
    gPad.SetFillStyle(0)
    tr.Draw("rawTdc-refTdc>>hLaserTime(250,-245,5)", "refTdc>0 && !isCalCh")
    hLaserTime = gROOT.FindObject("hLaserTime")
    hLaserTime.GetXaxis().SetTitle("hit time - ref. time [sample]")
    BinWidth = hLaserTime.GetXaxis().GetBinUpEdge(1) - hLaserTime.GetXaxis().GetBinLowEdge(1)
    ytitle = "Entries[/(" + str(BinWidth) + " sample)]"
    hLaserTime.GetYaxis().SetTitle(ytitle)
    hLaserTime.SetTitle("Laser timing distribution w.r.t. cal. pulse timing")
    if hLaserTime.Integral() > 10:
        MaxHeight = -1
        iBin = 1
        BinTime = hLaserTime.GetXaxis().GetBinLowEdge(1)
        PeakTime = BinTime
        while BinTime < (-10):
            tmpHeight = hLaserTime.GetBinContent(iBin)
            BinTime = hLaserTime.GetXaxis().GetBinCenter(iBin)
            if tmpHeight > MaxHeight:
                MaxHeight = tmpHeight
                PeakTime = BinTime
            iBin += 1
        hLaserTime.Fit("gaus", "", "", PeakTime - FitWidth, PeakTime + FitWidth)
        func = hLaserTime.GetFunction("gaus")
        PeakTime = func.GetParameter(1)
        cut = "TMath::Abs(rawTdc-refTdc-(" + str(PeakTime) + "))<" + str(FitWidth)

    canvas.cd(3)
    gPad.SetFrameFillStyle(0)
    gPad.SetFillStyle(0)
    cut += " && refTdc>0 && !isCalCh"
    tr.Draw("TMath::FloorNint((pixelId-1)/64+1):TMath::FloorNint((pixelId-1)%64+1)>>hLaserMap(64,0.5,64.5,8,0.5,8.5)",
            cut, "colz")
    hLaserMap = gROOT.FindObject("hLaserMap")
    hLaserMap.Scale(1. / tr.GetEntries())
    hLaserMap.SetStats(False)
    hLaserMap.SetTitle(root_output + " - laser occupancy (hits/event)")

    canvas.cd(5)
    gPad.SetFrameFillStyle(0)
    gPad.SetFillStyle(0)
    tr.Draw("eventNum[0]>>hNHit", "(nHit>0)*(nHit)")
    tr.Draw("eventNum[0]>>hHeader", "(nHit>0)*(nFEHeader)")
    tr.Draw("eventNum[0]>>hEmptyHeader", "(nHit>0)*(nEmptyFEHeader)")
    tr.Draw("eventNum[0]>>hWfm", "(nHit>0)*(nWaveform)")
    tr.Draw("eventNum[0]>>hError", "(nHit>0)*(errorFlag>0)")
    hNHit = gROOT.FindObject("hNHit")
    hHeader = gROOT.FindObject("hHeader")
    hEmptyHeader = gROOT.FindObject("hEmptyHeader")
    hWfm = gROOT.FindObject("hWfm")
    hError = gROOT.FindObject("hError")
    BinWidth = hNHit.GetXaxis().GetBinUpEdge(1) - hNHit.GetXaxis().GetBinLowEdge(1)
    hNHit.Scale(1. / BinWidth)
    hHeader.Scale(1. / BinWidth)
    hEmptyHeader.Scale(1. / BinWidth)
    hWfm.Scale(1. / BinWidth)
    hError.Scale(1. / BinWidth)
    hNHit.SetStats(False)
    hNHit.SetTitle(root_output + " - # hits/headers/waveforms")
    hNHit.SetLineColor(1)
    hHeader.SetLineColor(2)
    hEmptyHeader.SetLineColor(3)
    hWfm.SetLineColor(4)
    hError.SetLineColor(6)
    legend = TLegend(0.7, 0.175, 0.875, 0.4)
    legend.SetFillStyle(0)
    legend.SetBorderSize(0)
    legend.AddEntry(hNHit, "nHit")
    legend.AddEntry(hHeader, "# of FE headers")
    legend.AddEntry(hEmptyHeader, "# of empty FE headers")
    legend.AddEntry(hWfm, "# of waveforms")
    hNHit.GetXaxis().SetTitle("event number")
    ytitle = "average # every " + str(BinWidth) + " events"
    hNHit.GetYaxis().SetTitle(ytitle)
    hNHit.GetYaxis().SetRangeUser(0, 1.05 * hHeader.GetBinContent(hHeader.GetMaximumBin()))
    gPad.Clear()
    hNHit.Draw("hist")
    hHeader.Draw("samehist")
    hEmptyHeader.Draw("samehist")
    hWfm.Draw("samehist")
    legend.Draw()

    canvas.cd(6)
    gPad.SetFrameFillStyle(0)
    gPad.SetFillStyle(0)
    if hError.GetEntries() > 0:
        hError.SetStats(False)
        hError.SetTitle(root_output + " - error rate")
        hError.GetXaxis().SetTitle("event number")
        ytitle = "fraction of events with any errors for every" + str(BinWidth) + " events"
        hError.GetYaxis().SetTitle(ytitle)
        hError.Draw("hist")
    else:
        latex = TLatex()
        latex.SetNDC()
        latex.SetTextSize(0.1)
        latex.SetTextAlign(22)
        latex.DrawLatex(0.5, 0.5, "NO ERROR DETECTED")

    canvas.Print(pdf_output)

#    time.sleep(15)


def plotInterimFEDataNtupleAdvanced(root_output, FitWidth=5):

    pdf_output = root_output[0:root_output.rfind(".root")] + "_Advanced.png"
    root_out = root_output[0:root_output.rfind(".root")] + "_Advanced.root"

    print(root_output + " --> " + pdf_output)

    gStyle.SetFrameFillStyle(0)
    gStyle.SetFillStyle(0)
    gStyle.SetStatX(0.9)
    gStyle.SetStatY(0.9)
    gStyle.SetStatStyle(0)
    gStyle.SetOptStat(112210)
    gStyle.SetOptFit(1110)
    canvas = TCanvas("cv2", "cv2", 800, 800)
    canvas.SetFillStyle(0)

    f = TFile(root_output)
    tr = f.Get("tree")

    canvas.Divide(2, 3)
    canvas.cd(1)
    gPad.SetFrameFillStyle(0)
    gPad.SetFillStyle(0)
    tr.Draw("rawTdc-refTdc:winNum>>hTimeVSWin(512, 0, 512, 60, -160, -130)", "refTdc>0 && !isCalCh", "colz")
    hTimeVSWin = gROOT.FindObject("hTimeVSWin")
    hTimeVSWin.GetXaxis().SetTitle("hit first window")
    hTimeVSWin.GetYaxis().SetTitle("hit time - ref. time [sample]")
    hTimeVSWin.SetStats(False)
    hTimeVSWin.SetTitle(root_output + " hit time - ref time  VS window number.")

    canvas.cd(2)
    gPad.SetFrameFillStyle(0)
    gPad.SetFillStyle(0)
    tr.Draw("rawTdc-refTdc:sample>>hTimeVSSample(2560, 0, 256, 60, -160, -130)", "refTdc>0 && !isCalCh", "colz")
    hTimeVSSample = gROOT.FindObject("hTimeVSSample")
    hTimeVSSample.SetStats(False)
    hTimeVSSample.GetXaxis().SetTitle("hit sample [sample]")
    hTimeVSSample.GetYaxis().SetTitle("hit time - ref. time [sample]")
    hTimeVSSample.SetTitle("hit time - ref time VS hit time.")
    gPad.Update()

    canvas.cd(3)
    gPad.SetFrameFillStyle(0)
    gPad.SetFillStyle(0)
    tr.Draw("rawTdc-refTdc:pixelId>>hTimeVSPixel(512, 0, 512, 60, -160, -130 )", "refTdc>0 && !isCalCh", "colz")
    hTimeVSPixel = gROOT.FindObject("hTimeVSPixel")
    hTimeVSPixel.SetStats(False)
    hTimeVSPixel.GetXaxis().SetTitle("pixel ID")
    hTimeVSPixel.GetYaxis().SetTitle("hit time - ref. time [sample]")
    hTimeVSPixel.SetTitle("hit time - ref time VS pixel ID.")
    gPad.Update()

    canvas.cd(4)
    gPad.SetFrameFillStyle(0)
    gPad.SetFillStyle(0)
    tr.Draw("height:sample>>hHeightVSSample(256, 0, 256, 200, 0, 1000)", "", "colz")
    hHeightVSSample = gROOT.FindObject("hHeightVSSample")
    hHeightVSSample.SetStats(False)
    hHeightVSSample.GetXaxis().SetTitle("hit sample [sample]")
    hHeightVSSample.GetYaxis().SetTitle("hit amplitude [ADC] ")
    hHeightVSSample.SetTitle("amplitude VS hit time (incl calpulse).")
    gPad.SetLogz()
    gPad.Update()

    canvas.cd(5)
    gPad.SetFrameFillStyle(0)
    gPad.SetFillStyle(0)
    tr.Draw("sample>>hHitTime(400, -50, 350)")
    hHitTime = gROOT.FindObject("hHitTime")
    hHitTime.SetStats(False)
    hHitTime.GetXaxis().SetTitle("hit sample [sample]")
    hHitTime.GetYaxis().SetTitle("entries / 1 sample ")
    hHitTime.SetTitle("hit sample (incl calpulse).")
    gPad.Update()

    canvas.cd(6)
    gPad.SetFrameFillStyle(0)
    gPad.SetFillStyle(0)
    tr.Draw("Sum$(!isCalCh)>>hHitEff(200, 0, 200)", "rawTdc-refTdc > -160 && rawTdc-refTdc < -130")
    hHitEff = gROOT.FindObject("hHitEff")
    hHitEff.GetXaxis().SetTitle("# of hits candidates per event")
    hHitEff.SetTitle("# of laser hit candidates for each event in -160 < time < -130")
    gPad.Update()
    pave = hHitEff.FindObject("stats")
    pave.SetX1NDC(0.65)
    pave.SetX2NDC(0.95)
    pave.SetY1NDC(0.875)
    pave.SetY2NDC(0.625)

    canvas.Print(pdf_output)

    root_plot_out = TFile(root_out, "recreate")
    hTimeVSSample.Write()
    hTimeVSPixel.Write()
    hHeightVSSample.Write()
    hTimeVSWin.Write()
    hHitTime.Write()
    hHitEff.Write()

    root_plot_out.Close()
#    time.sleep(15)


if __name__ == '__main__':

    args = sys.argv
    if len(args) > 1:
        plotInterimFEDataNtupleSummary(args[1])
        plotInterimFEDataNtupleAdvanced(args[1])
    else:
        print("usage:")
        print(args[0] + " (output of plotInterimFEData)")
