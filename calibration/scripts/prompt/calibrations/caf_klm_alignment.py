##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Calibration of KLM alignment. It provides calibration constants for the BKLMAlignment
and EKLMAlignment database objects.
"""

import collections

import basf2
from caf.utils import ExpRun, IoV
from prompt import CalibrationSettings, INPUT_DATA_FILTERS
from prompt.utils import events_in_basf2_file

##############################
# REQUIRED VARIABLE #
##############################
# Used to identify the keys in input_data that your get_calibrations function will need in order
# to assign data correctly.
# Will be used to construct the calibration in the automated system, as well as set up the submission web forms.
# You can view the available input data formats from CalibrationSettings.allowed_data_formats

from prompt.calibrations.vxdcdc_alignment import settings as vxdcdc_alignment

#: Tells the automated system some details of this script
# Expert configuration:
# "required_events" : number of events in basf2 files selected for processing.
# "required_events_experiment" : required number of events per experiment.
# "events_per_file" : number of events per file.
# "millepede_entries" : minimal number of Millepede entries.
# Test with experiment 12 physics data:
# ~350000 events correspond to ~1300000 Millepede entries.
settings = CalibrationSettings(name="KLM alignmnent",
                               expert_username="oskin",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=["raw_physics", "raw_cosmic"],
                               input_data_filters={
                                   'raw_physics': [INPUT_DATA_FILTERS['Run Type']['physics'],
                                                   INPUT_DATA_FILTERS['Data Tag']['mumu_tight_or_highm_calib'],
                                                   INPUT_DATA_FILTERS['Data Quality Tag']['Good Or Recoverable']],
                                   'raw_cosmic': [INPUT_DATA_FILTERS['Run Type']['physics'],
                                                  INPUT_DATA_FILTERS['Data Tag']['cosmic_calib'],
                                                  INPUT_DATA_FILTERS['Data Quality Tag']['Good Or Recoverable']]
                               },
                               depends_on=[vxdcdc_alignment],
                               expert_config={
                                   "required_events": 5000000,
                                   "required_events_experiment": 500000,
                                   "events_per_file": 1000,
                                   "millepede_entries": 1000000,
                                   "millepede_entries_exp7": 500000
                               })


##############################


def select_input_files(file_to_iov_physics, file_to_iov_cosmic,
                       reduced_file_to_iov_physics, reduced_file_to_iov_cosmic,
                       required_events, required_events_experiment,
                       events_per_file):
    """
    Parameters:
        files_to_iov_physics (dict): Dictionary {run : IOV} for physics data.
        files_to_iov_cosmic (dict): Dictionary {run : IOV} for cosmic data.
        reduced_file_to_iov_physics (dict): Selected physics data.
        reduced_file_to_iov_cosmic (dict): Selected cosmic data.
        required_events (int): Required total number of events.
        required_events_experiment (int): Required number of events
                                          per experiment.
        events_per_file (int): Number of events per file. If non-positive, then
                               the number is not limited.
    """
    experiment_numbers = set()
    run_to_files_physics = collections.defaultdict(list)
    for input_file, file_iov in file_to_iov_physics.items():
        run = ExpRun(exp=file_iov.exp_low, run=file_iov.run_low)
        run_to_files_physics[run].append(input_file)
        experiment_numbers.add(file_iov.exp_low)
    run_to_files_cosmic = collections.defaultdict(list)
    for input_file, file_iov in file_to_iov_cosmic.items():
        run = ExpRun(exp=file_iov.exp_low, run=file_iov.run_low)
        run_to_files_cosmic[run].append(input_file)
        experiment_numbers.add(file_iov.exp_low)
    max_files_per_run = 0
    for files in run_to_files_physics.values():
        files_per_run = len(files)
        if files_per_run > max_files_per_run:
            max_files_per_run = files_per_run
    for files in run_to_files_cosmic.values():
        files_per_run = len(files)
        if files_per_run > max_files_per_run:
            max_files_per_run = files_per_run
    files_per_run = 0
    collected_events = 0
    collected_events_experiment = collections.defaultdict(int)
    select_events_experiment = collections.defaultdict(bool)
    for exp in experiment_numbers:
        collected_events_experiment[exp] = 0
        select_events_experiment[exp] = True
    while files_per_run < max_files_per_run:
        for run, files in run_to_files_physics.items():
            if not select_events_experiment[run.exp]:
                continue
            if files_per_run >= len(files):
                continue
            input_file = files[files_per_run]
            events = events_in_basf2_file(input_file)
            # Reject files without events.
            if events == 0:
                continue
            if events_per_file > 0 and events > events_per_file:
                events = events_per_file
            collected_events = collected_events + events
            collected_events_experiment[run.exp] = \
                collected_events_experiment[run.exp] + events
            reduced_file_to_iov_physics[input_file] = IoV(*run, *run)
            basf2.B2INFO(f'File {input_file} with {events} events is selected.')
        for run, files in run_to_files_cosmic.items():
            if not select_events_experiment[run.exp]:
                continue
            if files_per_run >= len(files):
                continue
            input_file = files[files_per_run]
            events = events_in_basf2_file(input_file)
            # Reject files without events.
            if events == 0:
                continue
            if events_per_file > 0 and events > events_per_file:
                events = events_per_file
            collected_events = collected_events + events
            collected_events_experiment[run.exp] = \
                collected_events_experiment[run.exp] + events
            reduced_file_to_iov_cosmic[input_file] = IoV(*run, *run)
            basf2.B2INFO(f'File {input_file} with {events} events is selected.')
        files_per_run = files_per_run + 1
        if collected_events >= required_events:
            all_experiments_selected = True
            for exp in experiment_numbers:
                if collected_events_experiment[exp] >= \
                        required_events_experiment:
                    select_events_experiment[exp] = False
                else:
                    all_experiments_selected = False
            if all_experiments_selected:
                break
    basf2.B2INFO(f'The total number of collected events is {collected_events}.')

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

    # Expert configuration.
    expert_config = kwargs.get("expert_config")

    # In this script we want to use one sources of input data.
    # Get the input files  from the input_data variable
    file_to_iov_physics = input_data["raw_physics"]
    file_to_iov_cosmic = input_data["raw_cosmic"]

    # Select input files.
    reduced_file_to_iov_physics = collections.OrderedDict()
    reduced_file_to_iov_cosmic = collections.OrderedDict()
    select_input_files(file_to_iov_physics, file_to_iov_cosmic,
                       reduced_file_to_iov_physics, reduced_file_to_iov_cosmic,
                       expert_config["required_events"],
                       expert_config["required_events_experiment"],
                       expert_config["events_per_file"])

    input_files_physics = sorted(list(reduced_file_to_iov_physics.keys()))
    basf2.B2INFO(f"Total number of 'physics' files actually used as input = {len(input_files_physics)}")

    input_files_cosmic = sorted(list(reduced_file_to_iov_cosmic.keys()))
    basf2.B2INFO(f"Total number of 'cosmic' files actually used as input = {len(input_files_cosmic)}")

    if not input_files_physics and not input_files_cosmic:
        raise Exception("No valid input files found!")

    # Get the overall IoV we our process should cover. Includes the end values that we may want to ignore since our output
    # IoV should be open ended. We could also use this as part of the input data selection in some way.
    requested_iov = kwargs["requested_iov"]

    from caf.utils import IoV
    # The actual value our output IoV payload should have. Notice that we've set it open ended.
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    ###################################################
    # Algorithm setup

    from ROOT import Belle2
    from ROOT.Belle2 import KLMChannelIndex, KLMElementNumbers
    from alignment import MillepedeCalibration

    # Create the algorithm.
    millepede = MillepedeCalibration(['BKLMAlignment', 'EKLMAlignment', 'EKLMSegmentAlignment'])

    # Fix module parameters.
    index = KLMChannelIndex(KLMChannelIndex.c_IndexLevelLayer)
    index2 = KLMChannelIndex(KLMChannelIndex.c_IndexLevelLayer)
    while (index != index2.end()):
        module = index.getKLMModuleNumber()
        if (index.getSubdetector() == KLMElementNumbers.c_BKLM):
            for ipar in [1, 2, 3, 4, 5, 6]:
                # Free parameters are idU, dV, dGamma.
                if ipar in [1, 2, 6]:
                    continue
                millepede.fixGlobalParam(Belle2.BKLMAlignment.getGlobalUniqueID(),
                                         module, ipar)
        else:
            for ipar in [1, 2, 6]:
                # No parameters are fixed; if necessary, uncomment the following:
                # millepede.fixGlobalParam(Belle2.EKLMAlignment.getGlobalUniqueID(),
                #                          module, ipar)
                continue
        index.increment()

    # Fix EKLM segment parameters.
    index.setIndexLevel(KLMChannelIndex.c_IndexLevelStrip)
    index2.setIndexLevel(KLMChannelIndex.c_IndexLevelStrip)
    index = index2.beginEKLM()
    index.useEKLMSegments()
    while (index != index2.endEKLM()):
        segment = index.getEKLMSegmentNumber()
        for ipar in [2, 6]:
            millepede.fixGlobalParam(
                Belle2.EKLMSegmentAlignment.getGlobalUniqueID(),
                segment, ipar)
        index.increment()

    cal_klm = millepede.create('KLMAlignment', [])
    # Number of entries is set in algorithm strategy now.
    millepede.algo.ignoreUndeterminedParams(True)
    millepede.algo.invertSign()

    ###################################################
    # Calibration setup

    from caf.framework import Collection

    ########
    # Collect on multiple input data types for one calibration

    from klm_calibration_utils import get_alignment_pre_collector_path_physics, get_alignment_pre_collector_path_cosmic

    entries = ''
    if expert_config["events_per_file"] > 0:
        last_entry = expert_config["events_per_file"]
        entries = f'0:{last_entry}'

    if input_files_physics:
        coll_physics = get_collector("raw_physics")
        rec_path_physics = get_alignment_pre_collector_path_physics(entry_sequence=entries)

        collection_physics = Collection(collector=coll_physics,
                                        input_files=input_files_physics,
                                        pre_collector_path=rec_path_physics)

        cal_klm.add_collection(name="physics", collection=collection_physics)

    if input_files_cosmic:
        coll_cosmic = get_collector("raw_cosmic")
        rec_path_cosmic = get_alignment_pre_collector_path_cosmic(entry_sequence=entries)

        collection_cosmic = Collection(collector=coll_cosmic,
                                       input_files=input_files_cosmic,
                                       pre_collector_path=rec_path_cosmic)

        cal_klm.add_collection(name="cosmic", collection=collection_cosmic)

    #####
    # Algorithm step config

    from klm_alignment import KLMAlignment

    cal_klm.algorithms = [millepede.algo]

    # Bugfix for Condor:
    from alignment.prompt_utils import fix_mille_paths_for_algo
    for algorithm in cal_klm.algorithms:
        fix_mille_paths_for_algo(algorithm)

    for algorithm in cal_klm.algorithms:
        algorithm.strategy = KLMAlignment
        algorithm.params = {
            "has_experiment_settings": True,
            "iov_coverage": output_iov,
            "millepede_entries": expert_config["millepede_entries"],
            "millepede_entries_exp7": expert_config["millepede_entries_exp7"]
        }

    # You must return all calibrations you want to run in the prompt process, even if it's only one
    return [cal_klm]

##############################


def get_collector(input_data_name):
    """
    Return the correct MillepedeCollector module setup for each data type.
    Placed here so it can be different for prompt compared to standard.
    """

    if input_data_name == "raw_physics":
        return basf2.register_module(
            'MillepedeCollector',
            components=['BKLMAlignment', 'EKLMAlignment',
                        'EKLMSegmentAlignment'],
            useGblTree=True,
            minPValue=1e-5)
    elif input_data_name == "raw_cosmic":
        return basf2.register_module(
            'MillepedeCollector',
            components=['BKLMAlignment', 'EKLMAlignment',
                        'EKLMSegmentAlignment'],
            useGblTree=True,
            minPValue=1e-5)

    raise Exception("Unknown input data name used when setting up collector")
