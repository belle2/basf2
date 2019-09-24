# -*- coding: utf-8 -*-

"""A complicated example calibration that takes several input data lists from raw data and performs
multiple calibrations."""

from prompt import CalibrationSettings

##############################
# REQUIRED VARIABLE #
##############################
# Used to identify the keys in input_data that your get_calibrations function will need in order
# to assign data correctly.
# Will be used to construct the calibration in the automated system, as well as set up the submission web forms.
# You can view the available input data formats from CalibrationSettings.allowed_data_formats

#: Tells the automated system some details of this script
settings = CalibrationSettings(name="Example Complex",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=["physics", "cosmics", "Bcosmics"])

##############################

##############################
# REQUIRED FUNCTION #
##############################
# The only function that MUST exist in this module. It should return a LIST of Calibration objects
# that have had their input files assigned and any configuration applied. The final output payload IoV(s)
# should also be set correctly to be open-ended e.g. IoV(exp_low, run_low, -1, -1)
#
# The database_chain, backend_args, backend, max_files_per_collector_job, and heartbeat of these
# calibrations will all be set/overwritten by the b2caf-prompt-run tool.


def get_calibrations(input_data, **kwargs):
    """
    Parameters:
      input_data (dict): Should contain every name from the 'input_data_names' variable as a key.
        Each value is a dictionary with {"/path/to/file_e1_r5.root": IoV(1,5,1,5), ...}. Useful for
        assigning to calibration.files_to_iov

      **kwargs: Configuration options to be sent in. Since this may change we use kwargs as a way to help prevent
        backwards compatibility problems. But you could use the correct arguments in b2caf-prompt-run for this
        release explicitly if you want to.

        Currently only kwargs["output_iov"] is used. This is the output IoV range that your payloads should
        correspond to. Generally your highest ExpRun payload should be open ended e.g. IoV(3,4,-1,-1)

    Returns:
      list(caf.framework.Calibration): All of the calibration objects we want to assign to the CAF process
    """
    # Set up config options
    from basf2 import register_module, create_path
    import ROOT
    from ROOT.Belle2 import TestCalibrationAlgorithm
    from caf.framework import Calibration, Collection
    # In this script we want to use three different sources of input data, and reconstruct them
    # differently before the Collector module runs.

    # Get the input files from the input_data variable
    input_files_physics = list(input_data["physics"].keys())
    input_files_cosmics = list(input_data["cosmics"].keys())
    input_files_Bcosmics = list(input_data["Bcosmics"].keys())

    # Get the overall IoV we want to cover, including the end values (probably -1,-1 for open ended)
    overall_iov = kwargs.get("output_iov", None)

    ###################################################
    # Collector setup
    # We'll make two instances of the same CollectorModule, but configured differently
    col_test_physics = register_module('CaTest')
    col_test_physics.param('granularity', 'all')
    col_test_physics.param('spread', 4)

    col_test_Bcosmics = register_module('CaTest')
    col_test_Bcosmics.param('granularity', 'all')
    col_test_Bcosmics.param('spread', 1)

    col_test_cosmics = register_module('CaTest')
    col_test_cosmics.param('granularity', 'all')
    col_test_cosmics.param('spread', 10)

    ###################################################
    # Reconstruction path setup
    # create the basf2 paths to run before each Collector module

    # Let's specify that not all events will be used per file
    # This assumes that we will be using the default max_files_per_collector_job=1 in b2caf-prompt-run!
    max_events = 100
    root_input = register_module('RootInput',
                                 entrySequences=[f'0:{max_events}']
                                 )

    rec_path_physics = create_path()
    rec_path_physics.add_module(root_input)
    rec_path_physics.add_module('Gearbox')
    rec_path_physics.add_module('Geometry', excludedComponents=['SVD', 'PXD', 'ARICH', 'BeamPipe', 'EKLM'])
    # could now add reconstruction modules dependent on the type on input data

    rec_path_cosmics = create_path()
    rec_path_cosmics.add_module(root_input)
    rec_path_cosmics.add_module('Gearbox')
    rec_path_cosmics.add_module('Geometry', excludedComponents=['SVD', 'PXD', 'CDC', 'BeamPipe', 'EKLM'])
    # could now add reconstruction modules dependent on the type on input data

    rec_path_Bcosmics = create_path()
    rec_path_Bcosmics.add_module(root_input)
    rec_path_Bcosmics.add_module('Gearbox')
    rec_path_Bcosmics.add_module('Geometry', excludedComponents=['SVD', 'PXD', 'CDC', 'BeamPipe', 'EKLM'])
    # could now add reconstruction modules dependent on the type on input data

    ###################################################
    # Algorithm setup
    # We'll only use one algorithm for each of our two Calibrations.
    alg_test1 = TestCalibrationAlgorithm()
    alg_test2 = TestCalibrationAlgorithm()

    ###################################################
    # Collection Setup
    collection_physics = Collection(collector=col_test_physics,
                                    input_files=input_files_physics,
                                    pre_collector_path=rec_path_physics,
                                    )

    collection_cosmics = Collection(collector=col_test_cosmics,
                                    input_files=input_files_cosmics,
                                    pre_collector_path=rec_path_cosmics,
                                    )

    collection_Bcosmics = Collection(collector=col_test_Bcosmics,
                                     input_files=input_files_Bcosmics,
                                     pre_collector_path=rec_path_Bcosmics,
                                     )

    ###################################################
    # Calibration setup

    cal_test1 = Calibration("TestCalibration_cosmics")
    # Add collections in with unique names
    cal_test1.add_collection(name="cosmics", collection=collection_cosmics)
    cal_test1.add_collection(name='Bcosmics', collection=collection_Bcosmics)
    cal_test1.algorithms = [alg_test1]
    # Do this for the default AlgorithmStrategy to force the output payload IoV
    cal_test1.algorithms[0].params = {"apply_iov": overall_iov}

    cal_test2 = Calibration("TestCalibration_physics")
    # Add collections in with unique names
    cal_test2.add_collection(name="physics", collection=collection_physics)
    cal_test2.algorithms = [alg_test2]
    # Do this for the default AlgorithmStrategy to force the output payload IoV
    cal_test2.algorithms[0].params = {"apply_iov": overall_iov}

    cal_test2.depends_on(cal_test1)

    # You must return all calibrations you want to run in the prompt process
    return [cal_test1, cal_test2]

##############################
