import ROOT
ROOT.gStyle.SetOptStat(0)
ROOT.gROOT.SetBatch()
import sys

if len(sys.argv) < 2:
    print("Usage: %s <name of root ntuple>" % sys.argv[0])
    sys.exit(-1)

f = ROOT.TFile.Open(sys.argv[1])

t = f.Get("feProps")
t.SetLineWidth(2)
t.SetLineColor(6)
calChannel = "ch % 8 == 0"
signalChannel = "ch % 8 != 0"
c = ROOT.TCanvas()

t.Draw("pyPeak0Ht:fePeakHt", calChannel, "hist")
c.SaveAs("pyPeak0Ht_vs_fePeakHt_cal.pdf")
c.Clear()

t.Draw("pyPeak0Ht:fePeakHt", signalChannel, "hist")
c.SaveAs("pyPeak0Ht_vs_fePeakHt_sig.pdf")
c.Clear()

t.Draw("pyPeak0Ht:pyPeak1Ht", calChannel, "hist")
c.SaveAs("pyPeak0Ht_vs_pyPeak1Ht.pdf")
c.Clear()

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
t.Draw("pyPeak0Ht", calChannel, "same")
adcHist.SetTitle("ADC counts in the calibration channel")
c.BuildLegend()
c.SaveAs("ADC_distribution_peak0_cal.pdf")
c.Clear()
adcHist.Reset()

adcHist.SetLineWidth(2)
adcHist.SetLineColor(4)
t.Draw("fePeakHt>>adcHist", signalChannel)
t.Draw("pyPeak0Ht", signalChannel, "same")
adcHist.SetTitle("ADC counts in the signal channels")
c.BuildLegend()
c.SaveAs("ADC_distribution_peak0_sig.pdf")
c.Clear()

tdcHist = ROOT.TH1D("tdcHist", "tdcHist;TDC", 256, 1, 257)
tdcHist.SetLineWidth(2)
tdcHist.SetLineColor(4)
t.Draw("fePeakTDC>>tdcHist", calChannel)
t.Draw("pyPeak0TDC", calChannel, "same")
tdcHist.SetTitle("TDC distribution calibration channel")
c.BuildLegend()
c.SaveAs("TDC_distribution_peak0_cal.pdf")
c.Clear()
tdcHist.Reset()

tdcHist.SetLineWidth(2)
tdcHist.SetLineColor(4)
t.Draw("fePeakTDC>>tdcHist", signalChannel)
t.Draw("pyPeak0TDC", signalChannel, "same")
tdcHist.SetTitle("TDC distribution signal channels")
c.BuildLegend()
c.SaveAs("TDC_distribution_peak0_signal.pdf")
c.Clear()
tdcHist.Reset()

tdcHist.SetLineWidth(2)
tdcHist.SetLineColor(4)
t.Draw("pyPeak1TDC>>tdcHist", calChannel + "&& nPyPeaks > 2")
tdcHist.SetTitle("TDC distribution 2nd peak calibration channel")
c.Update()
c.SaveAs("pyPeak1TDC_calChannel.pdf")
c.BuildLegend()
c.Clear()
tdcHist.Reset()

tdcHist.SetLineWidth(2)
tdcHist.SetLineColor(4)
t.Draw("pyPeak1TDC>>tdcHist", signalChannel + "&& nPyPeaks > 1")
tdcHist.SetTitle("TDC distribution 2nd peak signal channel")
c.Update()
c.SaveAs("pyPeak1TDC_signalChannel.pdf")
c.BuildLegend()
c.Clear()
tdcHist.Reset()

t.SetLineColor(4)
t.Draw("nPyPeaks", calChannel)
c.SaveAs("nPeaks_cal.pdf")
c.BuildLegend()
c.Clear()

t.Draw("nPyPeaks", signalChannel)
c.SaveAs("nPeaks_signal.pdf")
c.BuildLegend()
c.Clear()

c.Divide(3, 3)
c.cd(9)
h0 = ROOT.TH1D("pyh0", "all Channels", 1000, 50, 1050)
t.Draw("pyPeak0Ht>>pyh0")
hists = []
for iCanv in range(1, 9):
    c.cd(iCanv)
    h = ROOT.TH1D("pyh%d" % iCanv, "channel %d" % iCanv, 1000, 50, 1050)
    t.Draw("pyPeak0Ht>>pyh%d" % iCanv, "ch %% 8 == %d" % (iCanv-1))
    hists.append(h)
c.SaveAs("pyPeak0Ht_byChannel.pdf")

hists = []
c.Clear()
c.Divide(3, 3)
c.cd(9)
h0 = ROOT.TH1D("feh0", "all Channels", 1000, 50, 1050)
t.Draw("fePeakHt>>feh0")
for iCanv in range(1, 9):
    c.cd(iCanv)
    h = ROOT.TH1D("feh%d" % iCanv, "channel %d" % iCanv, 1000, 50, 1050)
    t.Draw("fePeakHt>>feh%d" % iCanv, "ch %% 8 == %d" % (iCanv-1))
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
    h = ROOT.TH1D("fe_asic%d" % iCanv, "ASIC %d" % iASIC, 1000, 50, 1050)
    t.Draw("fePeakHt>>fe_asic%d" % iCanv, "(ch/8) %% 4 == %d" % (iASIC))
    hists.append(h)
c.SaveAs("fePeakHt_byASIC.pdf")

# Count the number of failure modes
total = t.GetEntries()

print("Fraction with width > 30: %.3f" % (1.0*t.Draw("fePeakWd", "fePeakWd>30", "goff")/total))
print("Fraction with peaks at window borders: (average occupancy / bin = %.3f)" % (1./256))
print("\ttdc < 2        : %.3f" % (1.0*t.Draw("fePeakWd", "fePeakTDC<2", "goff")/total))
print("\t63 < tdc < 66  : %.3f" % (1.0*t.Draw("fePeakWd", "fePeakTDC>63 && fePeakTDC<66", "goff")/total))
print("\t127 < tdc < 130: %.3f" % (1.0*t.Draw("fePeakWd", "fePeakTDC>127 && fePeakTDC<130", "goff")/total))
print("\t191 < tdc < 194: %.3f" % (1.0*t.Draw("fePeakWd", "fePeakTDC>191 && fePeakTDC<194", "goff")/total))
print("\ttdc > 255      : %.3f" % (1.0*t.Draw("fePeakWd", "fePeakTDC>255", "goff")/total))
