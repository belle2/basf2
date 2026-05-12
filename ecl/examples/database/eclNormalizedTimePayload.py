#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#   eclNormalizedTimePayload.py creates an ECLTimingNormalization payload
#   (analysis/dbobjects/include/ECLTimingNormalization.h) from histograms
#   read from disk. Except for minimum normalization time, which is
#   hardwired.
#
#  usage:
#   basf2 eclNormalizedTimePayload.py

from ROOT import Belle2  # noqa: make Belle2 namespace available
from ROOT.Belle2 import ECLTimingNormalization, Database, IntervalOfValidity
from ROOT import TFile
import numpy as np

# ..Job parameters: input file name, and output payload name
# inputName = "tNormalized_data.root"
# payloadName = "ECLTimingNormalization_data"
# tNormalizedMin = 3.0 # ns, for data

inputName = "tNormalized_MC.root"
payloadName = "ECLTimingNormalization_MC"
tNormalizedMin = 1.6  # ns, for mc


# ..Preliminaries
inputFile = TFile(inputName, "READ")
parameters = ECLTimingNormalization()
nCrystals = 8736


# --------------------------------------------------
# ..Time walk
timeWalkHistogram = inputFile.Get("timeWalkParameters")
nParHist = timeWalkHistogram.GetNbinsY()
nParDBObject = 7  # dimension of array in dbobject
if nParHist != nParDBObject:
    print(f'ERROR in time walk: nParHist = {nParHist}, nParDBObject = {nParDBObject}')

timeWalk2DArray = np.empty((nCrystals, nParDBObject))
for cellID in range(1, nCrystals + 1):
    for ybin in range(1, nParHist + 1):
        value = timeWalkHistogram.GetBinContent(cellID, ybin)
        timeWalk2DArray[cellID-1, ybin-1] = value

parameters.setTimeWalkPar(timeWalk2DArray)


# --------------------------------------------------
# ..Background level dependence
backgroundHistogram = inputFile.Get("backgroundParameters")
nParHist = backgroundHistogram.GetNbinsY()
nParDBObject = 5  # dimension of array in dbobject
if nParHist != nParDBObject:
    print(f'ERROR in background: nParHist = {nParHist}, nParDBObject = {nParDBObject}')

background2DArray = np.empty((nCrystals, nParDBObject))
for cellID in range(1, nCrystals + 1):
    for ybin in range(1, nParHist + 1):
        value = backgroundHistogram.GetBinContent(cellID, ybin)
        background2DArray[cellID-1, ybin-1] = value

parameters.setBackgroundPar(background2DArray)


# --------------------------------------------------
# ..Energy dependence
energyHistogram = inputFile.Get("energyParameters")
nParHist = energyHistogram.GetNbinsY()
nParDBObject = 7  # dimension of array in dbobject
if nParHist != nParDBObject:
    print(f'ERROR in enegy: nParHist = {nParHist}, nParDBObject = {nParDBObject}')

energy2DArray = np.empty((nCrystals, nParDBObject))
for cellID in range(1, nCrystals + 1):
    for ybin in range(1, nParHist + 1):
        value = energyHistogram.GetBinContent(cellID, ybin)
        energy2DArray[cellID-1, ybin-1] = value

parameters.setEnergyPar(energy2DArray)


# --------------------------------------------------
# ..Minimum normalization time
parameters.setMinTNormalization(tNormalizedMin)


# --------------------------------------------------
# ..Create the payload
inputFile.Close()
database = Database.Instance()

iov = IntervalOfValidity(0, 0, -1, -1)
database.storeData(payloadName, parameters, iov)
print(f'Created ECLTimingNormalization payload with name {payloadName}')
