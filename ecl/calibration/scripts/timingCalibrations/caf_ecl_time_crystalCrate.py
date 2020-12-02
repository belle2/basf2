# -*- coding: utf-8 -*-

"""ECL timing calibration that performs the crystal and crate calibrations,
   merges the relevant crystal payloads, and makes validation plots."""

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
#     Default is to read in "hlt_bhabha" since we want to
#     run over cdst hlt_bhabha skim files.
settings = CalibrationSettings(name="ECL crystal and crate time calibrations and validations",
                               expert_username="ehill",
                               description=__doc__,
                               input_data_formats=["cdst"],
                               input_data_names=["hlt_bhabha"],
                               depends_on=[],
                               expert_config={
                                              "crysCrateIterationLevel": 1
                                             })

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
    file_to_iov_bhabha = input_data["hlt_bhabha"]
    file_to_iov_hadron = input_data["hlt_hadron"]

    # Could remove this limit on the number of files per run but will just
    # set to a large number in case we want to introduce it later.
    # Also, keeping it allows the crystal calibrations code to look like the
    # crates calibration code.
    max_files_per_run_calibration = 26
    max_files_per_run_validation = 2
    max_events_per_run_plotting = 1

    # We filter addition files if there are more than [max_files_per_run] files per run.
    # The input data files are sorted alphabetically by b2caf-prompt-run
    # already. This procedure respects that ordering
    from prompt.utils import filter_by_max_files_per_run

    # We filter addition files if there are more than [max_events_per_run] events per run.
    # For plotting after the calibrations
    from prompt.utils import filter_by_max_events_per_run

    reduced_file_to_iov_bhabha = filter_by_max_files_per_run(file_to_iov_bhabha, max_files_per_run_calibration)
    input_files_bhabha = list(reduced_file_to_iov_bhabha.keys())
    basf2.B2INFO(f"Total number of bhabha files actually used as input for crates = {len(input_files_bhabha)}")

    input_files_bhabha_full = list(file_to_iov_bhabha.keys())
    basf2.B2INFO(f"Total number of bhabha files as input for crystals = {len(input_files_bhabha_full)}")

    reduced_file_to_iov_hadron = filter_by_max_files_per_run(file_to_iov_hadron, max_files_per_run_validation)
    input_files_hadron = list(reduced_file_to_iov_hadron.keys())
    basf2.B2INFO(f"Total number of hadron files actually used as input = {len(input_files_hadron)}")

    # For plotting the sum of the crate and crystal calibration
    # constants after the calibrations are made.  To be used to
    # monitor crate time jumps.
    reduced_file_to_iov_plotting = filter_by_max_events_per_run(file_to_iov_bhabha, max_events_per_run_plotting)
    input_files_plotting = list(reduced_file_to_iov_plotting.keys())

    # Determine how many iterations of the crystal calibrations and
    # crate calibrations to perform.
    expert_config = kwargs.get("expert_config")
    crysCrateIterationLevel = expert_config["crysCrateIterationLevel"]
    print("expert_config:  crysCrateIterationLevel = ", crysCrateIterationLevel)

    ###################################################
    from basf2 import register_module, create_path
    import ROOT
    from ROOT import Belle2
    from ROOT.Belle2 import TestCalibrationAlgorithm
    from caf.framework import Collection

    ###################################################
    # Collector setup for both crates and crystals calibrations

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
                         input_files=input_files_bhabha,
                         pre_collector_path=rec_path_bhabha)

    eclTCol_crys = Collection(collector=col_bhabha,
                              input_files=input_files_bhabha_full,
                              pre_collector_path=rec_path_bhabha)

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
    # Perform the calibration depending on the number
    # of iterations user selected.

    if crysCrateIterationLevel == 2:
        ###################################################
        # Do 3 crate and 3 crystal calibrations, validations,
        # and plot the crate time jumps.
        print("crysCrateIterationLevel == 2")
        print("Performing these calibration iterations:  crate 1 -> crystal 1 -> crate 2 -> crystal 2 -> crate 3 -> crystal 3")

        ###################################################
        # Calibration setup for crates iteration 1

        from caf.framework import Calibration

        cal_crates_1 = Calibration("ECLcrateTimeCalibration_physics_1")
        cal_crates_1.add_collection(name="bhabha", collection=eclTCol)
        cal_crates_1.algorithms = [eclTAlgCrates]
        cal_crates_1.save_payloads = False

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
        cal_crystals_1.add_collection(name="bhabha", collection=eclTCol_crys)
        cal_crystals_1.algorithms = [eclTAlgCrystals]

        # Two payloads are created here:
        #    * ECLCrystalTimeOffset
        #    * ECLCrystalTimeOffsetBhabha
        # We technically want to save ECLCrystalTimeOffsetBhabha to the GT but because we don't want
        # ECLCrystalTimeOffset we save neither here and get the merging algorithm to save a copy
        # of ECLCrystalTimeOffsetBhabha later.
        cal_crystals_1.save_payloads = False

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
        cal_crates_2.save_payloads = False

        ###################################################
        # Calibration setup for crystals iteration 2

        cal_crystals_2 = Calibration("ECLcrystalTimeCalibration_physics_2")
        cal_crystals_2.add_collection(name="bhabha", collection=eclTCol_crys)
        cal_crystals_2.algorithms = [eclTAlgCrystals]

        # Two payloads are created here:
        #    * ECLCrystalTimeOffset
        #    * ECLCrystalTimeOffsetBhabha
        # We technically want to save ECLCrystalTimeOffsetBhabha to the GT but because we don't want
        # ECLCrystalTimeOffset we save neither here and get the merging algorithm to save a copy
        # of ECLCrystalTimeOffsetBhabha later.
        cal_crystals_2.save_payloads = False

        # Here we set the AlgorithmStrategy for our algorithm
        from caf.strategies import SingleIOV

        # The default value is SingleIOV, you don't have to set this, it is done automatically.
        # SingleIOV just takes all of the runs as one big IoV and executes the algorithm once on all of their data.
        # You can use granularity='run' or granularity='all' for the collector when using this strategy.

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
        cal_crystals_3.add_collection(name="bhabha", collection=eclTCol_crys)
        cal_crystals_3.algorithms = [eclTAlgCrystals]

        # Two payloads are created here:
        #    * ECLCrystalTimeOffset
        #    * ECLCrystalTimeOffsetBhabha
        # We technically want to save ECLCrystalTimeOffsetBhabha to the GT but because we don't want
        # ECLCrystalTimeOffset we save neither here and get the merging algorithm to save a copy
        # of ECLCrystalTimeOffsetBhabha later.
        cal_crystals_3.save_payloads = False

        # Here we set the AlgorithmStrategy for our algorithm
        from caf.strategies import SingleIOV

        # The default value is SingleIOV, you don't have to set this, it is done automatically.
        # SingleIOV just takes all of the runs as one big IoV and executes the algorithm once on all of their data.
        # You can use granularity='run' or granularity='all' for the collector when using this strategy.

        cal_crystals_3.strategies = SingleIOV

        ###################################################
        ###################################################
        # Include a merging Calibration that doesn't require input data but instead creates a
        # payload from the previous calibration payloads.

        # We use a dummy collector that barely outputs any data and we set the input files to a single file so
        # we spawn only one very fast job.
        # It doesn't matter which input file we choose as the output is never used.

        merging_alg_1 = Belle2.ECL.eclMergingCrystalTimingAlgorithm()
        cal_ecl_merge_1 = Calibration(name="ecl_t_merge_1", collector="DummyCollector", algorithms=[merging_alg_1],
                                      input_files=input_files_bhabha[:1])

        cal_ecl_merge_1.save_payloads = False

        # ..Uses cdst data so it requires prepare_cdst_analysis
        ecl_merge_pre_path_1 = basf2.create_path()
        prepare_cdst_analysis(ecl_merge_pre_path_1, components=['ECL'])
        ecl_merge_pre_path_1.pre_collector_path = ecl_merge_pre_path_1

        ###################################################
        # Include a merging Calibration that doesn't require input data but instead creates the "final"
        # payload from the previous calibration payloads.  This one has a broad iov again so we don't
        # have to worry about the user giving it the wrong iov.

        # We use a dummy collector that barely outputs any data and we set the input files to a single file so
        # we spawn only one very fast job.
        # It doesn't matter which input file we choose as the output is never used.

        merging_alg_2 = Belle2.ECL.eclMergingCrystalTimingAlgorithm()
        cal_ecl_merge_2 = Calibration(name="ecl_t_merge_2", collector="DummyCollector", algorithms=[merging_alg_2],
                                      input_files=input_files_bhabha[:1])

        cal_ecl_merge_2.save_payloads = False

        # ..Uses cdst data so it requires prepare_cdst_analysis
        ecl_merge_pre_path_2 = basf2.create_path()
        prepare_cdst_analysis(ecl_merge_pre_path_2, components=['ECL'])
        ecl_merge_pre_path_2.pre_collector_path = ecl_merge_pre_path_2

        ###################################################
        # Include a merging Calibration that doesn't require input data but instead creates the final
        # payload from the final crystal calibration payloads.  This one has the final iov as
        # defined by the user.

        # We use a dummy collector that barely outputs any data and we set the input files to a single file so
        # we spawn only one very fast job.
        # It doesn't matter which input file we choose as the output is never used.

        merging_alg_3 = Belle2.ECL.eclMergingCrystalTimingAlgorithm()
        cal_ecl_merge_3 = Calibration(name="ecl_t_merge_3", collector="DummyCollector", algorithms=[merging_alg_3],
                                      input_files=input_files_bhabha[:1])

        # ..Uses cdst data so it requires prepare_cdst_analysis
        ecl_merge_pre_path_3 = basf2.create_path()
        prepare_cdst_analysis(ecl_merge_pre_path_3, components=['ECL'])
        ecl_merge_pre_path_3.pre_collector_path = ecl_merge_pre_path_3

        # --------------------------------------------------------------
        # ..Force the output iovs to be open
        intermediate_iov = IoV(0, 0, -1, -1)
        requested_iov = kwargs.get("requested_iov", None)
        output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)
        for algorithm in cal_crystals_1.algorithms:
            algorithm.params = {"apply_iov": intermediate_iov}
        for algorithm in cal_crystals_2.algorithms:
            algorithm.params = {"apply_iov": intermediate_iov}
        for algorithm in cal_ecl_merge_1.algorithms:
            algorithm.params = {"apply_iov": intermediate_iov}
        for algorithm in cal_ecl_merge_2.algorithms:
            algorithm.params = {"apply_iov": intermediate_iov}

        # the final crystal payload is given the user's iov
        # cal_ecl_merge_3 is the same as cal_ecl_merge_2 except for the iov.
        for algorithm in cal_ecl_merge_3.algorithms:
            algorithm.params = {"apply_iov": output_iov}

        #############################################################
        #############################################################
        # Collector setup for ecl timing validation with bhabhas
        root_input = register_module('RootInput')
        rec_path_bhabha_val = create_path()
        rec_path_bhabha_val.add_module(root_input)
        if 'Gearbox' not in rec_path_bhabha_val:
            rec_path_bhabha_val.add_module('Gearbox')
        if 'Geometry' not in rec_path_bhabha_val:
            rec_path_bhabha_val.add_module('Geometry', useDB=True)

        prepare_cdst_analysis(rec_path_bhabha_val)    # for new 2020 cdst format

        col_bhabha_val = register_module('eclBhabhaTimeCalibrationValidationCollector')
        col_bhabha_val.param('timeAbsMax', 70)
        col_bhabha_val.param('saveTree', False)

        eclValTCol = Collection(collector=col_bhabha_val,
                                input_files=input_files_bhabha,
                                pre_collector_path=rec_path_bhabha_val)

        ##############################################################
        # Algorithm setup for ecl timing validation with bhabhas

        # Give the collector name to the algorithm since one algorithm
        # is used to analyse the results from several possible collectors
        eclValTAlgBhabha = Belle2.ECL.eclTValidationAlgorithm("eclBhabhaTimeCalibrationValidationCollector")

        # Define the CAF algorithm arguments
        # eclValTAlgBhabha.cellIDLo= 3
        # eclValTAlgBhabha.cellIDHi = 2
        eclValTAlgBhabha.meanCleanRebinFactor = 3
        eclValTAlgBhabha.meanCleanCutMinFactor = 0.4
        eclValTAlgBhabha.debugFilenameBase = "eclBhabhaTValidationAlgorithm"

        ####################################################################
        # Calibration setup base for ecl timing validation with bhabhas

        from caf.framework import Calibration

        valid_cal_bhabha = Calibration("ECLcrystalTimeCalValidation_bhabhaPhysics")
        valid_cal_bhabha.add_collection(name="bhabha", collection=eclValTCol)
        valid_cal_bhabha.algorithms = [eclValTAlgBhabha]
        valid_cal_bhabha.save_payloads = False

        # Here we set the AlgorithmStrategy for our algorithm
        from caf.strategies import SingleIOV

        # The default value is SingleIOV, you don't have to set this, it is done automatically.
        # SingleIOV just takes all of the runs as one big IoV and executes the algorithm once on all of their data.
        # You can use granularity='run' or granularity='all' for the collector when using this strategy.

        valid_cal_bhabha.strategies = SingleIOV

        #############################################################
        #############################################################
        # Collector setup for ecl timing validation with hadrons
        root_input = register_module('RootInput')
        rec_path_hadron_val = create_path()
        rec_path_hadron_val.add_module(root_input)
        if 'Gearbox' not in rec_path_hadron_val:
            rec_path_hadron_val.add_module('Gearbox')
        if 'Geometry' not in rec_path_hadron_val:
            rec_path_hadron_val.add_module('Geometry', useDB=True)

        prepare_cdst_analysis(rec_path_hadron_val)    # for new 2020 cdst format

        col_hadron_val = register_module('eclHadronTimeCalibrationValidationCollector')
        col_hadron_val.param('timeAbsMax', 70)
        col_hadron_val.param('saveTree', False)

        eclValTCol = Collection(collector=col_hadron_val,
                                input_files=input_files_hadron,
                                pre_collector_path=rec_path_hadron_val)

        ##############################################################
        # Algorithm setup for ecl timing validation with hadrons

        # Give the collector name to the algorithm since one algorithm
        # is used to analyse the results from several possible collectors
        eclValTAlgHadronic = Belle2.ECL.eclTValidationAlgorithm("eclHadronTimeCalibrationValidationCollector")

        # Define the CAF algorithm arguments
        # eclValTAlgHadronic.cellIDLo= 3
        # eclValTAlgHadronic.cellIDHi = 2
        eclValTAlgHadronic.meanCleanRebinFactor = 3
        eclValTAlgHadronic.meanCleanCutMinFactor = 0.4
        eclValTAlgHadronic.debugFilenameBase = "eclHadronTValidationAlgorithm"

        ####################################################################
        # Calibration setup base for ecl timing validation with hadrons

        from caf.framework import Calibration

        valid_cal_hadron = Calibration("ECLcrystalTimeCalValidation_hadronPhysics")
        valid_cal_hadron.add_collection(name="hadron", collection=eclValTCol)
        valid_cal_hadron.algorithms = [eclValTAlgHadronic]
        valid_cal_hadron.save_payloads = False

        # Here we set the AlgorithmStrategy for our algorithm
        from caf.strategies import SingleIOV

        # The default value is SingleIOV, you don't have to set this, it is done automatically.
        # SingleIOV just takes all of the runs as one big IoV and executes the algorithm once on all of their data.
        # You can use granularity='run' or granularity='all' for the collector when using this strategy.

        valid_cal_hadron.strategies = SingleIOV

        #######################################################################
        #######################################################################
        # Collector setup for the calibration constant plotting

        # Set up the collector but with only one event per file
        root_input = register_module('RootInput', entrySequences=['0:{}'.format(1)])

        rec_path_bhabha_plotting = create_path()
        rec_path_bhabha_plotting.add_module(root_input)
        if 'Gearbox' not in rec_path_bhabha_plotting:
            rec_path_bhabha_plotting.add_module('Gearbox')
        if 'Geometry' not in rec_path_bhabha_plotting:
            rec_path_bhabha_plotting.add_module('Geometry', useDB=True)

        prepare_cdst_analysis(rec_path_bhabha_plotting)  # for new 2020 cdst format

        col_bhabha_plotting = register_module('eclTimeShiftsPlottingCollector')
        eclTCol = Collection(collector=col_bhabha_plotting,
                             input_files=input_files_plotting,
                             pre_collector_path=rec_path_bhabha_plotting,
                             )

        # Set up the plotting.  Use the two files with the collector to
        # determine the run range to plot.  The plotting is done in the
        # algorithm

        tShifts_alg = Belle2.ECL.eclTimeShiftsAlgorithm()
        tShifts_alg.debugFilenameBase = "eclTimeShiftsAlgorithm"

        # +-30ns range allows for unexpectedly large jumps
        tShifts_alg.crysCrateShift_min = -30   # in ns
        tShifts_alg.crysCrateShift_max = 30    # in ns

        # Make the algorithm loop over the runs, not just the collector
        # tShifts_alg.algorithmReadPayloads = True
        tShifts_alg.forcePayloadIOVnotOpenEndedAndSequentialRevision = True

        cal_ecl_timeShifts = Calibration(name="ecl_t_shifts", algorithms=[tShifts_alg],
                                         input_files=input_files_plotting)
        cal_ecl_timeShifts.add_collection(name="bhabha", collection=eclTCol)

        cal_ecl_timeShifts.save_payloads = False

        #######################################################################
        #######################################################################
        # Set up calibrations for crystals and crates to be executed in order
        # to converge the calibrations.  Instead of a loop, just create the
        # calibration instances manually with dependencies.  Once these are
        # done, get the validations to execute.

        # cal_crates_1 depends on the crystal payload values in the global tag
        cal_crystals_1.depends_on(cal_crates_1)
        cal_ecl_merge_1.depends_on(cal_crystals_1)
        cal_crates_2.depends_on(cal_ecl_merge_1)
        cal_crystals_2.depends_on(cal_crates_2)
        cal_ecl_merge_2.depends_on(cal_crystals_2)
        cal_crates_3.depends_on(cal_ecl_merge_2)
        cal_crystals_3.depends_on(cal_crates_3)

        # cal_ecl_merge_3 - uses user iov
        cal_ecl_merge_3.depends_on(cal_crystals_3)

        # The validations can run independently of each other but rely on the
        # last calibration step
        # valid_cal_bhabha.depends_on(cal_crystals_3)
        # valid_cal_hadron.depends_on(cal_crystals_3)
        valid_cal_bhabha.depends_on(cal_ecl_merge_3)
        valid_cal_hadron.depends_on(cal_ecl_merge_3)

        # Plotting
        # cal_ecl_timeShifts.depends_on(cal_crystals_3)
        cal_ecl_timeShifts.depends_on(cal_ecl_merge_3)  # CHANGE SO THAT I CAN COMPARE PAYLOADS FROM PREVIOUS BUCKETS

        ###################################################
        # Finalize all calibrations

        # You must return all calibrations you want to run in the prompt process, even if it's only one
        # Calibrations will be executed in this order as a result of the dependencies defined by the "dependes_on(...)".
        return [cal_crates_1, cal_crystals_1, cal_ecl_merge_1,
                cal_crates_2, cal_crystals_2, cal_ecl_merge_2,
                cal_crates_3, cal_crystals_3, cal_ecl_merge_3,
                valid_cal_bhabha, valid_cal_hadron, cal_ecl_timeShifts]

    else:
        ###################################################
        # Do 2 crate and 1 crystal calibrations, validations,
        # and plot the crate time jumps.

        print("crysCrateIterationLevel value resulting in default calibration iteration set up")
        print("Performing these calibration iterations:  crate -> crystal -> crate")

        ###################################################
        # Calibration setup for crates iteration 1

        from caf.framework import Calibration

        cal_crates_1 = Calibration("ECLcrateTimeCalibration_physics_1")
        cal_crates_1.add_collection(name="bhabha", collection=eclTCol)
        cal_crates_1.algorithms = [eclTAlgCrates]
        cal_crates_1.save_payloads = False

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
        cal_crystals_1.add_collection(name="bhabha", collection=eclTCol_crys)
        cal_crystals_1.algorithms = [eclTAlgCrystals]

        # Two payloads are created here:
        #    * ECLCrystalTimeOffset
        #    * ECLCrystalTimeOffsetBhabha
        # We technically want to save ECLCrystalTimeOffsetBhabha to the GT but because we don't want
        # ECLCrystalTimeOffset we save neither here and get the merging algorithm to save a copy
        # of ECLCrystalTimeOffsetBhabha later.
        cal_crystals_1.save_payloads = False

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
        ###################################################
        # Include a merging Calibration that doesn't require input data but instead creates the final
        # payload from the previous calibration payloads.

        # We use a dummy collector that barely outputs any data and we set the input files to a single file so
        # we spawn only one very fast job.
        # It doesn't matter which input file we choose as the output is never used.

        merging_alg = Belle2.ECL.eclMergingCrystalTimingAlgorithm()
        cal_ecl_merge = Calibration(name="ecl_t_merge", collector="DummyCollector", algorithms=[merging_alg],
                                    input_files=input_files_bhabha[:1])

        # The important part is that we depend on all the calibrations we previously ran
        cal_ecl_merge.depends_on(cal_crystals_1)

        # ..Uses cdst data so it requires prepare_cdst_analysis
        ecl_merge_pre_path = basf2.create_path()
        prepare_cdst_analysis(ecl_merge_pre_path, components=['ECL'])
        ecl_merge_pre_path.pre_collector_path = ecl_merge_pre_path

        # --------------------------------------------------------------
        # ..Force the output iovs to be open
        intermediate_iov = IoV(0, 0, -1, -1)
        requested_iov = kwargs.get("requested_iov", None)
        output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)
        for algorithm in cal_crystals_1.algorithms:
            algorithm.params = {"apply_iov": intermediate_iov}
        for algorithm in cal_ecl_merge.algorithms:
            algorithm.params = {"apply_iov": output_iov}

        #############################################################
        #############################################################
        # Collector setup for ecl timing validation with bhabhas
        root_input = register_module('RootInput')
        rec_path_bhabha_val = create_path()
        rec_path_bhabha_val.add_module(root_input)
        if 'Gearbox' not in rec_path_bhabha_val:
            rec_path_bhabha_val.add_module('Gearbox')
        if 'Geometry' not in rec_path_bhabha_val:
            rec_path_bhabha_val.add_module('Geometry', useDB=True)

        prepare_cdst_analysis(rec_path_bhabha_val)    # for new 2020 cdst format

        col_bhabha_val = register_module('eclBhabhaTimeCalibrationValidationCollector')
        col_bhabha_val.param('timeAbsMax', 70)
        col_bhabha_val.param('saveTree', False)

        eclValTCol = Collection(collector=col_bhabha_val,
                                input_files=input_files_bhabha,
                                pre_collector_path=rec_path_bhabha_val)

        ##############################################################
        # Algorithm setup for ecl timing validation with bhabhas

        # Give the collector name to the algorithm since one algorithm
        # is used to analyse the results from several possible collectors
        eclValTAlgBhabha = Belle2.ECL.eclTValidationAlgorithm("eclBhabhaTimeCalibrationValidationCollector")

        # Define the CAF algorithm arguments
        # eclValTAlgBhabha.cellIDLo= 3
        # eclValTAlgBhabha.cellIDHi = 2
        eclValTAlgBhabha.meanCleanRebinFactor = 3
        eclValTAlgBhabha.meanCleanCutMinFactor = 0.4
        eclValTAlgBhabha.debugFilenameBase = "eclBhabhaTValidationAlgorithm"

        ####################################################################
        # Calibration setup base for ecl timing validation with bhabhas

        from caf.framework import Calibration

        valid_cal_bhabha = Calibration("ECLcrystalTimeCalValidation_bhabhaPhysics")
        valid_cal_bhabha.add_collection(name="bhabha", collection=eclValTCol)
        valid_cal_bhabha.algorithms = [eclValTAlgBhabha]
        valid_cal_bhabha.save_payloads = False

        # Here we set the AlgorithmStrategy for our algorithm
        from caf.strategies import SingleIOV

        # The default value is SingleIOV, you don't have to set this, it is done automatically.
        # SingleIOV just takes all of the runs as one big IoV and executes the algorithm once on all of their data.
        # You can use granularity='run' or granularity='all' for the collector when using this strategy.

        valid_cal_bhabha.strategies = SingleIOV

        #############################################################
        #############################################################
        # Collector setup for ecl timing validation with hadrons
        root_input = register_module('RootInput')
        rec_path_hadron_val = create_path()
        rec_path_hadron_val.add_module(root_input)
        if 'Gearbox' not in rec_path_hadron_val:
            rec_path_hadron_val.add_module('Gearbox')
        if 'Geometry' not in rec_path_hadron_val:
            rec_path_hadron_val.add_module('Geometry', useDB=True)

        prepare_cdst_analysis(rec_path_hadron_val)    # for new 2020 cdst format

        col_hadron_val = register_module('eclHadronTimeCalibrationValidationCollector')
        col_hadron_val.param('timeAbsMax', 70)
        col_hadron_val.param('saveTree', False)

        eclValTCol = Collection(collector=col_hadron_val,
                                input_files=input_files_hadron,
                                pre_collector_path=rec_path_hadron_val)

        ##############################################################
        # Algorithm setup for ecl timing validation with hadrons

        # Give the collector name to the algorithm since one algorithm
        # is used to analyse the results from several possible collectors
        eclValTAlgHadronic = Belle2.ECL.eclTValidationAlgorithm("eclHadronTimeCalibrationValidationCollector")

        # Define the CAF algorithm arguments
        # eclValTAlgHadronic.cellIDLo= 3
        # eclValTAlgHadronic.cellIDHi = 2
        eclValTAlgHadronic.meanCleanRebinFactor = 3
        eclValTAlgHadronic.meanCleanCutMinFactor = 0.4
        eclValTAlgHadronic.debugFilenameBase = "eclHadronTValidationAlgorithm"

        ####################################################################
        # Calibration setup base for ecl timing validation with hadrons

        from caf.framework import Calibration

        valid_cal_hadron = Calibration("ECLcrystalTimeCalValidation_hadronPhysics")
        valid_cal_hadron.add_collection(name="hadron", collection=eclValTCol)
        valid_cal_hadron.algorithms = [eclValTAlgHadronic]
        valid_cal_hadron.save_payloads = False

        # Here we set the AlgorithmStrategy for our algorithm
        from caf.strategies import SingleIOV

        # The default value is SingleIOV, you don't have to set this, it is done automatically.
        # SingleIOV just takes all of the runs as one big IoV and executes the algorithm once on all of their data.
        # You can use granularity='run' or granularity='all' for the collector when using this strategy.

        valid_cal_hadron.strategies = SingleIOV

        #######################################################################
        #######################################################################
        # Collector setup for the calibration constant plotting

        # Set up the collector but with only one event per file
        root_input = register_module('RootInput', entrySequences=['0:{}'.format(1)])

        rec_path_bhabha_plotting = create_path()
        rec_path_bhabha_plotting.add_module(root_input)
        if 'Gearbox' not in rec_path_bhabha_plotting:
            rec_path_bhabha_plotting.add_module('Gearbox')
        if 'Geometry' not in rec_path_bhabha_plotting:
            rec_path_bhabha_plotting.add_module('Geometry', useDB=True)

        prepare_cdst_analysis(rec_path_bhabha_plotting)  # for new 2020 cdst format

        col_bhabha_plotting = register_module('eclTimeShiftsPlottingCollector')
        eclTCol = Collection(collector=col_bhabha_plotting,
                             input_files=input_files_plotting,
                             pre_collector_path=rec_path_bhabha_plotting,
                             )

        # Set up the plotting.  Use the two files with the collector to
        # determine the run range to plot.  The plotting is done in the
        # algorithm

        tShifts_alg = Belle2.ECL.eclTimeShiftsAlgorithm()
        tShifts_alg.debugFilenameBase = "eclTimeShiftsAlgorithm"

        # +-30ns range allows for unexpectedly large jumps
        tShifts_alg.crysCrateShift_min = -30   # in ns
        tShifts_alg.crysCrateShift_max = 30    # in ns

        # Make the algorithm loop over the runs, not just the collector
        # tShifts_alg.algorithmReadPayloads = True
        tShifts_alg.forcePayloadIOVnotOpenEndedAndSequentialRevision = True

        cal_ecl_timeShifts = Calibration(name="ecl_t_shifts", algorithms=[tShifts_alg],
                                         input_files=input_files_plotting)
        cal_ecl_timeShifts.add_collection(name="bhabha", collection=eclTCol)


