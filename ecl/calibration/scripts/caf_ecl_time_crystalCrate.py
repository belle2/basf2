# -*- coding: utf-8 -*-

"""ECL timing calibration that performs the crystal and crate calibrations."""

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
settings = CalibrationSettings(name="ECL crystal and crate time calibrations",
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

    # Could remove this limit on the number of files per run but will just
    # set to a large number in case we want to introduce it later.
    # Also, keeping it allows the crystal calibrations code to look like the
    # crates calibration code.
    max_files_per_run = 1000

    # We filter addition files if there are more than [max_files_per_run] files per run.
    # The input data files are sorted alphabetically by b2caf-prompt-run
    # already. This procedure respects that ordering
    from prompt.utils import filter_by_max_files_per_run

    reduced_file_to_iov_physics = filter_by_max_files_per_run(file_to_iov_physics, max_files_per_run)
    input_files_physics = list(reduced_file_to_iov_physics.keys())
    basf2.B2INFO(f"Total number of files actually used as input = {len(input_files_physics)}")

    ###################################################
    from basf2 import register_module, create_path
    from ROOT import Belle2
    from caf.framework import Collection

    ###################################################
    # Collector setup for both crates and crystals

    root_input = register_module('RootInput')
    rec_path_bhabha = create_path()
    rec_path_bhabha.add_module(root_input)
    if 'Gearbox' not in rec_path_bhabha:
        rec_path_bhabha.add_module('Gearbox')
    if 'Geometry' not in rec_path_bhabha:
        rec_path_bhabha.add_module('Geometry', useDB=True)

    prepare_cdst_analysis(rec_path_bhabha)  # for new 2020 cdst format

    col_bhabha = register_module('ECLBhabhaTCollector')
    col_bhabha.param('timeAbsMax', 250)
    col_bhabha.param('minCrystal', 1)
    col_bhabha.param('maxCrystal', 8736)
    col_bhabha.param('saveTree', False)

    eclTCol = Collection(collector=col_bhabha,
                         input_files=input_files_physics,
                         pre_collector_path=rec_path_bhabha,
                         )

    ###################################################
    # Algorithm setup for crates

    eclTAlgCrates = Belle2.ECL.eclBhabhaTAlgorithm()

    # Define the CAF algorithm arguments
    # Set the cellIDLo to be larger than cellIDHi so that no crystal
    #    calibrations will be performed.
    eclTAlgCrates.cellIDLo = 3
    eclTAlgCrates.cellIDHi = 2
    eclTAlgCrates.debugOutput = True
    eclTAlgCrates.meanCleanRebinFactor = 3
    eclTAlgCrates.meanCleanCutMinFactor = 0.3
    eclTAlgCrates.debugFilenameBase = "eclBhabhaTAlgorithm"

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
    # Calibration setup for crates iteration 1

    from caf.framework import Calibration

    cal_crates_1 = Calibration("ECLcrateTimeCalibration_physics_1")
    cal_crates_1.add_collection(name="bhabha", collection=eclTCol)
    cal_crates_1.algorithms = [eclTAlgCrates]

    # Here we set the AlgorithmStrategy for our algorithm
    from caf.strategies import SimpleRunByRun

    # The SimpleRunByRun strategy executes your algorithm over runs
    # individually to give you payloads for each one (if successful)
    # It will not do any merging of runs which didn't contain enough data.
    # So failure is expected if your algorithm requires a large amount of data compared to run length.
    # You should only use granularity='run' for the collector when using this strategy.

    cal_crates_1.strategies = SimpleRunByRun

    # Most other options like database chain and backend args will be overwritten by b2caf-prompt-run.
    # So we don't bother setting them.

    ###################################################
    # Calibration setup for crystals iteration 1

    cal_crystals_1 = Calibration("ECLcrystalTimeCalibration_physics_1")
    cal_crystals_1.add_collection(name="bhabha", collection=eclTCol)
    cal_crystals_1.algorithms = [eclTAlgCrystals]

    # Here we set the AlgorithmStrategy for our algorithm
    from caf.strategies import SingleIOV

    # The default value is SingleIOV, you don't have to set this, it is done automatically.
    # SingleIOV just takes all of the runs as one big IoV and executes the algorithm once on all of their data.
    # You can use granularity='run' or granularity='all' for the collector when using this strategy.

    cal_crystals_1.strategies = SingleIOV

    ###################################################
    # Calibration setup for crates iteration 2
    cal_crates_2 = Calibration("ECLcrateTimeCalibration_physics_2")
    cal_crates_2.add_collection(name="bhabha", collection=eclTCol)
    cal_crates_2.algorithms = [eclTAlgCrates]
    cal_crates_2.strategies = SimpleRunByRun

    ###################################################
    # Calibration setup for crystals iteration 2
    cal_crystals_2 = Calibration("ECLcrystalTimeCalibration_physics_2")
    cal_crystals_2.add_collection(name="bhabha", collection=eclTCol)
    cal_crystals_2.algorithms = [eclTAlgCrystals]
    cal_crystals_2.strategies = SingleIOV

    ###################################################
    # Calibration setup for crates iteration 3
    cal_crates_3 = Calibration("ECLcrateTimeCalibration_physics_3")
    cal_crates_3.add_collection(name="bhabha", collection=eclTCol)
    cal_crates_3.algorithms = [eclTAlgCrates]
    cal_crates_3.strategies = SimpleRunByRun

    ###################################################
    # Calibration setup for crystals iteration 3
    cal_crystals_3 = Calibration("ECLcrystalTimeCalibration_physics_3")
    cal_crystals_3.add_collection(name="bhabha", collection=eclTCol)
    cal_crystals_3.algorithms = [eclTAlgCrystals]
    cal_crystals_3.strategies = SingleIOV

    ###################################################
    # Calibration setup for crates iteration 4
    cal_crates_4 = Calibration("ECLcrateTimeCalibration_physics_4")
    cal_crates_4.add_collection(name="bhabha", collection=eclTCol)
    cal_crates_4.algorithms = [eclTAlgCrates]
    cal_crates_4.strategies = SimpleRunByRun

    ###################################################
    # Calibration setup for crystals iteration 4
    cal_crystals_4 = Calibration("ECLcrystalTimeCalibration_physics_4")
    cal_crystals_4.add_collection(name="bhabha", collection=eclTCol)
    cal_crystals_4.algorithms = [eclTAlgCrystals]
    cal_crystals_4.strategies = SingleIOV

    ###################################################
    # Set up calibrations for crystals and crates to be
    # executed in order to converge the calibrations.
    # Instead of a loop, just create the calibration
    # instances manually with dependencies.

    # cal_crates_1 depends on the crystal payload values in the global tag
    cal_crystals_1.depends_on(cal_crates_1)

    cal_crates_2.depends_on(cal_crystals_1)
    cal_crystals_2.depends_on(cal_crates_2)

    cal_crates_3.depends_on(cal_crystals_2)
    cal_crystals_3.depends_on(cal_crates_3)

    cal_crates_4.depends_on(cal_crystals_3)
    cal_crystals_4.depends_on(cal_crates_4)

    ###################################################
    # Finalize all calibrations

    # You must return all calibrations you want to run in the prompt process, even if it's only one
    # Calibrations will be executed in this order as a result of the dependencies defined by the "dependes_on(...)".
    return [cal_crates_1, cal_crystals_1, cal_crates_2, cal_crystals_2, cal_crates_3, cal_crystals_3, cal_crates_4, cal_crystals_4]

##############################
