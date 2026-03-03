#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#   eclCrystalCalibPayload.py creates an ECLCrystalCalib payload
#   (analysis/dbobjects/include/ECLCrystalCalib.h) from a histogram
#   read from disk.
#
#  usage:
#   basf2 eclCrystalCalibPayload.py

from ROOT import Belle2  # noqa: make Belle2 namespace available
from ROOT.Belle2 import ECLCrystalCalib, Database, IntervalOfValidity
from ROOT import TFile

# ..Job parameters: input file and histogram names, and output payload name
inputName = "ECLRefAmplNom_36_2290.root"
histName = "newPayload"
payloadName = "ECLRefAmplNom"

# ..Preliminaries
inputFile = TFile(inputName, "READ")
parameters = ECLCrystalCalib()
nCrystals = 8736


# --------------------------------------------------
# ..Read in histogram of payload values and convert into a vector
payloadHist = inputFile.Get(histName)
payloadVector = [0.] * nCrystals
payloadUncVector = [0.] * nCrystals
for cellID in range(1, nCrystals + 1):
    payloadVector[cellID-1] = payloadHist.GetBinContent(cellID)
    payloadUncVector[cellID-1] = payloadHist.GetBinError(cellID)

print(payloadVector)
print(payloadUncVector)
parameters.setCalibVector(payloadVector, payloadUncVector)


# --------------------------------------------------
# ..Create the payload
inputFile.Close()
database = Database.Instance()

iov = IntervalOfValidity(0, 0, -1, -1)
database.storeData(payloadName, parameters, iov)
print(f'Created ECLCrystalCalib payload with name {payloadName}')
