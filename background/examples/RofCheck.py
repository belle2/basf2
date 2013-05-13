#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
# This script checks component/generator names in a list of ROF files.
# Usage:
#    chmod a+x RofCheck.py && RofCheck.py my_rof_files/*.root
# Example script - 2013 Belle II Collaboration
##############################################################################
import sys
import glob
from basf2 import *
import ROOT
from ROOT import Belle2

subdetectorCodes = {1: 'PXD', 2: 'SVD', 3: 'CDC', 4: 'TOP', 5: 'ARICH',\
                    6: 'ECL', 7: 'EKLM', 8: 'BKLM', 9: 'ECLsim'}

n_files = len(sys.argv)
ids = set([])
for file_name in sys.argv[1:n_files]:
    f = ROOT.TFile(file_name, "READ")
    tree = f.Get('ContentTree')
    tree.Draw('Subdetector', '', 'goff')
    h = ROOT.gDirectory.Get('htemp')
    detector = subdetectorCodes[int(h.GetMean())]
    tree.Draw('Component:Generator', '', 'goff')
    h = ROOT.gDirectory.Get('htemp')
    component = h.GetYaxis().GetLabels().First().GetName()
    generator = h.GetXaxis().GetLabels().First().GetName()
    print file_name + ':' + '\tDetector: ' + detector + \
        '\tComponent: ' + component + '\tGenerator: ' + generator
    if (detector, component, generator) in ids:
        print 'The combination ' + detector + ' + ' + component + \
        ' + ' + generator + ' is duplicated!'
    else:
        ids.add((detector, component, generator))
    f.Close()
