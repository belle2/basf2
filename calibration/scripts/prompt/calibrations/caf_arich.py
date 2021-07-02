##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
# -*- coding: utf-8 -*-

"""A simple example calibration that takes one input data list from raw data and performs
a single calibration."""

from prompt import CalibrationSettings, input_data_filters

#: Tells the automated system some details of this script
settings = CalibrationSettings(
    name="ARICH channel masks",
    expert_username="luka",
    description=__doc__,
    input_data_formats=["raw"],
    input_data_names=["bhabha_all_calib"],
    input_data_filters={
        "bhabha_all_calib": [
            input_data_filters["Data Tag"]["bhabha_all_calib"],
            input_data_filters["Run Type"]["physics"],
            input_data_filters["Data Quality Tag"]["Good Or Recoverable"]]},
    depends_on=[])

##############################


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
    file_to_iov_physics = input_data["bhabha_all_calib"]

    # We might have requested an enormous amount of data across a run range.
    # There's a LOT more files than runs!
    # Lets set some limits because this calibration doesn't need that much to run.
    max_files_per_run = 100

    # We filter out any more than 100 files per run. The input data files are sorted alphabetically by b2caf-prompt-run
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
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    ###################################################
    # Algorithm setup

    from basf2 import create_path
    from ROOT.Belle2 import ARICHChannelMaskMaker

    alg_arich = ARICHChannelMaskMaker()

    ###################################################
    # Calibration setup

    from caf.framework import Calibration
    from caf.strategies import SequentialRunByRun

    # module to be run prior the collector
    rec_path_1 = create_path()
    rec_path_1.add_module('ARICHUnpacker')

    cal_test = Calibration("ARICHChannelMasks",
                           collector="ARICHChannelMask",
                           algorithms=[alg_arich],
                           input_files=input_files_physics,
                           pre_collector_path=rec_path_1
                           )

    cal_test.strategies = SequentialRunByRun

    # Do this for the default AlgorithmStrategy to force the output payload IoV
    # It may be different if you are using another strategy like SequentialRunByRun
    for algorithm in cal_test.algorithms:
        algorithm.params = {"iov_coverage": output_iov}

    # Most other options like database chain and backend args will be overwritten by b2caf-prompt-run.
    # So we don't bother setting them.

    # You must return all calibrations you want to run in the prompt process, even if it's only one
    return [cal_test]

##############################
