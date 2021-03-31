# -*- coding: utf-8 -*-

"""A simple example calibration that takes one input data list from raw data and performs
a single calibration."""

from prompt import CalibrationSettings

##############################
# REQUIRED VARIABLE #
##############################
# Used to identify the keys in input_data that your get_calibrations function will need in order
# to assign data correctly.
# Will be used to construct the calibration in the automated system, as well as set up the submission web forms.
# You can view the available input data formats from CalibrationSettings.allowed_data_formats

#: Tells the automated system some details of this script
settings = CalibrationSettings(name="Example Simple",
                               expert_username="ddossett",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=["physics"],
                               depends_on=[],
                               expert_config={})

##############################

##############################
# REQUIRED FUNCTION #
##############################
# The only function that MUST exist in this module. It should return a LIST of Calibration objects
# that have had their input files assigned and any configuration applied. The final output payload IoV(s)
# should also be set correctly to be open-ended e.g. IoV(exp_low, run_low, -1, -1)
#
# The database_chain of these calibrations will all be set/overwritten by the b2caf-prompt-run tool.


def get_calibrations(input_data, **kwargs):
    """
    Parameters:
      input_data (dict): Should contain every name from the 'input_data_names' variable as a key.
        Each value is a dictionary with {"/path/to/file_e1_r5.root": IoV(1,5,1,5), ...}. Useful for
        assigning to calibration.files_to_iov

      **kwargs: Configuration options to be sent in. Since this may change we use kwargs as a way to help prevent
        backwards compatibility problems. But you could use the correct arguments in b2caf-prompt-run for this
        release explicitly if you want to.

        Currently only kwargs["requested_iov"] and kwargs["expert_config"] are used.

        "requested_iov" is the IoV range of the bucket and your payloads should correspond to this range.
        However your highest payload IoV should be open ended e.g. IoV(3,4,-1,-1)

        "expert_config" is the input configuration. It takes default values from your `CalibrationSettings` but these are
        overwritten by values from the 'expert_config' key in your input `caf_config.json` file when running ``b2caf-prompt-run``.

    Returns:
      list(caf.framework.Calibration): All of the calibration objects we want to assign to the CAF process
    """
    import basf2
    # Set up config options

    # In this script we want to use one sources of input data.
    # Get the input files  from the input_data variable
    file_to_iov_physics = input_data["physics"]

    # We might have requested an enormous amount of data across a run range.
    # There's a LOT more files than runs!
    # Lets set some limits because this calibration doesn't need that much to run.
    max_files_per_run = 2

    # If you are using Raw data there's a chance that input files could have zero events.
    # This causes a B2FATAL in basf2 RootInput so the collector job will fail.
    # Currently we don't have a good way of filtering this on the automated side, so we can check here.
    min_events_per_file = 1

    # We filter out any more than 2 files per run. The input data files are sorted alphabetically by b2caf-prompt-run
    # already. This procedure respects that ordering
    from prompt.utils import filter_by_max_files_per_run

    reduced_file_to_iov_physics = filter_by_max_files_per_run(file_to_iov_physics, max_files_per_run, min_events_per_file)
    input_files_physics = list(reduced_file_to_iov_physics.keys())
    basf2.B2INFO(f"Total number of files actually used as input = {len(input_files_physics)}")

    # Get the overall IoV we our process should cover. Includes the end values that we may want to ignore since our output
    # IoV should be open ended. We could also use this as part of the input data selection in some way.
    requested_iov = kwargs.get("requested_iov", None)

    # Get the expert configurations if you have something you might configure from them. It should always be available
    # expert_config = kwargs.get("expert_config")

    from caf.utils import IoV
    # The actual value our output IoV payload should have. Notice that we've set it open ended.
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    ###################################################
    # Algorithm setup

    from ROOT.Belle2 import TestCalibrationAlgorithm

    alg_test = TestCalibrationAlgorithm()

    ###################################################
    # Calibration setup

    from caf.framework import Calibration

    cal_test = Calibration("TestCalibration_physics",
                           collector="CaTest",
                           algorithms=[alg_test],
                           input_files=input_files_physics
                           )
    # Do this for the default AlgorithmStrategy to force the output payload IoV
    # It may be different if you are using another strategy like SequentialRunByRun
    for algorithm in cal_test.algorithms:
        algorithm.params = {"apply_iov": output_iov}

    # Most other options like database chain and backend args will be overwritten by b2caf-prompt-run.
    # So we don't bother setting them.

    # You must return all calibrations you want to run in the prompt process, even if it's only one
    return [cal_test]

##############################
