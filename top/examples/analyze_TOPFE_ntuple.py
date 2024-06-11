##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Script to analyze the output of TOPFE_qualityPlots.py

import sys
import ROOT
ROOT.gStyle.SetOptStat(0)
ROOT.gROOT.SetBatch()

if len(sys.argv) < 2:
    print(f"Usage: {sys.argv[0]} <name of root ntuple>")
    sys.exit(-1)

f = ROOT.TFile.Open(sys.argv[1])

t = f.Get("feProps")
t.SetLineWidth(2)
t.SetLineColor(6)
calChannel = "ch % 8 == 0"
signalChannel = "ch % 8 != 0"
c = ROOT.TCanvas()

nTRDHist = ROOT.TH1D("nTRDHist", "all channels;number of TOPRawDigits;waveforms", 1000, 0, 30)
t.Draw("nTOPRawDigits>>nTRDHist")
c.SaveAs("nTOPRawDigits_All.pdf")
c.SetLogy()
c.SaveAs("nTOPRawDigits_All_logy.pdf")
c.SetLogy(False)
c.Clear()
nTRDHist.Reset()

t.Draw("nTOPRawDigits>>nTRDHist", calChannel)
c.SaveAs("nTOPRawDigits_cal.pdf")
c.SetLogy()
c.SaveAs("nTOPRawDigits_cal_logy.pdf")
c.SetLogy(False)
c.Clear()
nTRDHist.Reset()

t.Draw("nTOPRawDigits>>nTRDHist", signalChannel)
c.SaveAs("nTOPRawDigits_sig.pdf")
c.SetLogy()
c.SaveAs("nTOPRawDigits_sig_logy.pdf")
c.SetLogy(False)
c.Clear()

t.Draw("fePeakHt:fePeak1Ht", calChannel, "hist")
c.SaveAs("fePeakHt_vs_fePeak1Ht_cal.pdf")
c.Clear()

wd_adcHist = ROOT.TH2D("wd_adcHist", "Width vs Pulse height;width;height", 130, 0, 130, 1100, 50, 1150)
t.Draw("fePeakHt:fePeakWd>>wd_adcHist", "", "colz")
c.SaveAs("fePeakWd_vs_fePeakHt.pdf")
c.Clear()
wd_adcHist.Reset()

wd_adcHist.SetTitle("Width vs Pulse height in the calibration channel;width;height")
t.Draw("fePeakHt:fePeakWd>>wd_adcHist", calChannel, "colz")
c.SaveAs("fePeakWd_vs_fePeakHt_cal.pdf")
c.Clear()
wd_adcHist.Reset()

wd_adcHist.SetTitle("Width vs Pulse height in the signal channel;width;height")
t.Draw("fePeakHt:fePeakWd>>wd_adcHist", signalChannel, "colz")
c.SaveAs("fePeakWd_vs_fePeakHt_sig.pdf")
c.Clear()
wd_adcHist.Reset()

wd_tdcHist = ROOT.TH2D("wd_tdcHist", "Width vs TDC;TDC;width", 256, 1, 257, 130, 0, 130)
t.Draw("fePeakWd:fePeakTDC>>wd_tdcHist", "", "colz")
c.SaveAs("fePeakWd_vs_TDC.pdf")
c.Clear()

ht_tdcHist = ROOT.TH2D("ht_tdcHist", "Height vs TDC;TDC;height", 256, 1, 257, 1100, 50, 1150)
t.Draw("fePeakHt:fePeakTDC>>ht_tdcHist", "", "colz")
c.SaveAs("fePeakHt_vs_TDC.pdf")
c.SetLogz()
c.Clear()

ch_tdcHist = ROOT.TH2D("ch_tdcHist", "Channel vs TDC;TDC;channel number", 256, 1, 257, 512, 0, 512)
t.Draw("ch:fePeakTDC>>ch_tdcHist", "", "colz")
c.SetLogz()
c.SaveAs("fePeakTDC_vs_ch.pdf")
c.Clear()
#
adcHist = ROOT.TH1D("adcHist", "ADC counts;ADC", 1000, 0, 1000)
adcHist.SetLineWidth(2)
adcHist.SetLineColor(4)
t.Draw("fePeakHt>>adcHist", calChannel)
adcHist.SetTitle("ADC counts in the calibration channel")
c.SaveAs("ADC_distribution_peak0_cal.pdf")
c.Clear()
adcHist.Reset()

adcHist.SetLineWidth(2)
adcHist.SetLineColor(4)
t.Draw("fePeakHt>>adcHist", signalChannel)
adcHist.SetTitle("ADC counts in the signal channels")
c.SaveAs("ADC_distribution_peak0_sig.pdf")
c.Clear()

tdcHist = ROOT.TH1D("tdcHist", "tdcHist;TDC;waveforms", 256, 1, 257)
tdcHist.SetLineWidth(2)
tdcHist.SetLineColor(4)
t.Draw("fePeakTDC>>tdcHist", calChannel)
tdcHist.SetTitle("TDC distribution calibration channel")
c.SaveAs("TDC_distribution_peak0_cal.pdf")
c.Clear()
tdcHist.Reset()

