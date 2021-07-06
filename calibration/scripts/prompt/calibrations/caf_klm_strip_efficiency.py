# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Calibration of KLM strip efficiency. It provides calibration constants for the KLMSripEfficiency
database object.
"""

import basf2
from prompt import CalibrationSettings, input_data_filters

##############################
# REQUIRED VARIABLE #
##############################
# Used to identify the keys in input_data that your get_calibrations function will need in order
# to assign data correctly.
# Will be used to construct the calibration in the automated system, as well as set up the submission web forms.
# You can view the available input data formats from CalibrationSettings.allowed_data_formats

#: Tells the automated system some details of this script
settings = CalibrationSettings(
    name='KLM strip efficiency',
    expert_username='depietro',
    description=__doc__,
    input_data_formats=['cdst'],
    input_data_names=['hlt_mumu'],
    input_data_filters={
        'hlt_mumu': [input_data_filters['Run Type']['physics'],
                     input_data_filters['Data Tag']['mumutight_calib'],
                     input_data_filters['Data Quality Tag']['Good Or Recoverable']]
    },
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
    file_to_iov_cdst = input_data['hlt_mumu']

    # We might have requested an enormous amount of data across a run range.
    # There's a LOT more files than runs!
    # Lets set some limits because this calibration doesn't need that much to run.
    # max_files_per_run = 2

    # If you are using Raw data there's a chance that input files could have zero events.
    # This causes a B2FATAL in basf2 RootInput so the collector job will fail.
    # Currently we don't have a good way of filtering this on the automated side, so we can check here.
    # min_events_per_file = 1

    # We filter out any more than 2 files per run. The input data files are sorted alphabetically by b2caf-prompt-run
    # already. This procedure respects that ordering

    # For testing
    # reduced_file_to_iov_cdst = filter_by_max_files_per_run(file_to_iov_cdst, max_files_per_run, min_events_per_file)
    # input_files_cdst = sorted(list(reduced_file_to_iov_cdst.keys()))
    input_files_cdst = sorted(list(file_to_iov_cdst.keys()))
    basf2.B2INFO(f'Total number of \'hlt_mumu\' files actually used as input = {len(input_files_cdst)}')

    if not input_files_cdst:
        raise Exception('No valid input files found!')

    # Get the overall IoV we our process should cover. Includes the end values that we may want to ignore since our output
    # IoV should be open ended. We could also use this as part of the input data selection in some way.
    requested_iov = kwargs['requested_iov']

    from caf.utils import IoV
    # The actual value our output IoV payload should have. Notice that we've set it open ended.
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    ###################################################
    # Algorithm setup

    from ROOT.Belle2 import KLMStripEfficiencyAlgorithm

    alg = KLMStripEfficiencyAlgorithm()

    ###################################################
    # Calibration setup

    from caf.framework import Calibration, Collection

    cal_klm = Calibration('KLMStripEfficiency')

    ########
    # Collect on multiple input data types for one calibration

    from klm_calibration_utils import get_strip_efficiency_pre_collector_path

    if input_files_cdst:
        muon_list_name = 'klmStripEfficiency'
        coll_cdst = get_collector(input_data_name='hlt_mumu',
                                  muon_list_name=muon_list_name)
        rec_path_cdst = get_strip_efficiency_pre_collector_path(muon_list_name=muon_list_name)

        collection_cdst = Collection(collector=coll_cdst,
                                     input_files=input_files_cdst,
                                     pre_collector_path=rec_path_cdst)

        cal_klm.add_collection(name='cdst', collection=collection_cdst)

    #####
    # Algorithm step config

    cal_klm.algorithms = [alg]

    from klm_strip_efficiency import KLMStripEfficiency

    for algorithm in cal_klm.algorithms:
        algorithm.strategy = KLMStripEfficiency
        algorithm.params = {'iov_coverage': output_iov}

    # You must return all calibrations you want to run in the prompt process, even if it's only one
    return [cal_klm]

##############################


def get_collector(input_data_name, muon_list_name):
    """
    Return the correct KLMStripEfficiencyCollector module setup for each data type.
    Placed here so it can be different for prompt compared to standard.
    """

    if input_data_name == 'hlt_mumu':
        return basf2.register_module('KLMStripEfficiencyCollector',
                                     MuonListName=f'mu+:{muon_list_name}',
                                     MinimalMatchingDigits=14,
                                     MinimalMatchingDigitsOuterLayers=4,
                                     MinimalMomentumNoOuterLayers=4.0)
    raise Exception("Unknown input data name used when setting up collector")
