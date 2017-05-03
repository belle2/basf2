from basf2 import *
set_log_level(LogLevel.INFO)

import os
import sys

import ROOT
from ROOT import Belle2

from caf.framework import Calibration, CAF
from caf import backends

from alignment import MillepedeCalibration


def main(argv):
    print('Demonstrator of Millepede alignment for VXD Half-shells')
    print('Usage basf2 SCRIPT_NAME DST_FILE.root [DST_FILE2.root ... ]')
    # Create a CAF instance to configure how we will run
    caf = CAF(max_iterations=3)

    inputFiles = []
    for file in argv[0:]:
        inputFiles.append(os.path.abspath(file))

    if not len(inputFiles):
        print('Usage basf2 SCRIPT_NAME DST_FILE.root [DST_FILE2.root ... ]')
        sys.exit(1)
    print(inputFiles)

    millepede = MillepedeCalibration(['VXDAlignment'], tracks=[''])
    millepede.algo.invertSign()
    # Add the constraints (auto-generated from hierarchy), so you can
    # play with unfixing degrees of freedom below
    # millepede.algo.steering().command('Fortranfiles')
    # millepede.algo.steering().command('constraints.txt')

    """
        {{"PXD.Ying"}, {Belle2::VxdID(1, 0, 0, 1)}},
        {{"PXD.Yang"}, {Belle2::VxdID(1, 0, 0, 2)}},
        {{"SVD.Pat"}, {Belle2::VxdID(3, 0, 0, 1)}},
        {{"SVD.Mat"}, {Belle2::VxdID(3, 0, 0, 2)}}
    """
    # millepede.fixPXDYing()
    # millepede.fixPXDYang()
    millepede.fixSVDPat()
    # millepede.fixSVDMat()
    # Now fix everything (even some non-existing stuff:-) except half-shells
    for layer in range(1, 7):
        for ladder in range(1, 17):
            # Fix also all ladders
            millepede.fixVXDid(layer, ladder, 0)
            for sensor in range(1, 6):
                # Fix all sensors
                millepede.fixVXDid(layer, ladder, sensor)
                pass

    caf.add_calibration(millepede.create('vxd_halfshell_alignment', inputFiles))
    caf.output_dir = 'caf_output'
    # Start her up!
    caf.run()
    print("Finished CAF Processing")

if __name__ == "__main__":
    # Pass arguments after script name as input files
    main(sys.argv[1:])
