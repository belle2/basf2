#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2

import os
import sys
import ROOT

from ROOT import Belle2

from caf import backends
from caf.framework import Calibration, CAF
from alignment import MillepedeCalibration

import reconstruction as reco
import modularAnalysis as ana


def main(argv):
    inputFiles = []
    for file in argv[0:]:
        inputFiles.append(os.path.abspath(file))
    if not len(inputFiles):
        print(' Usage: basf2 SCRIPT_NAME DST_FILE.root [DST_FILE2.root ... ]')
        sys.exit(1)

    # The Calibration and Alignment Framework
    caf = CAF()

    # predefined scenarios
    from alignment import setups
    # run std reco and select alignment particles/decays
    setups.do_reconstruction = True
    setups.do_analysis = True

    millepede = setups.setup_VXDHalfShells()

    # Align primary beamspot simultaneously with VXD half shells in addition
    millepede.set_components(['VXDAlignment', 'BeamVertex'])

    caf.add_calibration(millepede.create('vxd_shells_beamspot', inputFiles))

    caf.output_dir = 'caf_output'
    caf.run()

    print("Finished CAF Processing")


if __name__ == "__main__":
    # Pass arguments after script name as input files
    main(sys.argv[1:])
