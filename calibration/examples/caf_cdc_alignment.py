##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import basf2 as b2

import os
import sys


from caf.framework import CAF
from caf import backends

from reconstruction import add_cosmics_reconstruction

from alignment import setups

b2.set_log_level(b2.LogLevel.INFO)


def main(argv):
    print('Demonstrator of Millepede alignment of CDC layers with CAF')

    inputFiles = []
    for file in argv[0:]:
        inputFiles.append(os.path.abspath(file))

    if not len(inputFiles):
        print('Usage: basf2 caf_cdc_alignment DST_FILE.root [DST_FILE2.root ... ]')
        sys.exit(1)

    print('Input DST files:')
    print(inputFiles)

    reco_path = b2.create_path()
    reco_path.add_module('Progress')
    reco_path.add_module('Gearbox')
    reco_path.add_module('Geometry', excludedComponents=['SVD', 'PXD', 'ARICH', 'BeamPipe', 'EKLM'])
    # Add CDC CR reconstruction.
    add_cosmics_reconstruction(reco_path, components=['CDC'], merge_tracks=True, pruneTracks=False)

    setups.dirty_data = True
    millepede = setups.setup_CDCLayers_GCR_Karim()
    millepede.path = reco_path
    calib_init = millepede.create('cdc_layers_init', inputFiles)
    calib_init.max_iterations = 0
    calib_init.max_files_per_collector_job = 1

    setups.dirty_data = False
    millepede = setups.setup_CDCLayers_GCR_Karim()
    millepede.path = reco_path
    millepede.set_param(1.e-3, 'minPValue')
    calib_std = millepede.create('cdc_layers_std', inputFiles)
    calib_std.max_files_per_collector_job = 1

    calib_std.depends_on(calib_init)

    caf = CAF()

    caf.add_calibration(calib_init)
    caf.add_calibration(calib_std)

    caf.backend = backends.Local(2)

    caf.output_dir = 'caf_output'
    caf.run()

    print("Finished CAF Processing")


if __name__ == "__main__":
    # Pass all arguments after script name as input files
    main(sys.argv[1:])
