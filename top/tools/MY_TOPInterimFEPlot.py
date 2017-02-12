#!/usr/bin/env python
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# draw summary plots from output of MY_TOPInterimFEAna module (run******_slot**_ana.root)
# Usage: basf2 MY_TOPInterimFEPlot <output root file of MY_TOPInterimFECheck.py> (to be called in "MY_TOPInterimFECheck.py")
# ---------------------------------------------------------------------------------------

import sys
import time
from basf2 import *
from ROOT import gROOT, gStyle, gPad, TFile, TTree, TStyle, TCanvas, TH1F, TH2F, TF1, TLegend, TAxis, TPaveStats


def MY_TOPInterimFEPlot(root_output, FitWidth=5):

    pdf_output = root_output[0:root_output.rfind(".root")] + ".pdf"

    print(root_output + " --> " + pdf_output)

    gStyle.SetFrameFillStyle(0)
    gStyle.SetFillStyle(0)
    gStyle.SetStatX(0.9)
    gStyle.SetStatY(0.9)
    gStyle.SetStatStyle(0)
    gStyle.SetOptStat(112210)
    gStyle.SetOptFit(1110)
    canvas = TCanvas()
    canvas.SetFillStyle(0)

    f = TFile(root_output)
    tr = f.Get("tree")

    canvas.Divide(2, 2)
    canvas.cd(1)
    gPad.SetFrameFillStyle(0)
    gPad.SetFillStyle(0)
    tr.Draw("TMath::FloorNint((PixelId-1)/64+1):TMath::FloorNint((PixelId-1)%64+1)>>hCalMap(64,0.5,64.5,8,0.5,8.5)",
            "IsCalCh", "colz")
    hCalMap = gROOT.FindObject("hCalMap")
    hCalMap.SetStats(False)
    hCalMap.SetTitle(root_output + " - hit map only in cal. ch.")

    canvas.cd(2)
    gPad.SetFrameFillStyle(0)
    gPad.SetFillStyle(0)
    tr.Draw("Sum$(IsCalCh)>>hCalEff(65,-0.5,64.5)")
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
    tr.Draw("TdcRaw-RefTdc>>hLaserTime(200,-245,5)", "RefTdc>0 && !IsCalCh")
    hLaserTime = gROOT.FindObject("hLaserTime")
    hLaserTime.GetXaxis().SetTitle("hit time - ref. time [sample]")
    BinWidth = hLaserTime.GetXaxis().GetBinUpEdge(1) - hLaserTime.GetXaxis().GetBinLowEdge(1)
    ytitle = "Entries[/(" + str(BinWidth) + " sample)]"
    hLaserTime.GetYaxis().SetTitle(ytitle)
    hLaserTime.SetTitle("Laser timing distribution w.r.t. cal. pulse timing")
    if hLaserTime.Integral() > 10:
        PeakTime = hLaserTime.GetXaxis().GetBinCenter(hLaserTime.GetMaximumBin())
        hLaserTime.Fit("gaus", "", "", PeakTime - FitWidth, PeakTime + FitWidth)
        func = hLaserTime.GetFunction("gaus")
        PeakTime = func.GetParameter(1)
        cut = "TMath::Abs(TdcRaw-RefTdc-(" + str(PeakTime) + "))<" + str(FitWidth)

    canvas.cd(3)
    gPad.SetFrameFillStyle(0)
    gPad.SetFillStyle(0)
    cut += " && RefTdc>0 && !IsCalCh"
    tr.Draw("TMath::FloorNint((PixelId-1)/64+1):TMath::FloorNint((PixelId-1)%64+1)>>hLaserMap(64,0.5,64.5,8,0.5,8.5)",
            cut, "colz")
    hLaserMap = gROOT.FindObject("hLaserMap")
    hLaserMap.SetStats(False)
    hLaserMap.SetTitle(root_output + " - laser hit map")

    canvas.Print(pdf_output)

    time.sleep(15)

if __name__ == '__main__':

    args = sys.argv
    if len(args) > 1:
        MY_TOPInterimFEPlot(args[1])
    else:
        print("usage:")
        print(args[0] + " (output of MY_TOPInterimFEAna)")
