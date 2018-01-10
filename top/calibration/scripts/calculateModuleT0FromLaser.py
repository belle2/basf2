#!/usr/bin/env python
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# Script to exctract an approximate ModuleT0 constant using the laser data.
# The input of the file is the root file contining the tree with all the
# localT0 constants produced by  the TOPLaserCalibratorModule
# Usage: basf2 checkCalibrationOnCalpulseRun.py  <reference slot (1-16)>
#        <reference channel (0-511)>  <input_file.root>  <output_file.root>
#
# Contributors: Umberto Tamponi (tamponi@to.infn.it)
#
# ---------------------------------------------------------------------------------------

from basf2 import *
from ROOT import Belle2, TTree, TFile
import sys
import math
from array import array

refSlot = int(sys.argv[1]) - 1  # 1-based to 0-based
refChan = int(sys.argv[2])

laserT0 = [0.] * 16
laserT0Err = [0.] * 16

inputFile = TFile(str(sys.argv[3]))
inputTree = inputFile.Get('chT0')

for entry in inputTree:
    if(int(entry.channel) == refChan):
        laserT0[int(entry.slot) - 1] = float(entry.t0Const)
        laserT0Err[int(entry.slot) - 1] = 0.  # placeholder, waiting for the errors to be stored in the tree

inputFile.Close()


outFile = TFile(str(sys.argv[4]), 'recreate')
outTree = TTree('moduleT0laser', 'Module T0 constants using the laser')

moduleT0LaserConst = array('f', [0.])
moduleT0LaserErr = array('f', [0.])

outTree.Branch('moduleT0LaserConst', moduleT0LaserConst, ' moduleT0LaserConst/F')
outTree.Branch('moduleT0LaserErr', moduleT0LaserErr, ' moduleT0LaserErr/F')

print('------------------------------------------------------------------')
print('Results of TOP module-by-module sychronization usin the Laser data')
print('                                                                  ')
for iSlot in range(0, 16):
    moduleT0LaserConst = laserT0[iSlot] - laserT0[refSlot]
    moduleT0LaserErr = math.sqrt(laserT0Err[iSlot] * laserT0Err[iSlot] + laserT0Err[refSlot] * laserT0Err[refSlot])
    print('Slot ' + str(iSlot + 1) + ':  constant = ' + str(moduleT0LaserConst) + ' ns;  Error = ' + str(moduleT0LaserErr) + ' ns')
    outTree.Fill()
print('                                                                  ')
print('------------------------------------------------------------------')

outTree.Write()
outFile.Close()