tdcHist.SetLineWidth(2)
tdcHist.SetLineColor(4)
t.Draw("fePeakTDC>>tdcHist", signalChannel)
tdcHist.SetTitle("TDC distribution signal channels")
c.SaveAs("TDC_distribution_peak0_signal.pdf")
c.Clear()
tdcHist.Reset()

tdcHist.SetLineWidth(2)
tdcHist.SetLineColor(4)
t.Draw("fePeak1TDC>>tdcHist", calChannel + "&& nTOPRawDigits > 2")
tdcHist.SetTitle("TDC distribution 2nd peak calibration channel")
c.Update()
c.SaveAs("fePeak1TDC_calChannel.pdf")
c.BuildLegend()
c.Clear()
tdcHist.Reset()

tdcHist.SetLineWidth(2)
tdcHist.SetLineColor(4)
t.Draw("fePeak1TDC>>tdcHist", signalChannel + "&& nTOPRawDigits > 1")
tdcHist.SetTitle("TDC distribution 2nd peak signal channel")
c.Update()
c.SaveAs("fePeak1TDC_signalChannel.pdf")
c.BuildLegend()
c.Clear()
tdcHist.Reset()

hists = []
c.Clear()
c.Divide(3, 3)
c.cd(9)
h0 = ROOT.TH1D("feh0", "all Channels", 1000, 50, 1050)
t.Draw("fePeakHt>>feh0")
for iCanv in range(1, 9):
    c.cd(iCanv)
    h = ROOT.TH1D(f"feh{int(iCanv)}", f"channel {int(iCanv)};ADC;waveforms", 1000, 50, 1050)
    t.Draw(f"fePeakHt>>feh{int(iCanv)}", f"ch % 8 == {int(iCanv - 1)}")
    hists.append(h)
c.SaveAs("fePeakHt_byChannel.pdf")

hists = []
c.Clear()
c.Divide(3, 3)
c.cd(5)
h0 = ROOT.TH1D("fe_asic0", "all ASICs", 1000, 50, 1050)
t.Draw("fePeakHt>>fe_asic0")
for iASIC, iCanv in enumerate([1, 3, 7, 9]):
    c.cd(iCanv)
    h = ROOT.TH1D(f"fe_asic{int(iCanv)}", f"ASIC {int(iASIC)};ADC;waveforms", 1000, 50, 1050)
    t.Draw(f"fePeakHt>>fe_asic{int(iCanv)}", f"(ch/8) % 4 == {int(iASIC)}")
    hists.append(h)
c.SaveAs("fePeakHt_byASIC.pdf")

hists = []
c.Clear()
c.Divide(3, 3)
c.cd(5)
h0 = ROOT.TH1D("fe_carrier0", "all Carriers", 1000, 50, 1050)
t.Draw("fePeakHt>>fe_carrier0")
for iCarrier, iCanv in enumerate([1, 3, 7, 9]):
    c.cd(iCanv)
    h = ROOT.TH1D(f"fe_carrier{int(iCanv)}", f"Carrier {int(iCarrier)};ADC;waveforms", 1000, 50, 1050)
    t.Draw(f"fePeakHt>>fe_carrier{int(iCanv)}", f"(ch/8/4) % 4 == {int(iCarrier)}")
    hists.append(h)
c.SaveAs("fePeakHt_byCarrier.pdf")

hists = []
c.Clear()
c.Divide(3, 3)
c.cd(5)
h0 = ROOT.TH1D("fe_boardstack0", "all board stacks", 1000, 50, 1050)
t.Draw("fePeakHt>>fe_boardstack0")
for iBS, iCanv in enumerate([1, 3, 7, 9]):
    c.cd(iCanv)
    h = ROOT.TH1D(f"fe_boardstack{int(iCanv)}", f"board stack {int(iBS)};ADC;waveforms", 1000, 50, 1050)
    t.Draw(f"fePeakHt>>fe_boardstack{int(iCanv)}", f"(ch/8/4/4) % 4 == {int(iBS)}")
    hists.append(h)
c.SaveAs("fePeakHt_byBoardStack.pdf")

# Count the number of failure modes
total = t.GetEntries()

print(f"Fraction with width > 30: {1.0 * t.Draw('fePeakWd', 'fePeakWd>30', 'goff') / total:.3f}")
print(f"Fraction with peaks at window borders: (average occupancy / bin = {1.0 / 256:.3f})")
print(f"\ttdc < 2        : {1.0 * t.Draw('fePeakWd', 'fePeakTDC<2', 'goff') / total:.3f}")
print(f"\t63 < tdc < 66  : {1.0 * t.Draw('fePeakWd', 'fePeakTDC>63 && fePeakTDC<66', 'goff') / total:.3f}")
print(f"\t127 < tdc < 130: {1.0 * t.Draw('fePeakWd', 'fePeakTDC>127 && fePeakTDC<130', 'goff') / total:.3f}")
print(f"\t191 < tdc < 194: {1.0 * t.Draw('fePeakWd', 'fePeakTDC>191 && fePeakTDC<194', 'goff') / total:.3f}")
print(f"\ttdc > 255      : {1.0 * t.Draw('fePeakWd', 'fePeakTDC>255', 'goff') / total:.3f}")
