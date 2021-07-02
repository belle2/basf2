##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
# This steering file shows off how you can configure a single Calibration to run
# multiple different Collector configurations. Using different database chains,
# different Collector module configs, different input files etc.

import basf2 as b2

import os
import sys

from ROOT.Belle2 import TestCalibrationAlgorithm

from caf.framework import Calibration, CAF, Collection
from caf import backends

b2.set_log_level(b2.LogLevel.INFO)


def main(argv):
    if len(argv) == 2:
        data_dir_1 = argv[0]
        data_dir_2 = argv[1]
    else:
        print("Usage: python3 caf_multiple_collections.py <data directory 1> <data directory 2>")
        sys.exit(1)

    # In this script we want to use two different sources of input data, and reconstruct them
    # differently before the Collector module runs. So we will need
    #
    # 1) Two sets of input data files
    # 2) Two pre_collector paths for reconstruction
    # 3) Two instances of a Collector module (using the same prefix/name)
    # 4) Two database chains to use

    ###################################################
    # Input Data
    # This part assumes that you've created two sources of data using the calibration/examples/1_create_sample_DSTs.sh
    # twice.
    input_files_1 = [os.path.join(os.path.abspath(data_dir_1), '*.root')]
    input_files_2 = [os.path.join(os.path.abspath(data_dir_2), '*.root')]

    ###################################################
    # Collector setup
    # We'll make two instances of the same CollectorModule, but configured differently
    # They MUST have the same granularity!

    col_test_1 = b2.register_module('CaTest')
    col_test_1.set_name('Test')   # Sets the prefix of collected data
    col_test_1.param('granularity', 'all')
    col_test_1.param('spread', 1)

    col_test_2 = b2.register_module('CaTest')
    col_test_2.set_name('Test')   # Sets the prefix of collected data
    col_test_2.param('granularity', 'all')
    col_test_2.param('spread', 20)

    ###################################################
    # Reconstruction path setup
    # create the basf2 paths to run before each Collector module
    # Exclude the ARICH instead of CDC
    rec_path_1 = b2.create_path()
    rec_path_1.add_module('Gearbox')
    rec_path_1.add_module('Geometry', excludedComponents=['SVD', 'PXD', 'ARICH', 'BeamPipe', 'EKLM'])
    # could now add reconstruction modules dependent on the type on input data

    # Excluded CDC instead of ARICH
    rec_path_2 = b2.create_path()
    rec_path_2.add_module('Gearbox')
    rec_path_2.add_module('Geometry', excludedComponents=['SVD', 'PXD', 'CDC', 'BeamPipe', 'EKLM'])
    # could now add reconstruction modules dependent on the type on input data

    ###################################################
    # Algorithm setup
    # We only want one algorithm, but we're collecting two sets of data and merging when the algorithm uses them.
    alg_test = TestCalibrationAlgorithm()
    # Must have the same prefix as colllector prefixes. By default this algorithm and collectors
    # have the correct prefixes if we hadn't changed anything. So this is unnecessary, but we're just showing you how it's done.
    alg_test.setPrefix('Test')

    ###################################################
    # Collection Setup
    # This is the key! You want to create a Collection object for each Collector you want to run.
    # This object will hold your configuration for how the Collection jobs will proceed.
    # Every Calibration object has a 'default' collection, but if you don't add a collector module to it, it won't
    # run and will be skipped. So feel free to ignore it.
    #
    # Default values for many of these config options are set unless you set them here or later
    collection_1 = Collection(collector=col_test_1,
                              input_files=input_files_1,
                              pre_collector_path=rec_path_1,
                              max_files_per_collector_job=1
                              # database_chain=[LocalDatabase('path/to/database.txt'), CentralDatabase('global_tag')],
                              # output_patterns=["CollectorOutput.root"],  Does your collector output something else
                              #                                              you want passed to the algorithm?
                              # backend_args={'queue': 's'}  Only necessary for LSF/PBS backends
                              )
    # If you have some local databases or want to override the default global tag for this Collection you can do that
    # with these functions
    # collection_1.reset_database()
    # collection_1.use_local_database("mylocaldb/database.txt")
    # collection_1.use_central_database("BelleII_GlobalTag_Tutorial")

    collection_2 = Collection(collector=col_test_2,
                              input_files=input_files_2,
                              pre_collector_path=rec_path_2,
                              max_files_per_collector_job=1
                              )

    ###################################################
    # Calibration setup
    # We won't pust any collector/algorithm stuff in yet. We're going to build it up.
    # You CAN do Calibration('name', collector=<module>) and then add other collections afterwards,
    # but we won't in this script. See documentation for examples.
    cal_test = Calibration('TestCalibration')
    # Add collections in with unique names
    cal_test.add_collection(name='collection_1', collection=collection_1)
    cal_test.add_collection(name='collection_2', collection=collection_2)
    cal_test.algorithms = alg_test

    ###################
    #   IMPORTANT!    #
    ###################
    # Notice that we can set the database chain of each Collection separately. At this point there is confusion
    # over what database chain (if any) should be used for execution of the algorithms.
    #
    # if you have multiple collections and you want a specific database chain for your algorithms to use initially in this
    # Calibration, you should specify it via the Calibration as usual e.g.
    #
    # cal_test.reset_database()
    # cal_test.use_local_database('path/to/database.txt')
    #
    # This will also append a local database to the default Collection. If you don't want this to happen, just use
    #
    # cal_test.use_local_database('path/to/database.txt', apply_to_default_collection=False)

    ###################################################
    # Create a CAF instance to run our Calibrations
    cal_fw = CAF()
    cal_fw.add_calibration(cal_test)
    cal_fw.backend = backends.Local(max_processes=4)
    cal_fw.run()
    print("End of CAF processing.")


if __name__ == "__main__":
    main(sys.argv[1:])
