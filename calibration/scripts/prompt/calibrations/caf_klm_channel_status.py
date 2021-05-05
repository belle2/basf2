# -*- coding: utf-8 -*-

"""Calibration of KLM channel status."""

import collections

import basf2
from caf.utils import ExpRun, IoV
from prompt import CalibrationSettings, input_data_filters
from prompt.utils import events_in_basf2_file

##############################
# REQUIRED VARIABLE #
##############################
# Used to identify the keys in input_data that your get_calibrations function will need in order
# to assign data correctly.
# Will be used to construct the calibration in the automated system, as well as set up the submission web forms.
# You can view the available input data formats from CalibrationSettings.allowed_data_formats

#: Tells the automated system some details of this script
settings = CalibrationSettings(
    name='KLM channel status',
    expert_username='zhai',
    description=__doc__,
    input_data_formats=['raw'],
    input_data_names=['raw_beam', 'raw_cosmic', 'raw_physics'],
    input_data_filters={
        'raw_beam': [input_data_filters['Run Type']['beam'],
                     input_data_filters['Data Quality Tag']['Good Or Recoverable']],
        'raw_cosmic': [input_data_filters['Run Type']['cosmic'],
                       input_data_filters['Data Quality Tag']['Good Or Recoverable']],
        'raw_physics': [input_data_filters['Run Type']['physics'],
                        f"NOT {input_data_filters['Data Tag']['random_calib']}",
                        input_data_filters['Data Quality Tag']['Good Or Recoverable']]
    },
    depends_on=[])

##############################


def select_input_files(file_to_iov):
    """
    Parameters:
        files_to_iov (dict): Dictionary {run : IOV}.
        reduced_file_to_iov (dict): Selected data.
    """
    run_to_files = collections.defaultdict(list)
    for input_file, file_iov in file_to_iov.items():
        run = ExpRun(exp=file_iov.exp_low, run=file_iov.run_low)
        # Reject files without events.
        if events_in_basf2_file(input_file) > 0:
            run_to_files[run].append(input_file)
    reduced_file_to_iov = collections.OrderedDict()
    for run, files in run_to_files.items():
        for input_file in files:
            reduced_file_to_iov[input_file] = IoV(*run, *run)
    return reduced_file_to_iov

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
    file_to_iov_raw_beam = input_data['raw_beam']
    file_to_iov_raw_cosmic = input_data['raw_cosmic']
    file_to_iov_raw_physics = input_data['raw_physics']

    # Select input files (all data are necessary, only removes empty files).
    reduced_file_to_iov_raw_beam = select_input_files(file_to_iov_raw_beam)
    reduced_file_to_iov_raw_cosmic = select_input_files(file_to_iov_raw_cosmic)
    reduced_file_to_iov_raw_physics = select_input_files(file_to_iov_raw_physics)

    # Merge all input data.
    input_files_raw = list(reduced_file_to_iov_raw_beam.keys())
    input_files_raw.extend(list(reduced_file_to_iov_raw_cosmic.keys()))
    input_files_raw.extend(list(reduced_file_to_iov_raw_physics.keys()))
    input_files_raw.sort()
    basf2.B2INFO(f'Total number of raw-data files used as input = {len(input_files_raw)}')

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
