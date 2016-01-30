import ROOT
import sys
import os.path as path
ROOT.gROOT.SetBatch()
ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetOptFit(1)

infile = ROOT.TFile.Open(sys.argv[1])
outname = (path.basename(sys.argv[1])).replace('.root', '_fitWave.png')

tree = infile.Get("tree")
h1 = ROOT.TH1F("h1", ";ADC value;Sample Count", 600, -300, 300)

c = ROOT.TCanvas()
c.SetLogy()
tree.Draw('EventWaveformPackets.v_samples>>h1', 'm_flag<100')
h1.Fit('gaus')
h1.Draw()
c.SaveAs(outname)
