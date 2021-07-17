# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""ECL timing calibration that performs the crystal calibrations, one for  the whole set of runs."""

from prompt import CalibrationSettings
from reconstruction import *
from caf.utils import IoV


##############################
# REQUIRED VARIABLE #
##############################
# Used to identify the keys in input_data that your get_calibrations function will need in order
# to assign data correctly.
# Will be used to construct the calibration in the automated system, as well as set up the submission web forms.
# You can view the available input data formats from CalibrationSettings.allowed_data_formats

#: Tells the automated system some details of this script.
#     Default is to read in "bhabha_all_calib" since we want to
#     run over cdst bhabha_all_calib skim files.
settings = CalibrationSettings(
    name="ECL crystal time calibrations",
    expert_username="ehill",
    description=__doc__,
    input_data_formats=["cdst"],
    input_data_names=["bhabha_all_calib"],
    input_data_filters={
        "bhabha_all_calib": [
            "bhabha_all_calib",
            "4S",
            "Continuum",
            "Scan",
            "Good",
            "physics",
            "On"]},
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
    # The input data should be the bhabha skim
    file_to_iov_physics = input_data["bhabha_all_calib"]

    # Could remove this limit on the number of files per run but will just
    # set to a large number in case we want to introduce it later.
    # Also, keeping it allows the crystal calibrations code to look like the
    # crates calibration code.
    max_files_per_run = 2600

    # We filter addition files if there are more than [max_files_per_run] files per run.
    # The input data files are sorted alphabetically by b2caf-prompt-run
    # already. This procedure respects that ordering
    from prompt.utils import filter_by_max_files_per_run

    reduced_file_to_iov_physics = filter_by_max_files_per_run(file_to_iov_physics, max_files_per_run)
    input_files_physics = list(reduced_file_to_iov_physics.keys())
    basf2.B2INFO(f"Total number of files actually used as input = {len(input_files_physics)}")

    ###################################################
    import basf2
    from basf2 import register_module, create_path
    import ROOT
    from ROOT import Belle2
    from ROOT.Belle2 import TestCalibrationAlgorithm
    from caf.framework import Collection

    ###################################################
    # Collector setup for ecl crystals
    root_input = register_module('RootInput')
    rec_path_bhabha = create_path()
    rec_path_bhabha.add_module(root_input)
    if 'Gearbox' not in rec_path_bhabha:
        rec_path_bhabha.add_module('Gearbox')
    if 'Geometry' not in rec_path_bhabha:
        rec_path_bhabha.add_module('Geometry', useDB=True)

    prepare_cdst_analysis(rec_path_bhabha)    # for new 2020 cdst format

    # ====================================================
    t0BiasCorrection = -0.9  # Correct for the CDC t0 bias
    # ====================================================

    col_bhabha = register_module('ECLBhabhaTCollector')
    col_bhabha.param('timeAbsMax', 250)
    col_bhabha.param('minCrystal', 1)
    col_bhabha.param('maxCrystal', 8736)
    col_bhabha.param('saveTree', False)
    col_bhabha.param('hadronEventT0_TO_bhabhaEventT0_correction', t0BiasCorrection)

    eclTCol = Collection(collector=col_bhabha,
                         input_files=input_files_physics,
                         pre_collector_path=rec_path_bhabha)

    ###################################################
    # Algorithm setup for crystals

    eclTAlgCrystals = Belle2.ECL.eclBhabhaTAlgorithm()

    # Define the CAF algorithm arguments
    # Set the crateIDLo to be larger than crateIDHi so that no crate
    #    calibrations will be performed.
    eclTAlgCrystals.crateIDLo = 3
    eclTAlgCrystals.crateIDHi = 2
    eclTAlgCrystals.debugOutput = True
    eclTAlgCrystals.meanCleanRebinFactor = 3
    eclTAlgCrystals.meanCleanCutMinFactor = 0.3
    eclTAlgCrystals.debugFilenameBase = "eclBhabhaTAlgorithm"

    ###################################################
    # Calibration setup base for crystals

    from caf.framework import Calibration

    cal_crystals = Calibration("ECLcrystalTimeCalibration_physics")
    cal_crystals.add_collection(name="bhabha", collection=eclTCol)
    cal_crystals.algorithms = [eclTAlgCrystals]

    # Here we set the AlgorithmStrategy for our algorithm
    from caf.strategies import SingleIOV

    # The default value is SingleIOV, you don't have to set this, it is done automatically.
    # SingleIOV just takes all of the runs as one big IoV and executes the algorithm once on all of their data.
    # You can use granularity='run' or granularity='all' for the collector when using this strategy.

    cal_crystals.strategies = SingleIOV

    ###################################################
    # Include a merging Calibration that doesn't require input data but instead creates the final
    # payload from the previous calibration payloads.

    # We use a dummy collector that barely outputs any data and we set the input files to a single file so
    # we spawn only one very fast job.
    # It doesn't matter which input file we choose as the output is never used.

    merging_alg = Belle2.ECL.eclMergingCrystalTimingAlgorithm()
    cal_ecl_merge = Calibration(name="ecl_t_merge", collector="DummyCollector", algorithms=[merging_alg],
                                input_files=input_files_physics[:1])

    # The important part is that we depend on all the calibrations we previously ran
    cal_ecl_merge.depends_on(cal_crystals)

    # ..Uses cdst data so it requires prepare_cdst_analysis
    ecl_merge_pre_path = basf2.create_path()
    prepare_cdst_analysis(ecl_merge_pre_path, components=['ECL'])
    ecl_merge_pre_path.pre_collector_path = ecl_merge_pre_path

    # --------------------------------------------------------------
    # ..Force the output iovs to be open
    intermediate_iov = IoV(0, 0, -1, -1)
    requested_iov = kwargs.get("requested_iov", None)
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)
    for algorithm in cal_crystals.algorithms:
        algorithm.params = {"apply_iov": intermediate_iov}
    for algorithm in cal_ecl_merge.algorithms:
        algorithm.params = {"apply_iov": output_iov}

    ###################################################
    # Finalize all calibrations
    return [cal_crystals, cal_ecl_merge]
