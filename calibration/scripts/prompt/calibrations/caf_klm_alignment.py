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

from prompt.calibrations.vxdcdc_alignment import settings as vxdcdc_alignment

#: Tells the automated system some details of this script
settings = CalibrationSettings(name="KLM alignmnent",
                               expert_username="chilikin",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=["raw_physics", "raw_cosmic"],
                               depends_on=[vxdcdc_alignment])

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
    file_to_iov_physics = input_data["raw_physics"]
    file_to_iov_cosmic = input_data["raw_cosmic"]

    # We might have requested an enormous amount of data across a run range.
    # There's a LOT more files than runs!
    # Lets set some limits because this calibration doesn't need that much to run.
    max_files_per_run = 5

    # If you are using Raw data there's a chance that input files could have zero events.
    # This causes a B2FATAL in basf2 RootInput so the collector job will fail.
    # Currently we don't have a good way of filtering this on the automated side, so we can check here.
    min_events_per_file = 1

    # We filter out any more than 2 files per run. The input data files are sorted alphabetically by b2caf-prompt-run
    # already. This procedure respects that ordering
    from prompt.utils import filter_by_max_files_per_run

    reduced_file_to_iov_physics = filter_by_max_files_per_run(file_to_iov_physics, max_files_per_run, min_events_per_file)
    input_files_physics = sorted(list(reduced_file_to_iov_physics.keys()))
    basf2.B2INFO(f"Total number of 'physics' files actually used as input = {len(input_files_physics)}")

    reduced_file_to_iov_cosmic = filter_by_max_files_per_run(file_to_iov_cosmic, max_files_per_run, min_events_per_file)
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

    import ROOT
    from ROOT import Belle2
    from ROOT.Belle2 import KLMChannelIndex, KLMElementNumbers
    from alignment import MillepedeCalibration, calibrate

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

    millepede.algo.setMinEntries(500000)
    millepede.algo.ignoreUndeterminedParams(True)
    millepede.algo.invertSign()

    ###################################################
    # Calibration setup

    from caf.framework import Calibration, Collection

    cal_klm = Calibration("KLMAlignment")

    ########
    # Collect on multiple input data types for one calibration

    from klm_calibration_utils import get_alignment_pre_collector_path_physics, get_alignment_pre_collector_path_cosmic

    if input_files_physics:
        coll_physics = get_collector("raw_physics")
        rec_path_physics = get_alignment_pre_collector_path_physics(entry_sequence="0:1000")

        collection_physics = Collection(collector=coll_physics,
                                        input_files=input_files_physics,
                                        pre_collector_path=rec_path_physics)

        cal_klm.add_collection(name="physics", collection=collection_physics)

    if input_files_cosmic:
        coll_cosmic = get_collector("raw_cosmic")
        rec_path_cosmic = get_alignment_pre_collector_path_cosmic(entry_sequence="0:1000")

        collection_cosmic = Collection(collector=coll_cosmic,
                                       input_files=input_files_cosmic,
                                       pre_collector_path=rec_path_cosmic)

        cal_klm.add_collection(name="cosmic",  collection=collection_cosmic)

    #####
    # Algorithm step config

    from caf.strategies import SequentialRunByRun

    cal_klm.algorithms = [millepede.algo]

    for algorithm in cal_klm.algorithms:
        algorithm.strategy = SequentialRunByRun
        algorithm.params = {"apply_iov": output_iov}

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
