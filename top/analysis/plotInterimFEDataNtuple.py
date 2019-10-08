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
from ROOT import gROOT
gROOT.SetBatch(True)
from ROOT import gStyle, gPad, TFile, TTree, TStyle, TCanvas, TH1F, TH2F, TF1, TLegend, TAxis, TPaveStats, TLatex, TLine, TArrow


def plotInterimFEDataNtupleSummary(root_output, FitWidth=2, IsOfflineFEDisabled=False):

    DelayTimeRefl = 30  # [ns]
    ReflHalfTimwRange = 5  # [ns]

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
    canvas.Print((pdf_output + "["))

    f = TFile(root_output)
    tr = f.Get("tree")
    nEntries = tr.GetEntries()

    latex = TLatex()
    latex.SetNDC()
    latex.SetTextSize(0.03)
    latex.SetTextAlign(33)

    print("making summary plot for all the slots...")
    canvas.Clear()
    canvas.Divide(2, 2)
    canvas.cd(1)
    gPad.SetFrameFillStyle(0)
    gPad.SetFillStyle(0)
    tr.Draw("eventNum[0]>>hNHit", "(nHit>0)*(Sum$(offlineFlag<=0))")
    tr.Draw("eventNum[0]>>hHeader", "(nHit>0)*(nFEHeader)")
    tr.Draw("eventNum[0]>>hEmptyHeader", "(nHit>0)*(nEmptyFEHeader)")
    tr.Draw("eventNum[0]>>hWfm", "(nHit>0)*(nWaveform)")
    tr.Draw("eventNum[0]>>hError", "(nHit>0)*(errorFlag>0)")
    hNHit = gROOT.FindObject("hNHit")
    if hNHit.Integral() < 1:
        print("ERROR : no hits were found.")
        return
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
    legend = TLegend(0.6, 0.175, 0.875, 0.4)
    legend.SetFillStyle(0)
    legend.SetBorderSize(0)
    legend.AddEntry(hNHit, "nHit (no offline FE)")
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
    if hWfm.Integral() > 0:
        evtFracNoWfm = 1 - hWfm.Integral() / hNHit.Integral()
        print("fraction of events without waveform : " + str(int(1000 * evtFracNoWfm) / 10) + "%")
        latex.DrawLatex(0.875, 0.175, "fraction of no-waveform events : " + str(int(1000 * evtFracNoWfm) / 10) + "%")
    print(".", end="")

    canvas.cd(2)
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
        latex.SetTextSize(0.1)
        latex.SetTextAlign(22)
        latex.DrawLatex(0.5, 0.5, "NO ERROR DETECTED")
    print(".", end="")

    canvas.cd(3)
    gPad.SetFrameFillStyle(0)
    gPad.SetFillStyle(0)
    hSlotNumAll = TH1F("hSlotNumAll", "", 17, -0.5, 16.5)
    hSlotNumNoCalCh = TH1F("hSlotNumNoCalCh", "", 17, -0.5, 16.5)
    basicHitSelection = "hitQuality>0"  # && hitQuality<50 && hitQuality%10!=4"
    basicHitSelectionSingle = basicHitSelection + "&& offlineFlag<=0"
    tr.Draw("Sum$(" + basicHitSelectionSingle + ")>>hNHitAllSlots(100,-0.5,499.5)")
    tr.Draw("Sum$(" + basicHitSelectionSingle + "&&!isCalCh)>>hNHitAllSlotsNoCalCh(100,-0.5,499.5)")
    tr.Draw("rawTime>>hRawTimeAll(128,-128,640)", "hitQuality%10<4 && hitQuality>0")
    hNHitAll = [gROOT.FindObject("hNHitAllSlots")]
    hNHitNoCalCh = [gROOT.FindObject("hNHitAllSlotsNoCalCh")]
    hRawTimeList = [gROOT.FindObject("hRawTimeAll")]
    nSlotEntriesAll = []
    nSlotEntriesNoCalCh = []
    nSlotEntriesCalCh = []
    nSlotEntriesCalChWfm = []
    nHitMeanAll = []
    nHitMeanNoCalCh = []
    nHitMeanLaser = []
    strMultiplicity = "0"
    for slot in range(1, 17):
        slotCut = " && slotNum==" + str(slot)
        tr.Draw("Sum$(" + basicHitSelectionSingle + slotCut + ")>>hNHitAll" + str(slot) + "(90,2.5,92.5)")
        tr.Draw("Sum$(" + basicHitSelectionSingle + " && !isCalCh" + slotCut + ")>>hNHitNoCalCh" + str(slot) + "(90,2.5,92.5)")
        tr.Draw("rawTime>>hRawTime" + str(slot) + "(128,-128,1152)",
                "hitQuality>0 " + slotCut)
        hNHitAllTmp = gROOT.FindObject("hNHitAll" + str(slot))
        hNHitNoCalChTmp = gROOT.FindObject("hNHitNoCalCh" + str(slot))
        hRawTimeTmp = gROOT.FindObject("hRawTime" + str(slot))
        hNHitAll.append(hNHitAllTmp)
        hNHitNoCalCh.append(hNHitNoCalChTmp)
        hRawTimeList.append(hRawTimeTmp)
        nSlotEntriesAllTmp = hNHitAllTmp.Integral(hNHitAllTmp.GetXaxis().FindBin(3), hNHitAllTmp.GetXaxis().GetNbins() + 1)
        nSlotEntriesNoCalChTmp = hNHitNoCalChTmp.Integral(
            hNHitNoCalChTmp.GetXaxis().FindBin(3),
            hNHitNoCalChTmp.GetXaxis().GetNbins() + 1)
        hSlotNumAll.SetBinContent(slot + 1, nSlotEntriesAllTmp)
        hSlotNumNoCalCh.SetBinContent(slot + 1, nSlotEntriesNoCalChTmp)
        nSlotEntriesAll.append(nSlotEntriesAllTmp)
        nSlotEntriesCalCh.append(-1)
        nSlotEntriesCalChWfm.append(-1)
        nSlotEntriesNoCalCh.append(nSlotEntriesNoCalChTmp)
        nHitMeanAll.append(hNHitAllTmp.GetMean())
        nHitMeanNoCalCh.append(hNHitNoCalChTmp.GetMean())
        nHitMeanLaser.append(-1)  # defined later
        strMultiplicity += (" + (Sum$(" + basicHitSelectionSingle + slotCut + ")>=3)")
        print(".", end="")

    hSlotNumAll.SetLineColor(1)
    hSlotNumNoCalCh.SetLineColor(2)
    hSlotNumAll.SetFillStyle(0)
    hSlotNumNoCalCh.SetFillStyle(0)
    hSlotNumAll.SetStats(False)
    hSlotNumAll.Draw()
    hSlotNumAll.GetYaxis().SetRangeUser(0, nEntries * 1.2)
    hSlotNumAll.SetTitle("hit event number (nHit>=3) for each slot and multiplicity")
    hSlotNumAll.GetXaxis().SetTitle("slot number")
    line = TLine()
    line.SetLineColor(4)
    line.SetLineStyle(2)
    line.DrawLine(-0.5, nEntries, 16.5, nEntries)
    hSlotNumNoCalCh.Draw("same")
    tr.Draw(strMultiplicity + ">>hMultiplicity", "", "same")
    hMultiplicity = gROOT.FindObject("hMultiplicity")
    hMultiplicity.SetFillStyle(0)
    hMultiplicity.SetLineColor(3)
    legendSlot = TLegend(0.5, 0.65, 0.875, 0.875)
    legendSlot.SetFillStyle(0)
    legendSlot.SetBorderSize(0)
    legendSlot.AddEntry(hSlotNumAll, "all channels, single hit")
    legendSlot.AddEntry(hSlotNumNoCalCh, "w/o cal. channels, single hit")
    legendSlot2 = TLegend(legendSlot)
    legendSlot2.AddEntry(hMultiplicity, "# of hit modules")
    legendSlot2.Draw()
    print(".", end="")

    canvas.cd(4)
    gPad.SetFrameFillStyle(0)
    gPad.SetFillStyle(0)
    hNHitAll[0].SetLineColor(1)
    hNHitNoCalCh[0].SetLineColor(2)
    hNHitAll[0].SetStats(False)
    hNHitAll[0].SetTitle("# of total FE hits for all available slots")
    hNHitAll[0].GetXaxis().SetTitle("# of hits")
    hNHitAll[0].Draw()
    hNHitNoCalCh[0].Draw("same")
    legendSlot.Draw()
    LaserAdded = False
    print(".")

    canvas.Print(pdf_output)
    canvas.Clear()

    canvas.Divide(4, 4)
    for slot in range(1, 17):
        canvas.cd(slot)
        hRawTime = hRawTimeList[slot]
        hRawTime.SetStats(False)
        hRawTime.SetTitle("rawTime distribution")
        hRawTime.GetXaxis().SetTitle("rawTime")
        ytitle = "Entries [/(" + str(hRawTime.GetXaxis().GetBinUpEdge(1) - hRawTime.GetXaxis().GetBinLowEdge(1)) + " samples)]"
        hRawTime.GetYaxis().SetTitle(ytitle)
        slotstr = "slot" + "{0:02d}".format(slot)
        hRawTime.SetTitle("rawTime distribution : " + slotstr)
        hRawTime.Draw()
    canvas.Print(pdf_output)
    canvas.Clear()

    for slot in range(1, 17):

        slotCut = "slotNum==" + str(slot)
        if tr.GetEntries(slotCut) < 1:
            print("slot : " + str(slot) + " : no events")
            continue
        else:
            print("slot : " + str(slot) + " : processing...")

        slotstr = "slot" + "{0:02d}".format(slot)

        gStyle.SetStatX(0.875)
        gStyle.SetStatY(0.875)
        canvas.Clear()
        canvas.Divide(2, 4)

        # hit map for calibration channels (without offline FE)
        canvas.cd(1)
        gPad.SetFrameFillStyle(0)
        gPad.SetFillStyle(0)
        tr.Draw("TMath::FloorNint((pixelId-1)/64+1):TMath::FloorNint((pixelId-1)%64+1)>>hCalMap" +
                str(slot) + "(64,0.5,64.5,8,0.5,8.5)", ("isCalCh && offlineFlag<=0 && " + slotCut), "colz")
        hCalMap = gROOT.FindObject("hCalMap" + str(slot))
        hCalMap.SetStats(False)
        hCalMap.Scale(1. / nEntries)
        hCalMap.SetTitle(root_output + " - occupancy map only in cal. ch. (hits/evt) : " + slotstr)

        # number of hits for calibration channels (both without and with offline FE)
        canvas.cd(2)
        gPad.SetFrameFillStyle(0)
        gPad.SetFillStyle(0)
        tr.Draw("Sum$((isCalCh)*(offlineFlag<=0)*(" + slotCut + "))>>hCalEff" + str(slot) + "(65,-0.5,64.5)")
        hCalEff = gROOT.FindObject("hCalEff" + str(slot))
        hCalEff.GetXaxis().SetTitle("# of hits identified in cal. ch.")
        hCalEff.SetTitle("# of hits in cal. ch. for each event : " + slotstr)
        IsCosmicRayData = True if (hCalEff.GetMean() < 1.) else False
        tr.Draw("Sum$((hitQuality>=200)*(" + slotCut + "))>>hCalEffOfflineFE" + str(slot) + "(65,-0.5,64.5)", "", "same")
        hCalEffOfflineFE = gROOT.FindObject("hCalEffOfflineFE" + str(slot))
        hCalEffOfflineFE.SetLineColor(2)
        hCalEff.GetYaxis().SetRangeUser(0, max(hCalEff.GetBinContent(hCalEff.GetMaximumBin()),
                                               hCalEffOfflineFE.GetBinContent(hCalEffOfflineFE.GetMaximumBin())) * 1.3)
        legendEff = TLegend(0.15, 0.875, 0.5, 0.7)
        legendEff.SetFillStyle(0)
        legendEff.AddEntry(hCalEff, "w/o offline FE (" + str(int(hCalEff.GetMean() * 10 + 0.5) / 10.) + ")")
        if IsOfflineFEDisabled:
            legendEff.AddEntry(hCalEffOfflineFE, "w/ offline FE (disabled)")
        else:
            legendEff.AddEntry(hCalEffOfflineFE, "w/ offline FE (" + str(int(hCalEffOfflineFE.GetMean() * 10 + 0.5) / 10.) + ")")
        legendEff.Draw()
        gPad.Update()
        nSlotEntriesCalCh[slot - 1] = hCalEff.GetMean()
        nSlotEntriesCalChWfm[slot - 1] = hCalEffOfflineFE.GetMean()

        # laser timing distribution with offline FE (if available)
        canvas.cd(5)
        gPad.SetFrameFillStyle(0)
        gPad.SetFillStyle(0)
        gPad.SetLogy()
        cut = "refTime>0 && hitQuality>0 && hitQuality<50 && " + slotCut
        if IsOfflineFEDisabled:
            ytitle_hLaser = "Laser timing distribution (w/o offline FE) : " + slotstr
        else:
            cut += " && hitQuality>=10"
            ytitle_hLaser = "Laser timing distribution (w/ offline FE) : " + slotstr
        tr.Draw("time-refTime>>hLaserTime" + str(slot) + "(180,-125,55)", cut + " && !isCalCh")
        tr.Draw("time-refTime>>hLaserTimeCal" + str(slot) + "(180,-125,55)", cut + " && isCalCh", "same")
        hLaserTime = gROOT.FindObject("hLaserTime" + str(slot))
        hLaserTimeCal = gROOT.FindObject("hLaserTimeCal" + str(slot))
        hLaserTime.GetXaxis().SetTitle("hit time - ref. time [ns]")
        BinWidth = hLaserTime.GetXaxis().GetBinUpEdge(1) - hLaserTime.GetXaxis().GetBinLowEdge(1)
        ytitle = "Entries[/(" + str(BinWidth) + " ns)]"
        hLaserTime.GetYaxis().SetTitle(ytitle)
        hLaserTime.SetTitle(ytitle_hLaser)
        hLaserTimeCal.SetLineColor(2)
        cutDirectHit = cut
        cutReflHit = cut
        PeakTime = 1000000
        legX1 = 0.3
        legX2 = 0.475
        if hLaserTime.Integral(1, int(hLaserTime.GetXaxis().GetNbins() / 2)) < 0.5 * hLaserTime.Integral():
            gStyle.SetStatX(0.475)
            gStyle.SetStatY(0.875)
            legX1 = 0.7
            legX2 = 0.875
        legendTime = TLegend(legX1, 0.875, legX2, 0.7)
        legendTime.SetFillStyle(0)
        legendTime.SetBorderSize(0)
        legendTime.AddEntry(hLaserTime, "non-cal. channel")
        legendTime.AddEntry(hLaserTimeCal, "cal. channel")
        legendTime.Draw()
        if hLaserTime.Integral() > 10:
            MaxHeight = -1
            iBin = 1
            BinTime = hLaserTime.GetXaxis().GetBinLowEdge(1)
            PeakTime = BinTime
            while BinTime < hLaserTime.GetXaxis().GetBinUpEdge(hLaserTime.GetXaxis().GetNbins()):
                tmpHeight = hLaserTime.GetBinContent(iBin)
                BinTime = hLaserTime.GetXaxis().GetBinCenter(iBin)
                if tmpHeight > MaxHeight:
                    MaxHeight = tmpHeight
                    PeakTime = BinTime
                iBin += 1
            hLaserTime.Fit("gaus", "+", "", PeakTime - FitWidth, PeakTime + FitWidth)
            if hLaserTime.GetFunction("gaus"):
                func = hLaserTime.GetFunction("gaus")
                PeakTime = func.GetParameter(1)
                PeakHeight = func.GetParameter(0)
            else:
                PeakHeight = hLaserTime.GetBinContent(hLaserTime.GetXaxis().FindBin(PeakTime))
            cutDirectHit += " && TMath::Abs(time-refTime-(" + str(PeakTime) + "))<" + str(FitWidth)
            cutReflHit += " && TMath::Abs(time-refTime-(" + str(PeakTime + DelayTimeRefl) + "))<" + str(ReflHalfTimwRange)
            line = TLine()
            line.SetLineColor(3)
            arrow = TArrow()
            arrow.SetLineColor(3)
            line.DrawLine(PeakTime - FitWidth, 0.5, PeakTime - FitWidth, PeakHeight / 10.)
            line.DrawLine(PeakTime + FitWidth, 0.5, PeakTime + FitWidth, PeakHeight / 10.)
            line.DrawLine(
                PeakTime +
                DelayTimeRefl -
                ReflHalfTimwRange,
                0.5,
                PeakTime +
                DelayTimeRefl -
                ReflHalfTimwRange,
                PeakHeight /
                100.)
            line.DrawLine(
                PeakTime +
                DelayTimeRefl +
                ReflHalfTimwRange,
                0.5,
                PeakTime +
                DelayTimeRefl +
                ReflHalfTimwRange,
                PeakHeight /
                100.)
            arrow.DrawArrow(PeakTime - FitWidth, 5, PeakTime + FitWidth, 5, 0.01, "<>")
            arrow.DrawArrow(
                PeakTime +
                DelayTimeRefl -
                ReflHalfTimwRange,
                5,
                PeakTime +
                DelayTimeRefl +
                ReflHalfTimwRange,
                5,
                0.01,
                "<>")

        # laser hit map for direct hit peak with offline feature extraction (if available)
        canvas.cd(3)
        gPad.SetFrameFillStyle(0)
        gPad.SetFillStyle(0)
        tr.Draw("TMath::FloorNint((pixelId-1)/64+1):TMath::FloorNint((pixelId-1)%64+1)>>hLaserMap" +
                str(slot) + "(64,0.5,64.5,8,0.5,8.5)", cutDirectHit, "colz")
        hLaserMap = gROOT.FindObject("hLaserMap" + str(slot))
        hLaserMap.Scale(1. / tr.GetEntries())
        hLaserMap.SetStats(False)
        hLaserMap.SetTitle(root_output + " - laser direct hit occupancy (hits/event)")

        # laser hit map for reflected photon peak with offline feature extraction (if available)
        canvas.cd(4)
        gPad.SetFrameFillStyle(0)
        gPad.SetFillStyle(0)
        tr.Draw("TMath::FloorNint((pixelId-1)/64+1):TMath::FloorNint((pixelId-1)%64+1)>>hLaserReflMap" +
                str(slot) + "(64,0.5,64.5,8,0.5,8.5)",
                ("hitQuality>0 && hitQuality%10!=4 && " + slotCut if IsCosmicRayData else cutReflHit), "colz")
        hLaserReflMap = gROOT.FindObject("hLaserReflMap" + str(slot))
        hLaserReflMap.Scale(1. / tr.GetEntries())
        hLaserReflMap.SetStats(False)
        hLaserReflMap.SetTitle(root_output + (" - hit map for the whole timing region (hits/event)"
                                              if IsCosmicRayData else " - laser reflected hit occupancy (hits/event)"))

        # number of hits
        canvas.cd(6)
        gPad.SetFrameFillStyle(0)
        gPad.SetFillStyle(0)
        hNHitAll[slot].SetStats(False)
        hNHitAll[slot].SetTitle("number of hits for " + slotstr)
        hNHitAll[slot].SetLineColor(1)
        hNHitNoCalCh[slot].SetLineColor(2)
        hNHitAll[slot].Draw()
        hNHitNoCalCh[slot].Draw("same")
        latex.SetTextSize(0.05)
        latex.SetTextAlign(33)
        latex.DrawLatex(0.875, 0.785, "underflow : " + str(int(hNHitAll[slot].GetBinContent(0) * 1000 / nEntries) / 10) + "%")
        latex.DrawLatex(0.875, 0.685, "underflow : " + str(int(hNHitNoCalCh[slot].GetBinContent(0) * 1000 / nEntries) / 10) + "%")
        if PeakTime < 0:
            tr.Draw("Sum$(" + basicHitSelection + "&& " + slotCut + "&&TMath::Abs(time-refTime-(" +
                    str(PeakTime) + "))<" + str(FitWidth) + ")>>hNHitLaser" + str(slot), "", "same")
            hNHitLaser = gROOT.FindObject("hNHitLaser" + str(slot))
            hNHitLaser.SetFillStyle(0)
            hNHitLaser.SetLineColor(3)
            nHitMeanLaser[slot - 1] = hNHitLaser.GetMean()
            hMax = max(hNHitLaser.GetBinContent(hNHitLaser.GetMaximumBin()),
                       hNHitAll[slot].GetBinContent(hNHitAll[slot].GetMaximumBin()),
                       hNHitNoCalCh[slot].GetBinContent(hNHitNoCalCh[slot].GetMaximumBin()))
            hNHitAll[slot].GetYaxis().SetRangeUser(0, 1.1 * hMax)
            if not LaserAdded:
                legstr = "Laser direct hit (" + ("single hit" if IsOfflineFEDisabled else "multi hit") + ")"
                legendSlot.AddEntry(hNHitLaser, legstr)
                legendSlot.SetY1(0.55)
                LaserAdded = True
        else:
            hMax = max(hNHitAll[slot].GetBinContent(hNHitAll[slot].GetMaximumBin()),
                       hNHitNoCalCh[slot].GetBinContent(hNHitNoCalCh[slot].GetMaximumBin()))
            hNHitAll[slot].GetYaxis().SetRangeUser(0, 1.1 * hMax)
        legendSlot.Draw()

        # time indterval of the double calibration pulses (when offline FE is available)
        canvas.cd(7)
        gPad.SetFrameFillStyle(0)
        gPad.SetFillStyle(0)
        if IsOfflineFEDisabled or IsCosmicRayData:
            hRawTime = hRawTimeList[slot]
            hRawTime.SetStats(False)
            hRawTime.SetTitle("rawTime distribution")
            hRawTime.GetXaxis().SetTitle("rawTime")
            ytitle = "Entries [/(" + str(hRawTime.GetXaxis().GetBinUpEdge(1) - hRawTime.GetXaxis().GetBinLowEdge(1)) + " samples)]"
            hRawTime.GetYaxis().SetTitle(ytitle)
            hRawTime.SetTitle("rawTime distribution : " + slotstr)
            hRawTime.Draw()
        else:
            tr.Draw("time-refTime:TMath::FloorNint((pixelId-1)/8)>>hCalPulseInterval" + str(slot) + "(64,-0.5,63.5,100,19,24)",
                    # tr.Draw("time-refTime:TMath::FloorNint((pixelId-1)/8)>>hCalPulseInterval"
                    # + str(slot) + "(64,-0.5,63.5,100,7.5,32.5)",
                    "hitQuality>=200 && " + slotCut, "colz")
            hCalPulseInterval = gROOT.FindObject("hCalPulseInterval" + str(slot))
            hCalPulseInterval.GetXaxis().SetTitle("asic number = (pixelId-1)/8")
            hCalPulseInterval.GetYaxis().SetTitle("time interval of double cal. pulses [ns]")
            hCalPulseInterval.SetTitle("time interval of double calibration pulses : " + slotstr)
            hCalPulseInterval.SetStats(False)

        canvas.cd(8)
        gPad.SetFrameFillStyle(0)
        gPad.SetFillStyle(0)
        gPad.SetLogy()
        tr.Draw("sample>>hSample" + str(slot) + "(256,-0.5,255.5)",
                "slotNum==" + str(slot) + ("&&hitQuality>0&&hitQuality%10!=4" if IsCosmicRayData
                                           else "&&hitQuality>=100&&hitQuality<200"))
        hSample = gROOT.FindObject("hSample" + str(slot))
        hSample.SetFillStyle(0)
        hSample.SetLineColor(1)
        hSample.SetStats(False)
        hSample.SetTitle("entries of each sample# for TBC : " + slotstr)
        hSample.GetXaxis().SetTitle("Sample# for each hit" if IsCosmicRayData
                                    else "Sample# of the first calibration pulse")
        line.DrawLine(
            hSample.GetXaxis().GetBinLowEdge(1),
            nEntries / 4,
            hSample.GetXaxis().GetBinUpEdge(
                hSample.GetXaxis().GetNbins()),
            nEntries / 4)

        canvas.Print(pdf_output)

    canvas.Print((pdf_output + "]"))

    print("      (nEvt >=3hits) (mean nhit) (no cal.ch) (nCalHit) (nCalHit wfm) (nHitLaser)")
    for iSlot in range(0, 16):
        print("slot" + "{0:02d}".format(iSlot + 1) +
              "  " + "{0:7d}".format(int(nSlotEntriesAll[iSlot])) + " \t " + str('%03.2f' % nHitMeanAll[iSlot]) +
              " \t     " + str('%03.2f' % nHitMeanNoCalCh[iSlot]) + " \t" + str('%03.2f' % nSlotEntriesCalCh[iSlot]) +
              " \t  " + str('%03.2f' % nSlotEntriesCalChWfm[iSlot]) + " \t " + str('%03.2f' % nHitMeanLaser[iSlot]))

    print("nEntries = " + str(nEntries))
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
    canvas = TCanvas("cv2", "cv2", 1600, 1600)
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
    hTimeVSWin.GetYaxis().SetTitle("hit time - ref. time [ns]")
    hTimeVSWin.SetStats(False)
    hTimeVSWin.SetTitle(root_output + " hit time - ref time  VS window number.")

    canvas.cd(2)
    gPad.SetFrameFillStyle(0)
    gPad.SetFillStyle(0)
    tr.Draw("rawTdc-refTdc:sample>>hTimeVSSample(2560, 0, 256, 60, -160, -130)", "refTdc>0 && !isCalCh", "colz")
    hTimeVSSample = gROOT.FindObject("hTimeVSSample")
    hTimeVSSample.SetStats(False)
    hTimeVSSample.GetXaxis().SetTitle("hit sample [sample]")
    hTimeVSSample.GetYaxis().SetTitle("hit time - ref. time [ns]")
    hTimeVSSample.SetTitle("hit time - ref time VS hit time.")
    gPad.Update()

    canvas.cd(3)
    gPad.SetFrameFillStyle(0)
    gPad.SetFillStyle(0)
    tr.Draw("rawTdc-refTdc:pixelId>>hTimeVSPixel(512, 0, 512, 60, -160, -130 )", "refTdc>0 && !isCalCh", "colz")
    hTimeVSPixel = gROOT.FindObject("hTimeVSPixel")
    hTimeVSPixel.SetStats(False)
    hTimeVSPixel.GetXaxis().SetTitle("pixel ID")
    hTimeVSPixel.GetYaxis().SetTitle("hit time - ref. time [ns]")
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
    print("mean nHit value : " + str(hHitEff.GetMean()))
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
        # plotInterimFEDataNtupleAdvanced(args[1])
    else:
        print("usage:")
        print(args[0] + " (output of plotInterimFEData)")
