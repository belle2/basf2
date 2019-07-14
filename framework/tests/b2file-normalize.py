#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import tempfile
import subprocess
import ROOT

testFile = tempfile.NamedTemporaryFile()
fileName = testFile.name

rootfile = ROOT.TFile.Open(fileName, 'RECREATE')
hist = ROOT.TH1F('hist', 'TextHistogram', 100, 0, 1)
for i in range(1000):
    hist.Fill(ROOT.gRandom.Uniform())
hist.Write()
oldHist = hist.Clone()
oldHist.SetDirectory(0)
del(rootfile)

subprocess.run(['b2file-normalize', '-i', '-n', 'NormalizedRootFile', fileName], check=True)

newRootFile = ROOT.TFile.Open(fileName)
newHist = newRootFile.Get('hist')
assert 0 == newHist.Chi2Test(oldHist, 'PCHI2')

del(testFile)
