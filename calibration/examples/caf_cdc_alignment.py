from basf2 import *
set_log_level(LogLevel.INFO)

import os
import sys

import ROOT
from ROOT import Belle2

from caf.framework import Calibration, CAF
from caf import backends


def main(argv):
    print('Demonstrator of Millepede alignment of CDC layers')
    print('Usage basf2 SCRIPT_NAME DST_FILE.root [DST_FILE2.root ... ]')
    # Create a CAF instance to configure how we will run

    inputFiles = []
    for file in argv[0:]:
        inputFiles.append(os.path.abspath(file))

    if not len(inputFiles):
        print('Usage basf2 SCRIPT_NAME DST_FILE.root [DST_FILE2.root ... ]')
        sys.exit(1)
    print(inputFiles)

    from alignment import MillepedeCalibration

    millepede = MillepedeCalibration(['CDCAlignment', 'CDCLayerAlignment'])
    millepede.algo.invertSign()

    millepede.fixCDCLayerX(49)
    millepede.fixCDCLayerY(49)
    millepede.fixCDCLayerRot(49)
    millepede.fixCDCLayerX(55)
    millepede.fixCDCLayerY(55)
    millepede.fixCDCLayerRot(55)

    from caf.framework import Calibration, CAF

    caf = CAF(max_iterations=3)

    caf.add_calibration(millepede.create('cdc_layer_alignment', inputFiles))
    caf.output_dir = 'caf_output'

    caf.run()

    print("Finished CAF Processing")

if __name__ == "__main__":
    # Pass arguments after script name as input files
    main(sys.argv[1:])
