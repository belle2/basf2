##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
# -*- coding: utf-8 -*-

"""A complicated example calibration that takes several input data lists from raw data and performs
multiple calibrations. Only the second calibration will have its payloads placed into the final
outputdb directory by b2caf-prompt-run.

We make it so that this calibration depends on the result of a completely
different one 'example_simple'. Even though that calibration will not be run in this process, the automated
system can discover this dependency and use it when submitting tasks."""

from prompt import CalibrationSettings, input_data_filters

##############################
# REQUIRED VARIABLE #
##############################
# Used to identify the keys in input_data that your get_calibrations function will need in order
# to assign data correctly.
# Will be used to construct the calibration in the automated system, as well as set up the submission web forms.
# You can view the available input data formats from CalibrationSettings.allowed_data_formats

# We decide to only run this script once the simple one has run. This only affects the automated system when scheduling
# tasks. This script can always be run standalone.
from prompt.calibrations.example_simple import settings as example_simple

#: Tells the automated system some details of this script. The input_data_filters is only used for automated calibration (optional).
settings = CalibrationSettings(name="Example Complex",
                               expert_username="ddossett",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=["physics", "cosmics", "Bcosmics"],
                               input_data_filters={"physics": [f"NOT {input_data_filters['Magnet']['On']}",
                                                               input_data_filters["Data Tag"]["hadron_calib"],
                                                               input_data_filters["Data Quality Tag"]["Good"],
                                                               input_data_filters["Beam Energy"]["4S"],
                                                               input_data_filters["Run Type"]["physics"]],
                                                   "cosmics": [input_data_filters['Magnet']['Off'],
                                                               input_data_filters["Data Tag"]["cosmic_calib"],
                                                               input_data_filters["Data Quality Tag"]["Bad For Alignment"],
                                                               input_data_filters["Beam Energy"]["Continuum"],
                                                               f"NOT {input_data_filters['Run Type']['physics']}"],
                                                   "Bcosmics": [input_data_filters["Data Tag"]["cosmic_calib"],
                                                                input_data_filters["Data Quality Tag"]["Good"],
                                                                input_data_filters["Beam Energy"]["4S"]]},
                               depends_on=[example_simple],
                               expert_config={
                                   "physics_prescale": 0.2,
                                   "max_events_per_file": 100,
                                   "max_files_per_run": 2,
                                   "payload_boundaries": []
                               })

# The values in expert_config above are the DEFAULT for this script. They will be overwritten by values in caf_config.json

# Note that you are forced to import the relevant script that you depend on, even though you never use it.
# This is to make sure that this script won't run unless the dependent one exists, as well as automatically
# checking for circular dependency via Python's import statements.

##############################

##############################
# REQUIRED FUNCTION #
##############################
# The only function that MUST exist in this module. It should return a LIST of Calibration objects
# that have had their input files assigned and any configuration applied. The final output payload IoV(s)
# should also be set correctly to be open-ended e.g. IoV(exp_low, run_low, -1, -1)
#
# The database_chain of these calibrations will all be set/overwritten by the b2caf-prompt-run tool.


