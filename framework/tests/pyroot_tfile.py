#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# This test catches a regression in PyRoot (in root 5.34/19+?, fixed in 5.34/23)
from basf2 import find_file
import ROOT

path = find_file('framework/tests/chaintest_2.root')
f = ROOT.TFile(path)
tree = f.Get('tree')

tree.GetEntry(0)
hits = tree.PXDSimHits
nentries = hits.GetEntriesFast()
print(nentries)
hit = hits[0]
hit.Dump()
