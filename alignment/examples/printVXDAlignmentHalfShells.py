#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import sys
import ROOT
from ROOT import Belle2
if len(sys.argv) < 2:
    sys.exit("No input .root file with VXDAlignment payload specified!")

inputroot = sys.argv[1]
file = ROOT.TFile(inputroot, "OPEN")
vxd = file.Get("VXDAlignment_CORRECTIONS")

errfile = ROOT.TFile(str(inputroot).replace('VXDAlignment_CORRECTIONS', 'VXDAlignment_ERRORS'), "OPEN")
errvxd = errfile.Get("VXDAlignment_ERRORS")

ying = Belle2.VxdID(1, 0, 0, 1)
yang = Belle2.VxdID(1, 0, 0, 2)
pat = Belle2.VxdID(3, 0, 0, 1)
mat = Belle2.VxdID(3, 0, 0, 2)

shells = [ying, yang, pat, mat]
shellnames = ['PXD Ying', 'PXD Yang', 'SVD Pat', 'SVD Mat']

params = [1, 2, 3, 4, 5, 6]
paramnames = ['x', 'y', 'z', 'alpha', 'beta', 'gamma']

for ishell, shell in enumerate(shells):
    print('Half-shell: ', shellnames[ishell])
    for ipar, param in enumerate(params):
        print(paramnames[ipar], ' = ', vxd.get(shell.getID(), param), ' +/- ', errvxd.get(shell.getID(), param))
