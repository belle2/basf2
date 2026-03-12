#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#   eclWriteClusteringParameters creates an eclClusteringParameters payload
#   from the values specified in the steering file.
#
#  usage:
#   basf2 eclWriteClusteringParameters.py
#
#   where the parameters are:
#   LM_energy_cut [ECLLocalMaximumFinder]: Seed energy cut [GeV]
#   CRF_energy_cut0 [ECLCRFinder]: seed energy cut [GeV]
#   CRF_energy_cut1 [ECLCRFinder]: growth energy cut [GeV]
#   CRF_energy_cut2 [ECLCRFinder]: digit energy cut [GeV]
#   CRF_time_cut0 [ECLCRFinder]: seed time cut [ns]
#   CRF_time_cut1 [ECLCRFinder]: growth time cut [ns]
#   CRF_time_cut2 [ECLCRFinder]: digit time cut [ns]
#   CRF_timeCut_maxEnergy0 [ECLCRFinder]: max energy for seed time cut [GeV]
#   CRF_timeCut_maxEnergy1 [ECLCRFinder]: max energy for growth time cut [GeV]
#   CRF_timeCut_maxEnergy2 [ECLCRFinder]: max energy for digit time cut [GeV]

from ROOT import Belle2  # noqa: make Belle2 namespace available
from ROOT.Belle2 import ECLClusteringParameters, Database, IntervalOfValidity
import array

parameters = ECLClusteringParameters()
parameters.setLMEnergyCut(0.020)

CRF_energy_cut = array.array('d', [0.020, 0.020, 0.0005])
parameters.setCRFEnergyCut(CRF_energy_cut)

CRF_time_cut = array.array('d', [400.0, 400.0, 99999.0])
parameters.setCRFTimeCut(CRF_time_cut)

CRF_timeCut_maxEnergy = array.array('d', [99999.0, 99999.0, 0.0])
parameters.setCRFTimeCutMaxEnergy(CRF_timeCut_maxEnergy)

database = Database.Instance()

iov = IntervalOfValidity(0, 0, -1, -1)
database.storeData('ECLClusteringParameters', parameters, iov)
