#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import tempfile
import subprocess
import ROOT
import hashlib

testFile = tempfile.NamedTemporaryFile()
fileName = testFile.name

rootfile = ROOT.TFile.Open(fileName, 'RECREATE')
hist = ROOT.TH1F('hist', 'TextHistogram', 100, 0, 1)
ROOT.gRandom.SetSeed(42)
for i in range(1000):
    hist.Fill(ROOT.gRandom.Uniform())
hist.Write()
oldHist = hist.Clone()
oldHist.SetDirectory(0)
del(rootfile)

subprocess.run(['b2file-normalize', '-i', '-n', 'NormalizedRootFile', '-r', '61408', fileName], check=True)

checksum = hashlib.md5()
checksum.update(open(fileName, 'rb').read())
assert checksum.digest().hex() == 'b35738dbe1f8d07edd437007d379ba2c'

del(testFile)
