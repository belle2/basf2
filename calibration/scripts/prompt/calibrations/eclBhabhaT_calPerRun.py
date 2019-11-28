# -*- coding: utf-8 -*-

"""ECL timing calibration that performs the crate calibrations, one for each physics run."""

from prompt import CalibrationSettings

##############################
# REQUIRED VARIABLE #
##############################
# Used to identify the keys in input_data that your get_calibrations function will need in order
# to assign data correctly.
# Will be used to construct the calibration in the automated system, as well as set up the submission web forms.
# You can view the available input data formats from CalibrationSettings.allowed_data_formats

#: Tells the automated system some details of this script
settings = CalibrationSettings(name="ECL crate time calibrations",
                               expert_username="ehill",
                               description=__doc__,
                               input_data_formats=["cdst"],
                               input_data_names=["physics"],
                               depends_on=[])

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
    import basf2
    # Set up config options

    # In this script we want to use one sources of input data.
    # Get the input files  from the input_data variable
    file_to_iov_physics = input_data["physics"]

    # We might have requested an enormous amount of data across a run range.
    # There's a LOT more files than runs!
    # Lets set some limits because this calibration doesn't need that much to run.
    max_files_per_run = 2

    # We filter out any more than 2 files per run. The input data files are sorted alphabetically by b2caf-prompt-run
    # already. This procedure respects that ordering
    from prompt.utils import filter_by_max_files_per_run

    reduced_file_to_iov_physics = filter_by_max_files_per_run(file_to_iov_physics, max_files_per_run)
    input_files_physics = list(reduced_file_to_iov_physics.keys())
    basf2.B2INFO(f"Total number of files actually used as input = {len(input_files_physics)}")

    # Get the overall IoV we our process should cover. Includes the end values that we may want to ignore since our output
    # IoV should be open ended. We could also use this as part of the input data selection in some way.
    requested_iov = kwargs.get("requested_iov", None)

    from caf.utils import IoV
    # The actual value our output IoV payload should have. Notice that we've set it open ended.
#    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    ###################################################
    # Algorithm setup

    import ROOT
    from ROOT import Belle2
    from ROOT.Belle2 import TestCalibrationAlgorithm

    eclTAlg = Belle2.ECL.eclBhabhaTAlgorithm()

    # Define the CAF algorithm arguments
    eclTAlg.cellIDLo = 3
    eclTAlg.cellIDHi = 2
    eclTAlg.debugOutput = True
    eclTAlg.meanCleanRebinFactor = 3
    eclTAlg.meanCleanCutMinFactor = 0.3
    # eclTAlg.crateIDLo = 10
    # eclTAlg.crateIDHi = 9
    eclTAlg.debugFilenameBase = "eclBhabhaTAlgorithm"

    ###################################################
    # Calibration setup

    from caf.framework import Calibration

    cal_test = Calibration("ECLcrateTimeCalibration_physics",
                           collector="ECLBhabhaTCollector",
                           algorithms=[eclTAlg],
                           input_files=input_files_physics
                           )

    # Here we set the AlgorithmStrategy for our algorithm
    from caf.strategies import SimpleRunByRun

    # The SimpleRunByRun strategy executes your algorithm over runs
    # individually to give you payloads for each one (if successful)
    # It will not do any merging of runs which didn't contain enough data.
    # So failure is expected if your algorithm requires a large amount of data compared to run length.
    # You should only use granularity='run' for the collector when using this strategy.

    cal_test.strategies = SimpleRunByRun


#    # Do this for the default AlgorithmStrategy to force the output payload IoV
#    # It may be different if you are using another strategy like SequentialRunByRun
#    for algorithm in cal_test.algorithms:
#        algorithm.params = {"apply_iov": output_iov}

    # Most other options like database chain and backend args will be overwritten by b2caf-prompt-run.
    # So we don't bother setting them.

    # You must return all calibrations you want to run in the prompt process, even if it's only one
    return [cal_test]

##############################
