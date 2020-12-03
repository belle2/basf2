# -*- coding: utf-8 -*-

"""ECL timing calibration that performs the crate calibrations, one for each physics run."""

from prompt import CalibrationSettings
from reconstruction import prepare_cdst_analysis

##############################
# REQUIRED VARIABLE #
##############################
# Used to identify the keys in input_data that your get_calibrations function will need in order
# to assign data correctly.
# Will be used to construct the calibration in the automated system, as well as set up the submission web forms.
# You can view the available input data formats from CalibrationSettings.allowed_data_formats

#: Tells the automated system some details of this script.
#     Default is to read in "hlt_bhabha" since we want to
#     run over cdst hlt_bhabha skim files.
settings = CalibrationSettings(name="ECL crate time calibrations",
                               expert_username="ehill",
                               description=__doc__,
                               input_data_formats=["cdst"],
                               input_data_names=["hlt_bhabha"],
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
    # The input data should be the hlt bhabha skim
    file_to_iov_physics = input_data["hlt_bhabha"]

    # We might have requested an enormous amount of data across a run range.
    # There's a LOT more files than runs!
    # Lets set some limits because this calibration doesn't need that much to run.
    max_files_per_run = 26

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
    # Collector setup
    root_input = register_module('RootInput')
    rec_path_bhabha = create_path()
    rec_path_bhabha.add_module(root_input)
    if 'Gearbox' not in rec_path_bhabha:
        rec_path_bhabha.add_module('Gearbox')
    if 'Geometry' not in rec_path_bhabha:
        rec_path_bhabha.add_module('Geometry', useDB=True)

    prepare_cdst_analysis(rec_path_bhabha)  # for new 2020 cdst format

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
                         pre_collector_path=rec_path_bhabha,
                         )

    ###################################################
    # Algorithm setup

    eclTAlg = Belle2.ECL.eclBhabhaTAlgorithm()

    # Define the CAF algorithm arguments
    # Set the cellIDLo to be larger than cellIDHi so that no crystal
    #    calibrations will be performed.
    eclTAlg.cellIDLo = 3
    eclTAlg.cellIDHi = 2
    eclTAlg.debugOutput = True
    eclTAlg.meanCleanRebinFactor = 3
    eclTAlg.meanCleanCutMinFactor = 0.3
    eclTAlg.debugFilenameBase = "eclBhabhaTAlgorithm"

    ###################################################
    # Calibration setup

    from caf.framework import Calibration

    cal_test = Calibration("ECLcrateTimeCalibration_physics")
    cal_test.add_collection(name="bhabha", collection=eclTCol)
    cal_test.algorithms = [eclTAlg]

    # Here we set the AlgorithmStrategy for our algorithm
    from caf.strategies import SimpleRunByRun

    # The SimpleRunByRun strategy executes your algorithm over runs
    # individually to give you payloads for each one (if successful)
    # It will not do any merging of runs which didn't contain enough data.
    # So failure is expected if your algorithm requires a large amount of data compared to run length.
    # You should only use granularity='run' for the collector when using this strategy.

    cal_test.strategies = SimpleRunByRun

    # Most other options like database chain and backend args will be overwritten by b2caf-prompt-run.
    # So we don't bother setting them.

    # You must return all calibrations you want to run in the prompt process, even if it's only one
    return [cal_test]

##############################