def get_calibrations(input_data, **kwargs):
    """
    Parameters:
      input_data (dict): Should contain every name from the 'input_data_names' variable as a key.
        Each value is a dictionary with {"/path/to/file_e1_r5.root": IoV(1,5,1,5), ...}. Useful for
        assigning to calibration.files_to_iov

      **kwargs: Configuration options to be sent in. Since this may change we use kwargs as a way to help prevent
        backwards compatibility problems. But you could use the correct arguments in b2caf-prompt-run for this
        release explicitly if you want to.

        Currently only kwargs["requested_iov"] and kwargs["expert_config"] are used.

        "requested_iov" is the IoV range of the bucket and your payloads should correspond to this range.
        However your highest payload IoV should be open ended e.g. IoV(3,4,-1,-1)

        "expert_config" is the input configuration. It takes default values from your `CalibrationSettings` but these are
        overwritten by values from the 'expert_config' key in your input `caf_config.json` file when running ``b2caf-prompt-run``.

    Returns:
      list(caf.framework.Calibration): All of the calibration objects we want to assign to the CAF process
    """
    # Set up config options
    import basf2
    from basf2 import register_module, create_path
    from ROOT.Belle2 import TestCalibrationAlgorithm, TestBoundarySettingAlgorithm
    from caf.framework import Calibration, Collection
    from caf.strategies import SequentialBoundaries
    from caf.utils import vector_from_runs, ExpRun, IoV

    # In this script we want to use three different sources of input data, and reconstruct them
    # differently before the Collector module runs.

    # Get the input files from the input_data variable
    file_to_iov_physics = input_data["physics"]
    file_to_iov_cosmics = input_data["cosmics"]
    file_to_iov_Bcosmics = input_data["Bcosmics"]

    # We might have requested an enormous amount of data across a requested range.
    # There's a LOT more files than runs!
    # Lets set some limits because this calibration doesn't need that much to run.
    expert_config = kwargs.get("expert_config")
    max_files_per_run = expert_config["max_files_per_run"]
    basf2.B2INFO(f"Reducing to a maximum of {max_files_per_run} files per run.")

    # We filter out any more than 2 files per run. The input data files are sorted alphabetically by b2caf-prompt-run
    # already. This procedure respects that ordering
    from prompt.utils import filter_by_max_files_per_run

    reduced_file_to_iov_physics = filter_by_max_files_per_run(file_to_iov_physics, max_files_per_run)
    input_files_physics = list(reduced_file_to_iov_physics.keys())
    basf2.B2INFO(f"Total number of physics files actually used as input = {len(input_files_physics)}")

    reduced_file_to_iov_cosmics = filter_by_max_files_per_run(file_to_iov_cosmics, max_files_per_run)
    input_files_cosmics = list(reduced_file_to_iov_cosmics.keys())
    basf2.B2INFO(f"Total number of cosmics files actually used as input = {len(input_files_cosmics)}")

    reduced_file_to_iov_Bcosmics = filter_by_max_files_per_run(file_to_iov_Bcosmics, max_files_per_run)
    input_files_Bcosmics = list(reduced_file_to_iov_Bcosmics.keys())
    basf2.B2INFO(f"Total number of Bcosmics files actually used as input = {len(input_files_Bcosmics)}")

    # Get the overall request IoV we want to cover, including the end values. But we will probably want to replace the end values
    # with -1, -1 when setting the output payload IoVs.
    requested_iov = kwargs.get("requested_iov", None)

    # The actual value our output IoV payload should have. Notice that we've set it open ended.
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    ###################################################
    # Collector setup
    # We'll make two instances of the same CollectorModule, but configured differently
    col_test_physics = register_module("CaTest")
    # This has to be 'run' otherwise our SequentialBoundaries strategy can't work.
    # We could make it optional, based on the contents of the expert_config.
    col_test_physics.param("granularity", "run")
    col_test_physics.param("spread", 4)

    col_test_Bcosmics = register_module("CaTest")
    col_test_Bcosmics.param("granularity", "all")
    col_test_Bcosmics.param("spread", 1)

    col_test_cosmics = register_module("CaTest")
    col_test_cosmics.param("granularity", "all")
    col_test_cosmics.param("spread", 10)

    ###################################################
    # Reconstruction path setup
    # create the basf2 paths to run before each Collector module

    # Let's specify that not all events will be used per file for every Collection
    # Just set this with one element in the list if you use it. The value will be duplicated in collector subjobs if the number
    # of input files is larger than 1.
    max_events = expert_config["max_events_per_file"]
    root_input = register_module("RootInput",
                                 entrySequences=[f"0:{max_events}"]
                                 )

    # And/or we could set a prescale so that only a fraction of events pass onwards.
    # This is most useful for randomly selecting events throughout input files.
    # Note that if you set the entrySequences AS WELL as a prescale then you will be combining the entrySequences and prescale
    # so that only a few events are passed into the Prescale module, and then only a fraction of those will continue to the
    # Collector module.
    prescale = expert_config["physics_prescale"]
    prescale_mod = register_module("Prescale", prescale=prescale)
    empty_path = create_path()
    prescale_mod.if_false(empty_path, basf2.AfterConditionPath.END)

    rec_path_physics = create_path()
    rec_path_physics.add_module(root_input)
    rec_path_physics.add_module(prescale_mod)
    # could now add reconstruction modules dependent on the type of input data

    rec_path_cosmics = create_path()
    rec_path_cosmics.add_module(root_input)
    # could now add reconstruction modules dependent on the type of input data

    rec_path_Bcosmics = create_path()
    rec_path_Bcosmics.add_module(root_input)
    # could now add reconstruction modules dependent on the type of input data

    ###################################################
    # Algorithm setup
    # We'll only use one algorithm for each of our two Calibrations.
    alg_test1 = TestCalibrationAlgorithm()
    alg_test2 = TestBoundarySettingAlgorithm()

    # Send in a list of boundaries for our algorithm class and SequentialBoundaries strategy to use.
    # A boundary is the STARTING run number for a new payload and all data from runs between this run and the next
    # boundary will be used.
    # In our algorithm the first run in our data is always a starting boundary, so we can pass an empty list here
    # safely and still have it work.

    # We make sure that the first payload begins at the start of the requested IoV.
    # This is a quirk of SequentialBoundaries strategy as there must always be one boundary to START from.
    # You could elect to always set this yourself manually, but that seems error prone.
    payload_boundaries = [ExpRun(output_iov.exp_low, output_iov.run_low)]
    # Now we can add the boundaries that exist in the expert config. They are extra boundaries, so that we don't have
    # to set the initial one every time. If this is an empty list then we effectively run like the SingleIoV strategy.
    payload_boundaries.extend([ExpRun(*boundary) for boundary in expert_config["payload_boundaries"]])
    basf2.B2INFO(f"Expert set payload boundaries are: {expert_config['payload_boundaries']}")
    # Now set them all
    alg_test2.setBoundaries(vector_from_runs(payload_boundaries))  # This takes boundaries from the expert_config

    ###################################################
    # Collection Setup
    #
    # We set the maximum number of collector (sub)jobs. This isn't necessary (the default is 1000). But I do it here
    # just to show that b2caf-prompt-run no longer overwrites these types of settings (unless explicitly set).
    collection_physics = Collection(collector=col_test_physics,
                                    input_files=input_files_physics,
                                    pre_collector_path=rec_path_physics,
                                    max_collector_jobs=4
                                    )

    collection_cosmics = Collection(collector=col_test_cosmics,
                                    input_files=input_files_cosmics,
                                    pre_collector_path=rec_path_cosmics,
                                    max_collector_jobs=2
                                    )

    collection_Bcosmics = Collection(collector=col_test_Bcosmics,
                                     input_files=input_files_Bcosmics,
                                     pre_collector_path=rec_path_Bcosmics,
                                     max_collector_jobs=2
                                     )

    ###################################################
    # Calibration setup

    # We will set up two Calibrations. One which depends on the other.
    # However, the first Calibration will generate payloads that we don't want to save in our output database for upload.
    # Basically we want to ignore the payloads during the b2caf-prompt-run copying of the outputdb contents.
    # But we still use them as input to the next calibration.

    cal_test1 = Calibration("TestCalibration_cosmics")
    # Add collections in with unique names
    cal_test1.add_collection(name="cosmics", collection=collection_cosmics)
    cal_test1.add_collection(name="Bcosmics", collection=collection_Bcosmics)
    cal_test1.algorithms = [alg_test1]
    # Do this for the default AlgorithmStrategy to force the output payload IoV
    cal_test1.algorithms[0].params = {"apply_iov": output_iov}
    # Mark this calibration as one whose payloads should not be copied at the end.
    cal_test1.save_payloads = False

    cal_test2 = Calibration("TestCalibration_physics")
    # Add collections in with unique names
    cal_test2.add_collection(name="physics", collection=collection_physics)
    cal_test2.algorithms = [alg_test2]
    # We apply a a different strategy that will allow us to split the data we run over into chunks based on the boundaries above
    cal_test2.strategies = SequentialBoundaries
    # Do this to force the output payload IoV. Note the different name to above!
    cal_test2.algorithms[0].params["iov_coverage"] = output_iov

    cal_test2.depends_on(cal_test1)

    # You must return all calibrations you want to run in the prompt process
    return [cal_test1, cal_test2]

##############################
