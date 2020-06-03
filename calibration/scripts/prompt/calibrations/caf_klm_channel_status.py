# -*- coding: utf-8 -*-

"""A simple example calibration that takes one input data list from raw data and performs
a single calibration."""

import basf2
from prompt import CalibrationSettings

##############################
# REQUIRED VARIABLE #
##############################
# Used to identify the keys in input_data that your get_calibrations function will need in order
# to assign data correctly.
# Will be used to construct the calibration in the automated system, as well as set up the submission web forms.
# You can view the available input data formats from CalibrationSettings.allowed_data_formats

#: Tells the automated system some details of this script
settings = CalibrationSettings(name='KLM channel status',
                               expert_username='zhai',
                               description=__doc__,
                               input_data_formats=['raw'],
                               input_data_names=['raw'],
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

        Currently only kwargs["requested_iov"] is used. This is the output IoV range that your payloads should
        correspond to. Generally your highest ExpRun payload should be open ended e.g. IoV(3,4,-1,-1)

    Returns:
      list(caf.framework.Calibration): All of the calibration objects we want to assign to the CAF process
    """
    # Set up config options

    # In this script we want to use one sources of input data.
    # Get the input files  from the input_data variable
    file_to_iov_raw = input_data['raw']

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

    # For testing
    # reduced_file_to_iov_raw = filter_by_max_files_per_run(file_to_iov_raw, max_files_per_run, min_events_per_file)
    # input_files_raw = sorted(list(reduced_file_to_iov_raw.keys()))
    input_files_raw = sorted(list(file_to_iov_raw.keys()))
    basf2.B2INFO(f'Total number of \'raw\' files actually used as input = {len(input_files_raw)}')

    if not input_files_raw:
        raise Exception('No valid input files found!')

    # Get the overall IoV we our process should cover. Includes the end values that we may want to ignore since our output
    # IoV should be open ended. We could also use this as part of the input data selection in some way.
    requested_iov = kwargs['requested_iov']

    from caf.utils import IoV
    # The actual value our output IoV payload should have. Notice that we've set it open ended.
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    ###################################################
    # Algorithm setup

    import ROOT
    from ROOT.Belle2 import KLMChannelStatusAlgorithm

    alg = KLMChannelStatusAlgorithm()

    ###################################################
    # Calibration setup

    from caf.framework import Calibration, Collection

    cal_klm = Calibration('KLMChannelStatus')

    ########
    # Collect on multiple input data types for one calibration

    from klm_calibration_utils import get_channel_status_pre_collector_path

    if input_files_raw:
        coll_raw = get_collector('raw')
        rec_path_raw = get_channel_status_pre_collector_path()

        collection_raw = Collection(collector=coll_raw,
                                    input_files=input_files_raw,
                                    pre_collector_path=rec_path_raw)

        cal_klm.add_collection(name='raw', collection=collection_raw)

    #####
    # Algorithm step config

    cal_klm.algorithms = [alg]

    from klm_channel_status import KLMChannelStatus

    for algorithm in cal_klm.algorithms:
        algorithm.strategy = KLMChannelStatus
        algorithm.params = {'iov_coverage': output_iov}

    # You must return all calibrations you want to run in the prompt process, even if it's only one
    return [cal_klm]

##############################


def get_collector(input_data_name):
    """
    Return the correct KLMChannelStatusCollector module setup for each data type.
    Placed here so it can be different for prompt compared to standard.
    """

    if input_data_name == 'raw':
        return basf2.register_module('KLMChannelStatusCollector')
    raise Exception("Unknown input data name used when setting up collector")
