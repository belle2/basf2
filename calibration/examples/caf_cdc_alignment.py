from basf2 import *
set_log_level(LogLevel.INFO)

import os
import sys

import ROOT
from ROOT import Belle2

from caf.framework import Calibration, CAF
from caf import backends

import basf2
from reconstruction import add_cosmics_reconstruction
from ROOT import Belle2
import cdc.cr as gcr

from alignment import MillepedeCalibration
from alignment import setups


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

    main_path = basf2.create_path()
    main_path.add_module('Progress')
    main_path.add_module('Gearbox')
    main_path.add_module('Geometry', excludedComponents=['SVD', 'PXD', 'ARICH', 'BeamPipe', 'EKLM'])
    # Add CDC CR reconstruction.
    add_cosmics_reconstruction(main_path, components=['CDC'], merge_tracks=True, pruneTracks=False)

    setups.dirty_data = True
    millepede = setups.setup_CDCLayers_GCR_Karim()
    millepede.path = main_path
    calib_init = millepede.create('cdc_layers_init', inputFiles)
    calib_init.max_iterations = 1

    setups.dirty_data = False
    millepede = setups.setup_CDCLayers_GCR_Karim()
    millepede.path = main_path
    millepede.set_param(1.e-3, 'minPvalue')
    calib_std = millepede.create('cdc_layers_std', inputFiles)
    calib_std.depends_on(calib_init)

    caf = CAF()
    # calib.use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
    # use_local_database("cdc_crt/database.txt", "cdc_crt")
    # use_local_database("localDB/database.txt", "localDB")
    # calib.use_central_database("GT_gen_data_002.11_gcr2017-07")

    caf.add_calibration(calib_init)
    caf.add_calibration(calib_std)
    caf.output_dir = 'caf_output'

    caf.run()

    print("Finished CAF Processing")

if __name__ == "__main__":
    # Pass arguments after script name as input files
    main(sys.argv[1:])
