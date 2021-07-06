#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import sys


from caf.framework import CAF


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
    millepede.set_components(['VXDAlignment', 'BeamSpot'])

    caf.add_calibration(millepede.create('vxd_shells_beamspot', inputFiles))

    caf.output_dir = 'caf_output'
    caf.run()

    print("Finished CAF Processing")


if __name__ == "__main__":
    # Pass arguments after script name as input files
    main(sys.argv[1:])