#         cal_ecl_timeShifts = Calibration(name="ecl_t_shifts", collector="DummyCollector",
#                                          algorithms=[tShifts_alg], input_files=input_files_first_last)
        cal_ecl_timeShifts.save_payloads = False

        #######################################################################
        #######################################################################
        # Set up calibrations for crystals and crates to be executed in order
        # to converge the calibrations.  Instead of a loop, just create the
        # calibration instances manually with dependencies.  Once these are
        # done, get the validations to execute.

        # cal_crates_1 depends on the crystal payload values in the global tag
        cal_crystals_1.depends_on(cal_crates_1)
        cal_ecl_merge.depends_on(cal_crystals_1)
        cal_crates_2.depends_on(cal_ecl_merge)

        # The validations can run independently of each other but rely on the
        # last calibration step
        valid_cal_bhabha.depends_on(cal_crates_2)
        valid_cal_hadron.depends_on(cal_crates_2)

        # Plotting
        cal_ecl_timeShifts.depends_on(cal_crates_2)

        ###################################################
        # Finalize all calibrations

        # You must return all calibrations you want to run in the prompt process, even if it's only one
        # Calibrations will be executed in this order as a result of the dependencies defined by the "dependes_on(...)".
        return [cal_crates_1, cal_crystals_1, cal_ecl_merge, cal_crates_2,
                valid_cal_bhabha, valid_cal_hadron, cal_ecl_timeShifts]

##############################
