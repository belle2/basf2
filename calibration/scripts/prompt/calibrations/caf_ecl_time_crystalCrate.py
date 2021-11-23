# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""ECL timing calibration that performs the crystal and crate calibrations,
   merges the relevant crystal payloads, and makes validation plots.  It is the
   main script for executing the ECL timing calibrations."""

from prompt import CalibrationSettings, INPUT_DATA_FILTERS
from reconstruction import prepare_cdst_analysis, prepare_user_cdst_analysis
from caf.utils import IoV, ExpRun
import copy

##############################
# REQUIRED VARIABLE #
##############################
# Used to identify the keys in input_data that your get_calibrations function will need in order
# to assign data correctly.
# Will be used to construct the calibration in the automated system, as well as set up the submission web forms.
# You can view the available input data formats from CalibrationSettings.allowed_data_formats

#: Tells the automated system some details of this script.
#     Default is to read in "bhabha_all_calib" since we want to
#     run over cdst bhabha_all_calib skim files.  Also load in
#     the hadron skim for validations.
settings = CalibrationSettings(
    name="ECL crystal and crate time calibrations and validations",
    expert_username="ehill",
    description=__doc__,
    input_data_formats=["cdst"],
    input_data_names=["bhabha_all_calib", "hadron_calib"],
    input_data_filters={"bhabha_all_calib": [INPUT_DATA_FILTERS["Data Tag"]["bhabha_all_calib"],
                                             INPUT_DATA_FILTERS["Beam Energy"]["4S"],
                                             INPUT_DATA_FILTERS["Beam Energy"]["Continuum"],
                                             INPUT_DATA_FILTERS["Beam Energy"]["Scan"],
                                             INPUT_DATA_FILTERS["Data Quality Tag"]["Good"],
                                             INPUT_DATA_FILTERS["Run Type"]["physics"],
                                             INPUT_DATA_FILTERS["Magnet"]["On"]],
                        "hadron_calib": [INPUT_DATA_FILTERS["Data Tag"]["hadron_calib"],
                                         INPUT_DATA_FILTERS["Beam Energy"]["4S"],
                                         INPUT_DATA_FILTERS["Beam Energy"]["Continuum"],
                                         INPUT_DATA_FILTERS["Beam Energy"]["Scan"],
                                         INPUT_DATA_FILTERS["Data Quality Tag"]["Good"],
                                         INPUT_DATA_FILTERS["Run Type"]["physics"],
                                         INPUT_DATA_FILTERS["Magnet"]["On"]]},
    depends_on=[],
    expert_config={"numCrysCrateIterations": 1,
                   "maxNumberEvents": 50000000,
                   "payload_boundaries": [],
                   "t0_bhabhaToHadron_correction": 0})


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

    # Extract expert config variables input from command line (or from
    # the calibrations submission website via the command line).
    expert_config = kwargs.get("expert_config")

    # Determine how many iterations of the crystal calibrations and
    # crate calibrations to perform.
    numCrysCrateIterations = expert_config["numCrysCrateIterations"]
    print("expert_config:  numCrysCrateIterations = ", numCrysCrateIterations)

    # Set the maximum number of events to include per crystal calibration
    # NOTE: THIS IS NOT QUITE RIGHT SINCE THIS MAXIMUM DOESN'T WORK WITH
    # THE sequentialBoundaries SET UP FOR SEPARATE RUN NUMBER REGIONS FOR
    # CRYSTAL CALIBRATIONS !!  BUT THIS WILL WORK WELL IF THERE IS ONLY ONE
    # RUN RANGE REGION FOR THE CRYSTAL CALIBRATIONS.
    maxNumEventsCrystalCalib = expert_config["maxNumberEvents"]
    print("expert_config:  maxNumEventsCrystalCalib = ", maxNumEventsCrystalCalib)

    # Determine how large of an offset should be applied to correct for
    # differences in the CDC event t0 in bhabha and hadronic events
    t0_bhabhaToHadron_correction = expert_config["t0_bhabhaToHadron_correction"]
    print("expert_config:  t0_bhabhaToHadron_correction = ", t0_bhabhaToHadron_correction)

    from caf.strategies import SimpleRunByRun, SingleIOV, SequentialBoundaries
    import numpy as np

    # Determine the user defined calibration boundaries.  Most useful
    # for when multiple experiments are processed at the same time.
    # Useful for the crystal calibrations but not for the crate
    # calibrations, which are done run-by-run.
    payload_boundaries = []  # Introduce empty list of payload boundaries.
    payload_boundaries.extend([ExpRun(*boundary) for boundary in expert_config["payload_boundaries"]])
    basf2.B2INFO(f"Expert set payload boundaries are: {expert_config['payload_boundaries']}")
    print("payload_boundaries = ", payload_boundaries)

    # In this script we want to use one sources of input data.
    # Get the input files from the input_data variable
    # The input data should be the bhabha skim for the calibrations
    # and the hadron skim for the validations.
    file_to_iov_bhabha = input_data["bhabha_all_calib"]
    file_to_iov_hadron = input_data["hadron_calib"]

    # Set the maximum limits on the number of files/events per run.
    # If I decided to remove the limit on the number of files per run, it is just easier
    # to set it to a large number in case we want to introduce it later.
    # Also, keeping it allows the crystal calibrations code to look like the
    # crates calibration code even though they don't use the same list of input files.
    max_files_per_run_calibration = 26
    max_files_per_run_validation = 4
    max_events_per_run_plotting = 1

    setMaxEventsPerCrystalCalib = True
    max_events_per_crystalCalib = 50000000
    if (maxNumEventsCrystalCalib == -1):
        setMaxEventsPerCrystalCalib = False
    elif (maxNumEventsCrystalCalib > 0):
        max_events_per_crystalCalib = maxNumEventsCrystalCalib

    print("max_events_per_crystalCalib = ", max_events_per_crystalCalib)

    # We filter addition files if there are more than [max_files_per_run] files per run.
    # The input data files are sorted alphabetically by b2caf-prompt-run
    # already. This procedure respects that ordering
    from prompt.utils import filter_by_max_files_per_run

    # We filter addition files if there are more than [max_events_per_run] events per run.
    # For plotting after the calibrations
    from prompt.utils import filter_by_max_events_per_run

    # We filter addition files if there are more than [max_events_per_calibration] events per crystal calibration.
    # For doing the crystal calibrations
    from prompt.utils import filter_by_select_max_events_from_files

    reduced_file_to_iov_bhabha = filter_by_max_files_per_run(file_to_iov_bhabha, max_files_per_run_calibration)
    input_files_bhabha = list(reduced_file_to_iov_bhabha.keys())
    basf2.B2INFO(f"Total number of bhabha files actually used as input for crates = {len(input_files_bhabha)}")

    reduced_file_to_iov_bhabha_1perRun = filter_by_max_files_per_run(file_to_iov_bhabha, 1)
    input_files_bhabha_1perRun = list(reduced_file_to_iov_bhabha_1perRun.keys())
    basf2.B2INFO(f"Total number of bhabha files actually used for dummy collectors = {len(input_files_bhabha_1perRun)}")

    input_files_bhabha_full = list(file_to_iov_bhabha.keys())
    basf2.B2INFO(f"Total number of bhabha files as input = {len(input_files_bhabha_full)}")

    if (setMaxEventsPerCrystalCalib):
        temp_input_files_bhabha_full = input_files_bhabha_full.copy()
        input_files_bhabha_crystal_maxEvents = filter_by_select_max_events_from_files(
            temp_input_files_bhabha_full, max_events_per_crystalCalib)
        basf2.B2INFO("Total number of bhabha files that could be used as input for crystal calibration ...")
        basf2.B2INFO(f"(ignoring payload boundaries) if a max # events is used = {len(input_files_bhabha_crystal_maxEvents)}")

        if not input_files_bhabha_crystal_maxEvents:
            input_files_bhabha_crystal_maxEvents = input_files_bhabha_full.copy()
            print("Number of events < maximum so set the input files list equal to the full list of input bhabha files.")

    input_files_bhabha_crystal = input_files_bhabha_full.copy()
    if (setMaxEventsPerCrystalCalib):
        input_files_bhabha_crystal = input_files_bhabha_crystal_maxEvents.copy()
        print("List of input files slimmed for crystals.")
    else:
        print("User requested max number of events filter to not be used so all bhabha input files being used for crystals")

    basf2.B2INFO(f"Total number of bhabha files being used for crystal calibration = {len(input_files_bhabha_crystal)}")

    reduced_file_to_iov_hadron = filter_by_max_files_per_run(file_to_iov_hadron, max_files_per_run_validation)
    input_files_hadron = list(reduced_file_to_iov_hadron.keys())
    basf2.B2INFO(f"Total number of hadron files actually used as input = {len(input_files_hadron)}")

    # For plotting the sum of the crate and crystal calibration
    # constants after the calibrations are made.  To be used to
    # monitor crate time jumps.
    reduced_file_to_iov_plotting = filter_by_max_events_per_run(file_to_iov_bhabha, max_events_per_run_plotting)
    input_files_plotting = list(reduced_file_to_iov_plotting.keys())

    # Interval of validity for the calibrations
    intermediate_iov = IoV(0, 0, -1, -1)
    requested_iov = kwargs.get("requested_iov", None)
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    ###################################################
    from basf2 import register_module, create_path
    import ROOT
    from ROOT import Belle2
    from ROOT.Belle2 import TestCalibrationAlgorithm
    from caf.framework import Collection

    ###################################################
    # Collector setup for both crates and crystals calibrations
    print("Set up the base for the collectors for the calibrations")

    root_input = register_module('RootInput')
    rec_path_bhabha = create_path()
    rec_path_bhabha.add_module(root_input)
    if 'Gearbox' not in rec_path_bhabha:
        rec_path_bhabha.add_module('Gearbox')
    if 'Geometry' not in rec_path_bhabha:
        rec_path_bhabha.add_module('Geometry', useDB=True)

    prepare_cdst_analysis(rec_path_bhabha)  # for new 2020 cdst format

    # ====================================================
    # t0BiasCorrection = -0.9  # Correct for the CDC t0 bias in ns
    t0BiasCorrection = t0_bhabhaToHadron_correction  # Correct for the CDC t0 bias in ns
    # ====================================================

    col_bhabha = register_module('ECLBhabhaTCollector')
    col_bhabha.param('timeAbsMax', 250)
    col_bhabha.param('minCrystal', 1)
    col_bhabha.param('maxCrystal', 8736)
    col_bhabha.param('saveTree', False)
    col_bhabha.param('skipTrgSel', False)
    col_bhabha.param('hadronEventT0_TO_bhabhaEventT0_correction', t0BiasCorrection)

    eclTCol = Collection(collector=col_bhabha,
                         input_files=input_files_bhabha,
                         pre_collector_path=rec_path_bhabha)

    eclTCol_crys = Collection(collector=col_bhabha,
                              input_files=input_files_bhabha_crystal,
                              pre_collector_path=rec_path_bhabha)

    ###################################################
    # Algorithm setup for crates
    print("Set up the base for the crate calibration algorithm")

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
    print("Set up the base for the crystal calibration algorithm")

    eclTAlgCrystals = Belle2.ECL.eclBhabhaTAlgorithm()

    # Define the CAF algorithm arguments
    # Set the crateIDLo to be larger than crateIDHi so that no crate
    #    calibrations will be performed.  More variables set later in code....
    eclTAlgCrystals.crateIDLo = 3
    eclTAlgCrystals.crateIDHi = 2
    eclTAlgCrystals.debugOutput = True
    eclTAlgCrystals.meanCleanRebinFactor = 3
    eclTAlgCrystals.meanCleanCutMinFactor = 0.3
    eclTAlgCrystals.debugFilenameBase = "eclBhabhaTAlgorithm"

    # Make two versions of these algorithms that differ only in whether
    #    or not the previous bucket's payload information is saved
    #    to a temporary payload for comparison purposes at each
    #    iteration.
    eclTAlgCrystals_saveNotReadPrevPayload = copy.deepcopy(eclTAlgCrystals)
    eclTAlgCrystals_readNotSavePrevPayload = copy.deepcopy(eclTAlgCrystals)

    eclTAlgCrystals_saveNotReadPrevPayload.savePrevCrysPayload = True
    eclTAlgCrystals_saveNotReadPrevPayload.readPrevCrysPayload = False

    eclTAlgCrystals_readNotSavePrevPayload.savePrevCrysPayload = False
    eclTAlgCrystals_readNotSavePrevPayload.readPrevCrysPayload = True

    ###################################################
    # Algorithm setup for crystal payload merger
    print("Set up the base for the crystal merger algorithm")

    merging_alg = Belle2.ECL.eclMergingCrystalTimingAlgorithm()
    merging_alg.readPrevCrysPayload = True

    ####################################################
    # Introduce the number of crate+crystal iterations
    # requested by the user if the number of iterations
    # requested is a valid number above 0. The default
    # without a valid user input is to do 2 crate +
    # 1 crystal calibrations.

    if numCrysCrateIterations <= 0:
        print("numCrysCrateIterations NEEDS TO BE > 0 !!!")
    elif numCrysCrateIterations > 0:
        ###################################################
        # Do multiple iterations as determined by the user

        from caf.framework import Calibration
        print("Performing ", numCrysCrateIterations, " iterations of crate and crystal calibrations.")

        crysCalibBaseName = "ECLcrystalTimeCalibration_physics_"
        crateCalibBaseName = "ECLcrateTimeCalibration_physics_"
        mergeCalibBaseName = "ecl_t_merge_"

        calibs = []

        # --------------------------------------------------------------
        # Loop over all the iterations to set up the crate and crystal
        # calibrations.

        for i in range(numCrysCrateIterations):
            crysCalibName = crysCalibBaseName + str(i)
            crateCalibName = crateCalibBaseName + str(i)
            mergeCalibName = mergeCalibBaseName + str(i)

            print("iteration = ", i)
            print("crysCalibName = ", crysCalibName)
            print("crateCalibName = ", crateCalibName)

            ###################################################
            # Calibration setup for crates iteration
            cal_crates_i = Calibration(crateCalibName)
            cal_crates_i.add_collection(name="bhabha", collection=eclTCol)
            cal_crates_i.algorithms = [eclTAlgCrates]
            cal_crates_i.save_payloads = False
            cal_crates_i.strategies = SimpleRunByRun
            cal_crates_i.backend_args = {"request_memory": "4 GB"}

            ###################################################
            # Calibration setup for crystals iteration

            cal_crystals_i = Calibration(crysCalibName)
            cal_crystals_i.add_collection(name="bhabha", collection=eclTCol_crys)
            cal_crystals_i.backend_args = {"request_memory": "4 GB"}

            # If this is the first iteration then save the previous crystal payload
            # values to a temporary storage payload.  If this is not the first iteration
            # then read in the previous stored payload and plot the change in the crystal
            if i == 0:
                cal_crystals_i.algorithms = [eclTAlgCrystals_saveNotReadPrevPayload]
                print("crystal algorithm: save previous payload for comparison purposes but do not read it")
                print("eclTAlgCrystals_saveNotReadPrevPayload.savePrevCrysPayload = ",
                      eclTAlgCrystals_saveNotReadPrevPayload.savePrevCrysPayload)
                print("eclTAlgCrystals_saveNotReadPrevPayload.readPrevCrysPayload = ",
                      eclTAlgCrystals_saveNotReadPrevPayload.readPrevCrysPayload)
            else:
                cal_crystals_i.algorithms = [eclTAlgCrystals_readNotSavePrevPayload]
                print("crystal algorithm: do not save previous payload but do read it in for comparison purposes")
                print("eclTAlgCrystals_readNotSavePrevPayload.savePrevCrysPayload = ",
                      eclTAlgCrystals_readNotSavePrevPayload.savePrevCrysPayload)
                print("eclTAlgCrystals_readNotSavePrevPayload.readPrevCrysPayload = ",
                      eclTAlgCrystals_readNotSavePrevPayload.readPrevCrysPayload)

            cal_crystals_i.save_payloads = False

            # If payload boundaries are set use SequentialBoundaries
            # otherwise use SingleIOV
            if payload_boundaries:
                cal_crystals_i.strategies = SequentialBoundaries
            else:
                cal_crystals_i.strategies = SingleIOV

            ###################################################
            # Setup for merging crystals payloads

            cal_ecl_merge_i = Calibration(name=mergeCalibName, collector="DummyCollector",
                                          input_files=input_files_bhabha_1perRun)

            # Read in the previous crystal payload values for comparision
            cal_ecl_merge_i.algorithms = [merging_alg]
            print("merge algorithm: read previous payload for comparison purposes")
            print("merging_alg.readPrevCrysPayload = ", merging_alg.readPrevCrysPayload)

            cal_ecl_merge_i.save_payloads = False
            ecl_merge_pre_path_i = basf2.create_path()
            prepare_cdst_analysis(ecl_merge_pre_path_i)
            ecl_merge_pre_path_i.pre_collector_path = ecl_merge_pre_path_i

            # If payload boundaries are set use SequentialBoundaries
            # otherwise use SingleIOV
            if payload_boundaries:
                cal_ecl_merge_i.strategies = SequentialBoundaries
            else:
                cal_ecl_merge_i.strategies = SingleIOV

            # Modify the iov for each of the calibration algorithms
            for algorithm in cal_crystals_i.algorithms:
                # The payload of the final crystal calibration iteration
                # is given the user's iov.
                if i == numCrysCrateIterations - 1:
                    # Set payload iov information for SequentialBoundaries
                    # and SingleIOV strategies.
                    if payload_boundaries:
                        algorithm.params = {"iov_coverage": output_iov,
                                            "payload_boundaries": payload_boundaries}

                        print("Set iov for final crystals alg - SequentialBoundaries")
                    else:
                        algorithm.params = {"apply_iov": output_iov}
                        print("Set iov for final crystals alg - SingleIOV ")
                # Force the output iovs to be open for the intermediate
                # step calibrations.
                else:
                    if payload_boundaries:
                        algorithm.params = {"iov_coverage": intermediate_iov,
                                            "payload_boundaries": payload_boundaries}
                        print("Set iov for intermediate crystals alg - SequentialBoundaries")
                    else:
                        algorithm.params = {"apply_iov": intermediate_iov}
                        print("Set iov for intermediate crystals alg - SingleIOV ")

            # Modify the iov for each of the payload merging algorithms
            for algorithm in cal_ecl_merge_i.algorithms:
                # The payload of the final crystal calibration iteration
                # is given the user's iov.
                if i == numCrysCrateIterations - 1:
                    # Set payload iov information for SequentialBoundaries
                    # and SingleIOV strategies.
                    if payload_boundaries:
                        algorithm.params = {"iov_coverage": output_iov,
                                            "payload_boundaries": payload_boundaries}

                        print("Set iov for final merging alg - SequentialBoundaries")
                    else:
                        algorithm.params = {"apply_iov": output_iov}
                        print("Set iov for final merging alg - SingleIOV ")
                # Force the output iovs to be open for the intermediate
                # step calibrations.
                else:
                    if payload_boundaries:
                        algorithm.params = {"iov_coverage": intermediate_iov,
                                            "payload_boundaries": payload_boundaries}
                        print("Set iov for intermediate merging alg - SequentialBoundaries")
                    else:
                        algorithm.params = {"apply_iov": intermediate_iov}
                        print("Set iov for intermediate merging alg - SingleIOV ")

            # Fill the calibs array with all the "calibrations" that will be run
            calibs = np.append(calibs, [cal_crates_i, cal_crystals_i, cal_ecl_merge_i])

        # Make sure that the final paylaods get saved.
        calibs[len(calibs) - 3].save_payloads = True   # crates
        calibs[len(calibs) - 1].save_payloads = True   # crystals

        #############################################################
        #############################################################
        # Set up the validations now
        #############################################################
        #############################################################

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

        prepare_user_cdst_analysis(rec_path_bhabha_val)    # for new 2020 cdst format

        col_bhabha_val = register_module('eclBhabhaTimeCalibrationValidationCollector')
        col_bhabha_val.param('timeAbsMax', 70)
        col_bhabha_val.param('saveTree', False)
        col_bhabha_val.param('skipTrgSel', False)

        eclValTCol = Collection(collector=col_bhabha_val,
                                input_files=input_files_bhabha_crystal,
                                pre_collector_path=rec_path_bhabha_val)

        ##############################################################
        # Algorithm setup for ecl timing validation with bhabhas

        # Give the collector name to the algorithm since one algorithm
        # is used to analyse the results from several possible collectors
        eclValTAlgBhabha = Belle2.ECL.eclTValidationAlgorithm("eclBhabhaTimeCalibrationValidationCollector")

        # Define the CAF algorithm arguments
        eclValTAlgBhabha.meanCleanRebinFactor = 3
        eclValTAlgBhabha.meanCleanCutMinFactor = 0.4
        eclValTAlgBhabha.clusterTimesFractionWindow_maxtime = 1.5
        eclValTAlgBhabha.debugFilenameBase = "eclBhabhaTValidationAlgorithm"

        ####################################################################
        # Calibration setup base for ecl timing validation with bhabhas

        from caf.framework import Calibration

        valid_cal_bhabha = Calibration("ECLcrystalTimeCalValidation_bhabhaPhysics")
        valid_cal_bhabha.add_collection(name="bhabha", collection=eclValTCol)
        valid_cal_bhabha.save_payloads = False
        valid_cal_bhabha.backend_args = {"request_memory": "4 GB"}

        # Make a second version of this algorithm that differs only in
        #    that it is instructed to read the previous crystal payload.
        #    The version where we don't read in the previous payload
        #    is left over from a previous code version
        eclValTAlgBhabha_readPrevPayload = copy.deepcopy(eclValTAlgBhabha)
        eclValTAlgBhabha_readPrevPayload.readPrevCrysPayload = True

        valid_cal_bhabha.algorithms = [eclValTAlgBhabha_readPrevPayload]
        print("bhabha validation: read previous payload for comparison purposes")
        print("eclValTAlgBhabha_readPrevPayload.readPrevCrysPayload = ",
              eclValTAlgBhabha_readPrevPayload.readPrevCrysPayload)

        # If payload boundaries are set use SequentialBoundaries
        # otherwise use SingleIOV
        if payload_boundaries:
            valid_cal_bhabha.strategies = SequentialBoundaries
        else:
            valid_cal_bhabha.strategies = SingleIOV

        for algorithm in valid_cal_bhabha.algorithms:
            # Set payload iov information for SequentialBoundaries
            # and SingleIOV strategies.
            if payload_boundaries:
                algorithm.params = {"iov_coverage": output_iov,
                                    "payload_boundaries": payload_boundaries}

                print("Set iov for bhabha validation alg - SequentialBoundaries")
            else:
                algorithm.params = {"apply_iov": output_iov}
                print("Set iov for bhabha validation alg - SingleIOV")

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

        prepare_user_cdst_analysis(rec_path_hadron_val)    # for new 2020 cdst format

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
        eclValTAlgHadronic.meanCleanRebinFactor = 3
        eclValTAlgHadronic.meanCleanCutMinFactor = 0.4
        eclValTAlgHadronic.clusterTimesFractionWindow_maxtime = 8
        eclValTAlgHadronic.debugFilenameBase = "eclHadronTValidationAlgorithm"

        ####################################################################
        # Calibration setup base for ecl timing validation with hadrons

        from caf.framework import Calibration

        valid_cal_hadron = Calibration("ECLcrystalTimeCalValidation_hadronPhysics")
        valid_cal_hadron.add_collection(name="hadron", collection=eclValTCol)
        valid_cal_hadron.save_payloads = False
        valid_cal_hadron.backend_args = {"request_memory": "4 GB"}

        # Make a second version of this algorithm that differs only in
        #    that it is instructed to read the previous crystal payload
        eclValTAlgHadron_readPrevPayload = copy.deepcopy(eclValTAlgHadronic)
        eclValTAlgHadron_readPrevPayload.readPrevCrysPayload = True

        # Make a second version of this algorithm that differs only in
        #    that it is instructed to read the previous crystal payload.
        #    The version where we don't read in the previous payload
        #    is left over from a previous code version
        valid_cal_hadron.algorithms = [eclValTAlgHadron_readPrevPayload]
        print("hadron validation: read previous payload for comparison purposes")
        print("eclValTAlgHadron_readPrevPayload.readPrevCrysPayload = ",
              eclValTAlgHadron_readPrevPayload.readPrevCrysPayload)

        # If payload boundaries are set use SequentialBoundaries
        # otherwise use SingleIOV
        if payload_boundaries:
            valid_cal_hadron.strategies = SequentialBoundaries
        else:
            valid_cal_hadron.strategies = SingleIOV

        for algorithm in valid_cal_hadron.algorithms:
            # Set payload iov information for SequentialBoundaries
            # and SingleIOV strategies.
            if payload_boundaries:
                algorithm.params = {"iov_coverage": output_iov,
                                    "payload_boundaries": payload_boundaries}

                print("Set iov for hadron validation alg - SequentialBoundaries")
            else:
                algorithm.params = {"apply_iov": output_iov}
                print("Set iov for hadron validation alg - SingleIOV")

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
                             pre_collector_path=rec_path_bhabha_plotting)

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

        cal_ecl_timeShifts = Calibration(name="ecl_t_shifts", algorithms=[tShifts_alg],
                                         input_files=input_files_plotting)
        cal_ecl_timeShifts.add_collection(name="bhabha", collection=eclTCol)

        cal_ecl_timeShifts.save_payloads = False

        # If payload boundaries are set use SequentialBoundaries
        # otherwise use SingleIOV
        if payload_boundaries:
            cal_ecl_timeShifts.strategies = SequentialBoundaries
        else:
            cal_ecl_timeShifts.strategies = SingleIOV

        for algorithm in cal_ecl_timeShifts.algorithms:
            # Set payload iov information for SequentialBoundaries
            # and SingleIOV strategies.
            if payload_boundaries:
                algorithm.params = {"iov_coverage": output_iov,
                                    "payload_boundaries": payload_boundaries}

                print("Set iov for crate time plotting alg - SequentialBoundaries")
            else:
                algorithm.params = {"apply_iov": output_iov}
                print("Set iov for crate time plotting alg - SingleIOV")

        ##################################################
        # Set the dependencies to determine the order in
        # which calibrations have to be performed.  The
        # order determines the sequential order and
        # which calibrations can be run in parallel.

        # Make the crate and crytsal calibrations and the cerge crystal
        # process occur in the order loaded into the array.
        for i in range(len(calibs) - 1):
            calibs[i + 1].depends_on(calibs[i])

        # The validations and plotting can run independently of each other
        # but rely on the last calibration step
        valid_cal_bhabha.depends_on(calibs[len(calibs) - 1])
        valid_cal_hadron.depends_on(calibs[len(calibs) - 1])
        cal_ecl_timeShifts.depends_on(calibs[len(calibs) - 1])

        ###################################################
        # Finalize all calibrations

        calibs = np.append(calibs, [valid_cal_bhabha, valid_cal_hadron, cal_ecl_timeShifts])

        print("##############")
        print("List of calibrations:")
        for c in calibs:
            print("  ", c.name, " depends on ", c.dependencies, ", save payloads = ", c.save_payloads)
        print("##############")

        # You must return all calibrations you want to run in the prompt process, even if it's only one
        # Calibrations will be executed in this order as a result of the dependencies defined by the "dependes_on(...)".
        return calibs

##############################
