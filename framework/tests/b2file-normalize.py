#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

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

newRootFile = ROOT.TFile.Open(fileName)
newHist = newRootFile.Get('hist')
assert 0 == newHist.Chi2Test(oldHist, 'PCHI2')
del(newRootFile)

checksum = hashlib.md5()
checksum.update(open(fileName, 'rb').read())
print(checksum.digest().hex())
assert checksum.digest().hex() == '4b616ad813c35498f6f8aa09f2bcebda'
del(testFile)
