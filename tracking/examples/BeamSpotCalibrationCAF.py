#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#############################################################
# The CAF script for the BeamSpot calibration
#############################################################

import basf2

import os
import sys
import multiprocessing

from ROOT import Belle2
from ROOT.Belle2 import BeamSpotAlgorithm

from caf.framework import CAF, Calibration, CentralDatabase
from caf import backends
from caf import strategies

import modularAnalysis as ana

basf2.set_log_level(basf2.LogLevel.INFO)


def BeamSpotCalibration(files, tags):
    """
    Function to get the BeamSpot calibration object.
    Takes list of files and list of GTs as arguments.
    """

    path = basf2.create_path()
    path.add_module('Progress')
    path.add_module('RootInput')

    # Select mumu decays with enough hits
    muSelection = '[p>1.0]'
    muSelection += ' and abs(dz)<2.0 and abs(dr)<0.5'
    muSelection += ' and nPXDHits >=1 and nSVDHits >= 8 and nCDCHits >= 20'
    ana.fillParticleList('mu+:BS', muSelection, path=path)
    ana.reconstructDecay('Upsilon(4S):BS -> mu+:BS mu-:BS', '9.5<M<11.5', path=path)

    # Init the BeamSpot collector and algo
    collector = basf2.register_module('BeamSpotCollector', Y4SPListName='Upsilon(4S):BS')
    algorithm = BeamSpotAlgorithm()

    calibration = Calibration('BeamSpot',
                              collector=collector,
                              algorithms=algorithm,
                              input_files=files,
                              pre_collector_path=path,
                              database_chain=[CentralDatabase(tag) for tag in tags],
                              output_patterns=None,
                              max_files_per_collector_job=1,
                              backend_args=None
                              )

    # The segmentation is done in the algorithm
    calibration.strategies = strategies.SingleIOV

    return calibration


if __name__ == "__main__":
    input_files = [os.path.abspath(file) for file in Belle2.Environment.Instance().getInputFilesOverride()]

    if not len(input_files):
        print("You have to specify some input file(s) (raw data or di - muon skim with raw objects)\n"
              "using the standard basf2 command line option - i")
        print("See: basf2 -h")
        sys.exit(1)

    beamspot = BeamSpotCalibration(input_files, ['data_reprocessing_prompt', 'online_bucket9'])

    cal_fw = CAF()
    cal_fw.add_calibration(beamspot)
    cal_fw.backend = backends.LSF()

    # Try to guess if we are at KEKCC and change the backend to Local if not
    if multiprocessing.cpu_count() < 10:
        cal_fw.backend = backends.Local(8)

    cal_fw.run()
