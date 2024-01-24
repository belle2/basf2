#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import sys
import glob
from ROOT import Belle2

# Usage: basf2 run_ee5x5_algorithm_crystalEnergy.py "inputfiles*.root"
# or: bsub -q s -o logfile.txt 'basf2 run_ee5x5_algorithm_crystalEnergy.py "inputfiles*.root"'

# Run just the algorithm part of the eclee5x5 calibration, which calibrates the single crystal
# energy response using Bhabha events. Runs on the output produced by the collector stage.

# This scripts sets parameters to find payload ECLCrystalEnergy5x5 using data. Other scripts
# are available to obtain payloads "ECLExpee5x5E", "ECLeedPhiData", or "ECLeedPhiMC", or to
# just merge the input histograms and write them to the specified output file.

# algo.setStoreConst = True: store the specified payload, if calculation is successful.
# algo.setStoreConst = False: do not write to the database

# specified output file includes diagnostic histograms.

algo = Belle2.ECL.eclee5x5Algorithm()
fileNames = ['eclee5x5CollectorOutput.root']
narg = len(sys.argv)
if(narg == 2):
    fileNames = glob.glob(sys.argv[1])
print("input fileNames = ", fileNames)
algo.setInputFileNames(fileNames)
algo.setOutputName("eclee5x5_algorithm_crystalEnergy.root")
algo.setMinEntries(150)
algo.setPayloadName("ECLCrystalEnergy5x5")
algo.setStoreConst(True)


# Run algorithm after merging all input files, as opposed to one run at a time
alg_result = algo.execute()
print("result of eclee5x5Algorithm was", alg_result)

# Commits a successful list of dbobjects to localdb/
algo.commit()
