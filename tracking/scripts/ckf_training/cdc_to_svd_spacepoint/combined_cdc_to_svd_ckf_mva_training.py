#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import itertools
import os
import subprocess

import basf2
from tracking import add_track_finding
from tracking.path_utils import add_hit_preparation_modules
from tracking.harvesting_validation.combined_module import CombinedTrackingValidationModule
import background
import simulation
from packaging import version
from pathlib import Path
import basf2_mva
import tracking.root_utils as root_utils

# from ckf_training import my_basf2_mva_teacher

# wrap python modules that are used here but not in the externals into a try except block
install_helpstring_formatter = ("\nCould not find {module} python module.Try installing it via\n"
                                "  python3 -m pip install [--user] {module}\n")
try:
    import b2luigi
    from b2luigi.core.utils import create_output_dirs
    from b2luigi.basf2_helper import Basf2PathTask, Basf2Task
    from b2luigi.basf2_helper.utils import get_basf2_git_hash
except ModuleNotFoundError:
    print(install_helpstring_formatter.format(module="b2luigi"))
    raise

# If b2luigi version 0.3.2 or older, it relies on $BELLE2_RELEASE being "head",
# which is not the case in the new externals. A fix has been merged into b2luigi
# via https://github.com/nils-braun/b2luigi/pull/17 and thus should be available
# in future releases.
if (
    version.parse(b2luigi.__version__) <= version.parse("0.3.2") and
    get_basf2_git_hash() is None and
    os.getenv("BELLE2_LOCAL_DIR") is not None
):
    print(f"b2luigi version could not obtain git hash because of a bug not yet fixed in version {b2luigi.__version__}\n"
          "Please install the latest version of b2luigi from github via\n\n"
          "  python3 -m pip install --upgrade [--user] git+https://github.com/nils-braun/b2luigi.git\n")
    raise ImportError


def my_basf2_mva_teacher(
    records_files,
    tree_name,
    weightfile_identifier,
    target_variable="truth",
    exclude_variables=None,
    fast_bdt_option=[200, 8, 3, 0.1]  # nTrees, nCuts, nLevels, shrinkage
):
    """
    My custom wrapper for basf2 mva teacher.  Adapted from code in ``trackfindingcdc_teacher``.

    :param records_files: List of files with collected ("recorded") variables to use as training data for the MVA.
    :param tree_name: Name of the TTree in the ROOT file from the ``data_collection_task``
           that contains the training data for the MVA teacher.
    :param weightfile_identifier: Name of the weightfile that is created.
           Should either end in ".xml" for local weightfiles or in ".root", when
           the weightfile needs later to be uploaded as a payload to the conditions
           database.
    :param target_variable: Feature/variable to use as truth label in the quality estimator MVA classifier.
    :param exclude_variables: List of collected variables to not use in the training of the QE MVA classifier.
           In addition to variables containing the "truth" substring, which are excluded by default.
    :param fast_bdt_option: specified fast BDT options, default: [200, 8, 3, 0.1] [nTrees, nCuts, nLevels, shrinkage]
    """
    if exclude_variables is None:
        exclude_variables = []

    weightfile_extension = Path(weightfile_identifier).suffix
    if weightfile_extension not in {".xml", ".root"}:
        raise ValueError(f"Weightfile Identifier should end in .xml or .root, but ends in {weightfile_extension}")

    # extract names of all variables from one record file
    with root_utils.root_open(records_files[0]) as records_tfile:
        input_tree = records_tfile.Get(tree_name)
        feature_names = [leave.GetName() for leave in input_tree.GetListOfLeaves()]

    # get list of variables to use for training without MC truth
    truth_free_variable_names = [
        name
        for name in feature_names
        if (
            ("truth" not in name) and
            (name != target_variable) and
            (name not in exclude_variables)
        )
    ]
    if "weight" in truth_free_variable_names:
        truth_free_variable_names.remove("weight")
        weight_variable = "weight"
    elif "__weight__" in truth_free_variable_names:
        truth_free_variable_names.remove("__weight__")
        weight_variable = "__weight__"
    else:
        weight_variable = ""

    # Set options for MVA training
    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector(*records_files)
    general_options.m_treename = tree_name
    general_options.m_weight_variable = weight_variable
    general_options.m_identifier = weightfile_identifier
    general_options.m_variables = basf2_mva.vector(*truth_free_variable_names)
    general_options.m_target_variable = target_variable
    fastbdt_options = basf2_mva.FastBDTOptions()

    fastbdt_options.m_nTrees = fast_bdt_option[0]
    fastbdt_options.m_nCuts = fast_bdt_option[1]
    fastbdt_options.m_nLevels = fast_bdt_option[2]
    fastbdt_options.m_shrinkage = fast_bdt_option[3]
    # Train a MVA method and store the weightfile (MVAFastBDT.root) locally.
    basf2_mva.teacher(general_options, fastbdt_options)


def create_fbdt_option_string(fast_bdt_option):
    """
    returns a readable string created by the fast_bdt_option array
    """
    return "_nTrees" + str(fast_bdt_option[0]) + "_nCuts" + str(fast_bdt_option[1]) + "_nLevels" + \
        str(fast_bdt_option[2]) + "_shrin" + str(int(round(100*fast_bdt_option[3], 0)))


class GenerateSimTask(Basf2PathTask):
    """
    Generate simulated Monte Carlo with background overlay.

    Make sure to use different ``random_seed`` parameters for the training data
    format the classifier trainings and for the test data for the respective
    evaluation/validation tasks.
    """

    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
    #: Random basf2 seed. It is further used to read of the production process to preserve
    # clearness in the b2luigi output.
    random_seed = b2luigi.Parameter()
    #: Number of events to generate.
    n_events = b2luigi.IntParameter()
    #: Directory with overlay background root files
    bkgfiles_dir = b2luigi.Parameter(
        #: \cond
        hashed=True
        #: \endcond
    )
    #: specify queue. E.g. choose between 'l' (long), 's' (short) or 'sx' (short, extra ram)
    queue = 'l'

    #: Name of the ROOT output file with generated and simulated events.
    def output_file_name(self, n_events=None, random_seed=None):
        """
        Create output file name depending on number of events and production
        mode that is specified in the random_seed string.
        """
        if n_events is None:
            n_events = self.n_events
        if random_seed is None:
            random_seed = self.random_seed
        return "generated_mc_N" + str(n_events) + "_" + random_seed + ".root"

    def output(self):
        """
        Generate list of output files that the task should produce.
        The task is considered finished if and only if the outputs all exist.
        """
        yield self.add_to_output(self.output_file_name())

    def create_path(self):
        """
        Create basf2 path to process with event generation and simulation.
        """
        basf2.set_random_seed(self.random_seed)
        path = basf2.create_path()
        if self.experiment_number in [0, 1003]:
            runNo = 0
        else:
            runNo = 0
            raise ValueError(
                f"Simulating events with experiment number {self.experiment_number} is not implemented yet.")
        path.add_module(
            "EventInfoSetter", evtNumList=[self.n_events], runList=[runNo], expList=[self.experiment_number]
        )
        path.add_module("EvtGenInput")
        # activate simulation of dead/masked pixel and reproduce detector gain, which will be
        # applied at reconstruction level when the data GT is present in the DB chain
        # path.add_module("ActivatePXDPixelMasker")
        # path.add_module("ActivatePXDGainCalibrator")
        bkg_files = background.get_background_files(self.bkgfiles_dir)

        simulation.add_simulation(path, bkgfiles=bkg_files, bkgOverlay=True, usePXDDataReduction=False)

        path.add_module(
            "RootOutput",
            outputFileName=self.get_output_file_name(self.output_file_name()),
        )
        return path


# I don't use the default MergeTask or similar because they only work if every input file is called the same.
# Additionally, I want to add more features like deleting the original input to save storage space.
class SplitNMergeSimTask(Basf2Task):
    """
    Generate simulated Monte Carlo with background overlay.

    Make sure to use different ``random_seed`` parameters for the training data
    format the classifier trainings and for the test data for the respective
    evaluation/validation tasks.
    """
    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
    #: Random basf2 seed. It is further used to read of the production process to preserve
    # clearness in the b2luigi output.
    random_seed = b2luigi.Parameter()
    #: Number of events to generate.
    n_events = b2luigi.IntParameter()
    #: Directory with overlay background root files
    bkgfiles_dir = b2luigi.Parameter(
        #: \cond
        hashed=True
        #: \endcond
    )
    #: specify queue. E.g. choose between 'l' (long), 's' (short) or 'sx' (short, extra ram)
    queue = 'sx'

    #: Name of the ROOT output file with generated and simulated events.
    def output_file_name(self, n_events=None, random_seed=None):
        """
        Create output file name depending on number of events and production
        mode that is specified in the random_seed string.
        """
        if n_events is None:
            n_events = self.n_events
        if random_seed is None:
            random_seed = self.random_seed
        return "generated_mc_N" + str(n_events) + "_" + random_seed + ".root"

    def output(self):
        """
        Generate list of output files that the task should produce.
        The task is considered finished if and only if the outputs all exist.
        """
        yield self.add_to_output(self.output_file_name())

    def requires(self):
        """
        This task requires several GenerateSimTask to be finished so that he required number of events is created.
        """
        n_events_per_task = MainTask.n_events_per_task
        quotient, remainder = divmod(self.n_events, n_events_per_task)
        for i in range(quotient):
            yield GenerateSimTask(
                bkgfiles_dir=self.bkgfiles_dir,
                num_processes=MainTask.num_processes,
                random_seed=self.random_seed + '_' + str(i).zfill(3),
                n_events=n_events_per_task,
                experiment_number=self.experiment_number,
                )
        if remainder > 0:
            yield GenerateSimTask(
                bkgfiles_dir=self.bkgfiles_dir,
                num_processes=MainTask.num_processes,
                random_seed=self.random_seed + '_' + str(quotient).zfill(3),
                n_events=remainder,
                experiment_number=self.experiment_number,
                )

    @b2luigi.on_temporary_files
    def process(self):
        """
        When all GenerateSimTasks finished, merge the output.
        """
        create_output_dirs(self)

        file_list = []
        for _, file_name in self.get_input_file_names().items():
            file_list.append(*file_name)
        print("Merge the following files:")
        print(file_list)
        cmd = ["b2file-merge", "-f"]
        args = cmd + [self.get_output_file_name(self.output_file_name())] + file_list
        subprocess.check_call(args)
        print("Finished merging. Now remove the input files to save space.")
        cmd2 = ["rm", "-f"]
        for tempfile in file_list:
            args = cmd2 + [tempfile]
            subprocess.check_call(args)


class StateRecordingTask(Basf2PathTask):
    """
    Record the data for the three state filters for the CDCToSVDSpacePointCKF.

    This task requires that the events used for training have been simulated before, which is done using the
    ``SplitMergeSimTask``.
    """
    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
    #: Random basf2 seed. It is further used to read of the production process to preserve
    # clearness in the b2luigi output.
    random_seed = b2luigi.Parameter()
    #: Number of events to generate for training.
    n_events = b2luigi.IntParameter()

    #: Layer on which to toggle for recording the information for training.
    layer = b2luigi.IntParameter()

    def output(self):
        """
        Generate list of output files that the task should produce.
        The task is considered finished if and only if the outputs all exist.
        """
        for record_fname in ["records1.root", "records2.root", "records3.root"]:
            yield self.add_to_output(record_fname)

    def requires(self):
        """
        This task only requires that the input files have been created.
        """
        yield SplitNMergeSimTask(
            bkgfiles_dir=MainTask.bkgfiles_by_exp[self.experiment_number],
            experiment_number=self.experiment_number,
            random_seed=self.random_seed,
            n_events=self.n_events,
        )

    def create_state_recording_path(self, layer, records1_fname, records2_fname, records3_fname):
        """
        Create a path for the recording. To record the data for the SVD state filters, CDC tracks are required, and these must
        be truth matched before. The data have to recorded for each layer of the SVD, i.e. layers 3 to 6, but also an artificial
        layer 7.

        :param layer: The layer for which the data are recorded,
        :param records1_fname: Name of the records1 file.
        :param records2_fname: Name of the records2 file.
        :param records3_fname: Name of the records3 file.
        """
        path = basf2.create_path()

        file_list = []
        for _, file_name in self.get_input_file_names().items():
            file_list.append(*file_name)
        file_list = [x for x in file_list if ("generated_mc_N" in x and "training" in x and ".root" in x)]
        path.add_module("RootInput", inputFileNames=file_list)

        path.add_module("Gearbox")
        path.add_module("Geometry")
        path.add_module("SetupGenfitExtrapolation")

        add_hit_preparation_modules(path, components=["SVD"])

        add_track_finding(path, reco_tracks="CDCRecoTracks", components=["CDC"], prune_temporary_tracks=False)

        path.add_module('TrackFinderMCTruthRecoTracks',
                        RecoTracksStoreArrayName="MCRecoTracks",
                        WhichParticles=[],
                        UsePXDHits=True,
                        UseSVDHits=True,
                        UseCDCHits=True)

        path.add_module("MCRecoTracksMatcher", UsePXDHits=False, UseSVDHits=False, UseCDCHits=True,
                        mcRecoTracksStoreArrayName="MCRecoTracks",
                        prRecoTracksStoreArrayName="CDCRecoTracks")
        path.add_module("DAFRecoFitter", recoTracksStoreArrayName="CDCRecoTracks")

        path.add_module("CDCToSVDSpacePointCKF",
                        inputRecoTrackStoreArrayName="CDCRecoTracks",
                        outputRecoTrackStoreArrayName="VXDRecoTracks",
                        outputRelationRecoTrackStoreArrayName="CDCRecoTracks",

                        relationCheckForDirection="backward",
                        reverseSeed=False,
                        writeOutDirection="backward",

                        firstHighFilter="truth",
                        firstEqualFilter="recording",
                        firstEqualFilterParameters={"treeName": "records1", "rootFileName":
                                                    records1_fname, "returnWeight": 1.0},
                        firstLowFilter="none",
                        firstHighUseNStates=0,
                        firstToggleOnLayer=layer,

                        advanceHighFilter="advance",

                        secondHighFilter="truth",
                        secondEqualFilter="recording",
                        secondEqualFilterParameters={"treeName": "records2", "rootFileName":
                                                     records2_fname, "returnWeight": 1.0},
                        secondLowFilter="none",
                        secondHighUseNStates=0,
                        secondToggleOnLayer=layer,

                        updateHighFilter="fit",

                        thirdHighFilter="truth",
                        thirdEqualFilter="recording",
                        thirdEqualFilterParameters={"treeName": "records3", "rootFileName": records3_fname},
                        thirdLowFilter="none",
                        thirdHighUseNStates=0,
                        thirdToggleOnLayer=layer,

                        filter="none",
                        exportTracks=False,

                        enableOverlapResolving=False)

        return path

    def create_path(self):
        """
        Create basf2 path to process with event generation and simulation.
        """
        return self.create_state_recording_path(
            layer=self.layer,
            records1_fname=self.get_output_file_name("records1.root"),
            records2_fname=self.get_output_file_name("records2.root"),
            records3_fname=self.get_output_file_name("records3.root"),
        )


class CKFStateFilterTeacherTask(Basf2Task):
    """
    A teacher task runs the basf2 mva teacher on the training data provided by a
    data collection task.

    In this task the three state filters are trained, each with the corresponding recordings from the different layers.
    It will be executed for each FastBDT option defined in the MainTask.
    """
    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
    #: Random basf2 seed. It is further used to read of the production process to preserve
    # clearness in the b2luigi output.
    random_seed = b2luigi.Parameter()
    #: Number of events to generate for the training data set.
    n_events = b2luigi.IntParameter()
    #: Hyperparameter option of the FastBDT algorithm. default are the FastBDT default values.
    fast_bdt_option = b2luigi.ListParameter(
        #: \cond
        hashed=True, default=[50, 8, 3, 0.1]
        #: \endcond
    )
    #: Number of the filter for which the records files are to be processed
    filter_number = b2luigi.IntParameter()
    #: Feature/variable to use as truth label in the quality estimator MVA classifier.
    training_target = b2luigi.Parameter(
        #: \cond
        default="truth"
        #: \endcond
    )
    #: List of collected variables to not use in the training of the QE MVA classifier.
    # In addition to variables containing the "truth" substring, which are excluded by default.
    exclude_variables = b2luigi.ListParameter(
        #: \cond
        # hashed=True, default=[]
        hashed=True, default=[
                "id",
                "last_id",
                "number",
                "last_layer",

                "seed_cdc_hits",
                "seed_svd_hits",
                "seed_lowest_svd_layer",
                "seed_lowest_cdc_layer",
                "quality_index_triplet",
                "quality_index_circle",
                "quality_index_helix",
                "cluster_1_charge",
                "cluster_2_charge",
                "mean_rest_cluster_charge",
                "min_rest_cluster_charge",
                "std_rest_cluster_charge",
                "cluster_1_seed_charge",
                "cluster_2_seed_charge",
                "mean_rest_cluster_seed_charge",
                "min_rest_cluster_seed_charge",
                "std_rest_cluster_seed_charge",
                "cluster_1_size",
                "cluster_2_size",
                "mean_rest_cluster_size",
                "min_rest_cluster_size",
                "std_rest_cluster_size",
                "cluster_1_snr",
                "cluster_2_snr",
                "mean_rest_cluster_snr",
                "min_rest_cluster_snr",
                "std_rest_cluster_snr",
                "cluster_1_charge_over_size",
                "cluster_2_charge_over_size",
                "mean_rest_cluster_charge_over_size",
                "min_rest_cluster_charge_over_size",
                "std_rest_cluster_charge_over_size",
        ]
        #: \endcond
    )

    def get_weightfile_xml_identifier(self, fast_bdt_option=None, filter_number=1):
        """
        Name of the xml weightfile that is created by the teacher task.
        It is subsequently used as a local weightfile in the following validation tasks.
        """
        if fast_bdt_option is None:
            fast_bdt_option = self.fast_bdt_option
        fast_bdt_string = create_fbdt_option_string(fast_bdt_option)
        weightfile_name = f"trk_CDCToSVDSpacePointStateFilter_{filter_number}" + fast_bdt_string
        return weightfile_name + ".xml"

    def requires(self):
        """
        This task requires that the recordings for the state filters.
        """
        for layer in [3, 4, 5, 6, 7]:
            yield self.clone(
                StateRecordingTask,
                experiment_number=self.experiment_number,
                n_events=self.n_events,
                random_seed="training",
                layer=layer,
            )

    def output(self):
        """
        Generate list of output files that the task should produce.
        The task is considered finished if and only if the outputs all exist.
        """
        yield self.add_to_output(self.get_weightfile_xml_identifier(filter_number=self.filter_number))

    def process(self):
        """
        Use basf2_mva teacher to create MVA weightfile from collected training
        data variables.

        This is the main process that is dispatched by the ``run`` method that
        is inherited from ``Basf2Task``.
        """
        records_files = self.get_input_file_names(f"records{self.filter_number}.root")
        tree_name = f"records{self.filter_number}"
        print(f"Processed records files: {records_files=},\nfeature tree name: {tree_name=}")

        my_basf2_mva_teacher(
            records_files=records_files,
            tree_name=tree_name,
            weightfile_identifier=self.get_output_file_name(self.get_weightfile_xml_identifier(filter_number=self.filter_number)),
            target_variable=self.training_target,
            exclude_variables=self.exclude_variables,
            fast_bdt_option=self.fast_bdt_option,
        )


class ResultRecordingTask(Basf2PathTask):
    """
    Task to record data for the final result filter. This requires trained state filters.
    The cuts on the state filter classifiers are set to rather low values to ensure that all signal is contained in the
    recorded file. Also, the values for XXXXXHighUseNStates are chosen conservatively, i.e. rather on the high side.
    """

    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
    #: Random basf2 seed. It is further used to read of the production process to preserve
    # clearness in the b2luigi output.
    random_seed = b2luigi.Parameter()
    #: Number of events to generate for the training data set.
    n_events = b2luigi.IntParameter()
    #: Hyperparameter option of the FastBDT algorithm. default are the FastBDT default values.
    fast_bdt_option = b2luigi.ListParameter(
        #: \cond
        hashed=True, default=[50, 8, 3, 0.1]
        #: \endcond
    )
    #: Name of the records file for training the final result filter
    result_filter_records_name = b2luigi.Parameter()

    def output(self):
        """
        Generate list of output files that the task should produce.
        The task is considered finished if and only if the outputs all exist.
        """
        yield self.add_to_output(self.result_filter_records_name)

    def requires(self):
        """
        This task requires that the training SplitMergeSimTask is finished, as well as that the state filters are trained
        using the CKFStateFilterTeacherTask..
        """
        yield SplitNMergeSimTask(
            bkgfiles_dir=MainTask.bkgfiles_by_exp[self.experiment_number],
            experiment_number=self.experiment_number,
            random_seed=self.random_seed,
            n_events=self.n_events,
        )
        filter_numbers = [1, 2, 3]
        for filter_number in filter_numbers:
            yield self.clone(
                CKFStateFilterTeacherTask,
                experiment_number=self.experiment_number,
                n_events=self.n_events,
                random_seed=self.random_seed,
                filter_number=filter_number,
                fast_bdt_option=self.fast_bdt_option
            )

    def create_result_recording_path(self, result_filter_records_name):
        """
        Create a path for the recording of the result filter. This file is then used to train the result filter.

        :param result_filter_records_name: Name of the recording file.
        """

        path = basf2.create_path()

        file_list = []
        for _, file_name in self.get_input_file_names().items():
            file_list.append(*file_name)
        file_list = [x for x in file_list if ("generated_mc_N" in x and "training" in x and ".root" in x)]
        path.add_module("RootInput", inputFileNames=file_list)

        path.add_module("Gearbox")
        path.add_module("Geometry")
        path.add_module("SetupGenfitExtrapolation")

        add_hit_preparation_modules(path, components=["SVD"])

        add_track_finding(path, reco_tracks="CDCRecoTracks", components=["CDC"], prune_temporary_tracks=False)

        path.add_module('TrackFinderMCTruthRecoTracks',
                        RecoTracksStoreArrayName="MCRecoTracks",
                        WhichParticles=[],
                        UsePXDHits=True,
                        UseSVDHits=True,
                        UseCDCHits=True)

        path.add_module("MCRecoTracksMatcher", UsePXDHits=False, UseSVDHits=False, UseCDCHits=True,
                        mcRecoTracksStoreArrayName="MCRecoTracks",
                        prRecoTracksStoreArrayName="CDCRecoTracks")
        path.add_module("DAFRecoFitter", recoTracksStoreArrayName="CDCRecoTracks")

        fast_bdt_string = create_fbdt_option_string(self.fast_bdt_option)
        path.add_module("CDCToSVDSpacePointCKF",
                        inputRecoTrackStoreArrayName="CDCRecoTracks",
                        outputRecoTrackStoreArrayName="VXDRecoTracks",
                        outputRelationRecoTrackStoreArrayName="CDCRecoTracks",

                        relationCheckForDirection="backward",
                        reverseSeed=False,
                        writeOutDirection="backward",

                        firstHighFilter="mva_with_direction_check",
                        firstHighFilterParameters={
                            "identifier": self.get_input_file_names(f"trk_CDCToSVDSpacePointStateFilter_1{fast_bdt_string}.xml")[0],
                            "cut": 0.001,
                            "direction": "backward"},
                        firstHighUseNStates=10,

                        advanceHighFilter="advance",
                        advanceHighFilterParameters={"direction": "backward"},

                        secondHighFilter="mva",
                        secondHighFilterParameters={
                            "identifier": self.get_input_file_names(f"trk_CDCToSVDSpacePointStateFilter_2{fast_bdt_string}.xml")[0],
                            "cut": 0.001},
                        secondHighUseNStates=10,

                        updateHighFilter="fit",

                        thirdHighFilter="mva",
                        thirdHighFilterParameters={
                            "identifier": self.get_input_file_names(f"trk_CDCToSVDSpacePointStateFilter_3{fast_bdt_string}.xml")[0],
                            "cut": 0.001},
                        thirdHighUseNStates=10,

                        filter="recording",
                        filterParameters={"rootFileName": result_filter_records_name},
                        exportTracks=False,

                        enableOverlapResolving=True)

        return path

    def create_path(self):
        """
        Create basf2 path to process with event generation and simulation.
        """
        return self.create_result_recording_path(
            result_filter_records_name=self.get_output_file_name(self.result_filter_records_name),
        )


class CKFResultFilterTeacherTask(Basf2Task):
    """
    A teacher task runs the basf2 mva teacher on the training data provided by a
    data collection task.

    Since teacher tasks are needed for all quality estimators covered by this
    steering file and the only thing that changes is the required data
    collection task and some training parameters, I decided to use inheritance
    and have the basic functionality in this base class/interface and have the
    specific teacher tasks inherit from it.
    """
    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
    #: Random basf2 seed. It is further used to read of the production process to preserve
    # clearness in the b2luigi output.
    random_seed = b2luigi.Parameter()
    #: Number of events to generate for the training data set.
    n_events = b2luigi.IntParameter()
    #: Hyperparameter option of the FastBDT algorithm. default are the FastBDT default values.
    fast_bdt_option_state_filter = b2luigi.ListParameter(
        #: \cond
        hashed=True, default=[50, 8, 3, 0.1]
        #: \endcond
    )
    #: Hyperparameter option of the FastBDT algorithm. default are the FastBDT default values.
    fast_bdt_option_result_filter = b2luigi.ListParameter(
        #: \cond
        hashed=True, default=[200, 8, 3, 0.1]
        #: \endcond
    )
    #: Name of the input file name
    result_filter_records_name = b2luigi.Parameter()
    #: Feature/variable to use as truth label in the quality estimator MVA classifier.
    training_target = b2luigi.Parameter(
        #: \cond
        default="truth"
        #: \endcond
    )
    #: List of collected variables to not use in the training of the QE MVA classifier.
    # In addition to variables containing the "truth" substring, which are excluded by default.
    exclude_variables = b2luigi.ListParameter(
        #: \cond
        hashed=True, default=[]
        #: \endcond
    )

    def get_weightfile_xml_identifier(self, fast_bdt_option=None):
        """
        Name of the xml weightfile that is created by the teacher task.
        It is subsequently used as a local weightfile in the following validation tasks.
        """
        if fast_bdt_option is None:
            fast_bdt_option = self.fast_bdt_option_result_filter
        fast_bdt_string = create_fbdt_option_string(fast_bdt_option)
        weightfile_name = "trk_CDCToSVDSpacePointResultFilter" + fast_bdt_string
        return weightfile_name + ".xml"

    def requires(self):
        """
        Generate list of luigi Tasks that this Task depends on.
        """
        yield ResultRecordingTask(
                experiment_number=self.experiment_number,
                n_events=self.n_events,
                random_seed=self.random_seed,
                fast_bdt_option=self.fast_bdt_option_state_filter,
                result_filter_records_name=self.result_filter_records_name,
        )

    def output(self):
        """
        Generate list of output files that the task should produce.
        The task is considered finished if and only if the outputs all exist.
        """
        yield self.add_to_output(self.get_weightfile_xml_identifier())

    def process(self):
        """
        Use basf2_mva teacher to create MVA weightfile from collected training
        data variables.

        This is the main process that is dispatched by the ``run`` method that
        is inherited from ``Basf2Task``.
        """
        records_files = self.get_input_file_names(self.result_filter_records_name)
        tree_name = "records"
        print(f"Processed records files for result filter training: {records_files=},\nfeature tree name: {tree_name=}")

        my_basf2_mva_teacher(
            records_files=records_files,
            tree_name=tree_name,
            weightfile_identifier=self.get_output_file_name(self.get_weightfile_xml_identifier()),
            target_variable=self.training_target,
            exclude_variables=self.exclude_variables,
            fast_bdt_option=self.fast_bdt_option_result_filter,
        )


class ValidationAndOptimisationTask(Basf2PathTask):
    """
    Validate the performance of the trained filters by trying various combinations of FastBDT options, as well as cut values
    for the states, the number of best candidates kept after each filter, and similar for the result filter.
    """
    #: Experiment number of the conditions database, e.g. defines simulation geometry.
    experiment_number = b2luigi.IntParameter()
    #: Number of events to generate for the training data set.
    n_events_training = b2luigi.IntParameter()
    #: FastBDT option to use to train the StateFilters
    fast_bdt_option_state_filter = b2luigi.ListParameter(
        # #: \cond
        hashed=True, default=[50, 8, 3, 0.1]
        # #: \endcond
    )
    #: FastBDT option to use to train the Result Filter
    fast_bdt_option_result_filter = b2luigi.ListParameter(
        # #: \cond
        hashed=True, default=[200, 8, 3, 0.1]
        # #: \endcond
    )
    #: Number of events to generate for the testing, validation, and optimisation data set.
    n_events_testing = b2luigi.IntParameter()
    #: Value of the cut on the MVA classifier output for accepting a state during CKF tracking.
    state_filter_cut = b2luigi.FloatParameter()
    #: How many states should be kept at maximum in the combinatorial part of the CKF tree search.
    use_n_best_states = b2luigi.IntParameter()
    #: Value of the cut on the MVA classifier output for a result candidate.
    result_filter_cut = b2luigi.FloatParameter()
    #: How many results should be kept at maximum to search for overlaps.
    use_n_best_results = b2luigi.IntParameter()

    def output(self):
        """
        Generate list of output files that the task should produce.
        The task is considered finished if and only if the outputs all exist.
        """
        fbdt_state_filter_string = create_fbdt_option_string(self.fast_bdt_option_state_filter)
        fbdt_result_filter_string = create_fbdt_option_string(self.fast_bdt_option_result_filter)
        yield self.add_to_output(
            f"cdc_to_svd_spacepoint_ckf_validation{fbdt_state_filter_string}_{fbdt_result_filter_string}.root")

    def requires(self):
        """
        This task requires trained result filters, trained state filters, and that an independent data set for validation was
        created using the SplitMergeSimTask with the random seed optimisation.
        """
        fbdt_state_filter_string = create_fbdt_option_string(self.fast_bdt_option_state_filter)
        yield CKFResultFilterTeacherTask(
            result_filter_records_name=f"filter_records{fbdt_state_filter_string}.root",
            experiment_number=self.experiment_number,
            n_events=self.n_events_training,
            fast_bdt_option_state_filter=self.fast_bdt_option_state_filter,
            fast_bdt_option_result_filter=self.fast_bdt_option_result_filter,
            random_seed='training'
        )
        yield SplitNMergeSimTask(
            bkgfiles_dir=MainTask.bkgfiles_by_exp[self.experiment_number],
            experiment_number=self.experiment_number,
            n_events=self.n_events_testing,
            random_seed="optimisation",
        )
        filter_numbers = [1, 2, 3]
        for filter_number in filter_numbers:
            yield self.clone(
                CKFStateFilterTeacherTask,
                experiment_number=self.experiment_number,
                random_seed="training",
                n_events=self.n_events_training,
                filter_number=filter_number,
                fast_bdt_option=self.fast_bdt_option_state_filter
            )

    def create_optimisation_and_validation_path(self):
        """
        Create a path to validate the trained filters.
        """
        path = basf2.create_path()

        file_list = []
        for _, file_name in self.get_input_file_names().items():
            file_list.append(*file_name)
        file_list = [x for x in file_list if ("generated_mc_N" in x and "optimisation" in x and ".root" in x)]
        path.add_module("RootInput", inputFileNames=file_list)

        path.add_module("Gearbox")
        path.add_module("Geometry")
        path.add_module("SetupGenfitExtrapolation")

        add_hit_preparation_modules(path, components=["SVD"])

        add_track_finding(path, reco_tracks="CDCRecoTracks", components=["CDC"], prune_temporary_tracks=False)

        fbdt_state_filter_string = create_fbdt_option_string(self.fast_bdt_option_state_filter)
        fbdt_result_filter_string = create_fbdt_option_string(self.fast_bdt_option_result_filter)
        path.add_module("CDCToSVDSpacePointCKF",

                        inputRecoTrackStoreArrayName="CDCRecoTracks",
                        outputRecoTrackStoreArrayName="VXDRecoTracks",
                        outputRelationRecoTrackStoreArrayName="CDCRecoTracks",

                        relationCheckForDirection="backward",
                        reverseSeed=False,
                        writeOutDirection="backward",

                        firstHighFilter="mva_with_direction_check",
                        firstHighFilterParameters={
                            "identifier": self.get_input_file_names(
                                f"trk_CDCToSVDSpacePointStateFilter_1{fbdt_state_filter_string}.xml")[0],
                            "cut": self.state_filter_cut,
                            "direction": "backward"},
                        firstHighUseNStates=self.use_n_best_states,

                        advanceHighFilter="advance",
                        advanceHighFilterParameters={"direction": "backward"},

                        secondHighFilter="mva",
                        secondHighFilterParameters={
                            "identifier": self.get_input_file_names(
                                f"trk_CDCToSVDSpacePointStateFilter_2{fbdt_state_filter_string}.xml")[0],
                            "cut": self.state_filter_cut},
                        secondHighUseNStates=self.use_n_best_states,

                        updateHighFilter="fit",

                        thirdHighFilter="mva",
                        thirdHighFilterParameters={
                            "identifier": self.get_input_file_names(
                                f"trk_CDCToSVDSpacePointStateFilter_3{fbdt_state_filter_string}.xml")[0],
                            "cut": self.state_filter_cut},
                        thirdHighUseNStates=self.use_n_best_states,

                        filter="mva",
                        filterParameters={
                            "identifier": self.get_input_file_names(
                                f"trk_CDCToSVDSpacePointResultFilter{fbdt_result_filter_string}.xml")[0],
                            "cut": self.result_filter_cut},
                        useBestNInSeed=self.use_n_best_results,

                        exportTracks=True,
                        enableOverlapResolving=True)

        path.add_module('RelatedTracksCombiner',
                        VXDRecoTracksStoreArrayName="VXDRecoTracks",
                        CDCRecoTracksStoreArrayName="CDCRecoTracks",
                        recoTracksStoreArrayName="RecoTracks")

        path.add_module('TrackFinderMCTruthRecoTracks',
                        RecoTracksStoreArrayName="MCRecoTracks",
                        WhichParticles=[],
                        UsePXDHits=True,
                        UseSVDHits=True,
                        UseCDCHits=True)

        path.add_module("MCRecoTracksMatcher", UsePXDHits=False, UseSVDHits=True, UseCDCHits=True,
                        mcRecoTracksStoreArrayName="MCRecoTracks",
                        prRecoTracksStoreArrayName="RecoTracks")

        path.add_module(
            CombinedTrackingValidationModule(
                output_file_name=self.get_output_file_name(
                    f"cdc_to_svd_spacepoint_ckf_validation{fbdt_state_filter_string}_{fbdt_result_filter_string}.root"),
                reco_tracks_name="RecoTracks",
                mc_reco_tracks_name="MCRecoTracks",
                name="",
                contact="",
                expert_level=200))

        return path

    def create_path(self):
        """
        Create basf2 path to process with event generation and simulation.
        """
        return self.create_optimisation_and_validation_path()


class MainTask(b2luigi.WrapperTask):
    """
    Wrapper task that needs to finish for b2luigi to finish running this steering file.

    It is done if the outputs of all required subtasks exist.  It is thus at the
    top of the luigi task graph.  Edit the ``requires`` method to steer which
    tasks and with which parameters you want to run.
    """
    #: Number of events to generate for the training data set.
    n_events_training = b2luigi.get_setting(
        #: \cond
        "n_events_training", default=1000
        #: \endcond
    )
    #: Number of events to generate for the test data set.
    n_events_testing = b2luigi.get_setting(
        #: \cond
        "n_events_testing", default=500
        #: \endcond
    )
    #: Number of events per task. Used to split up the simulation tasks.
    n_events_per_task = b2luigi.get_setting(
        #: \cond
        "n_events_per_task", default=100
        #: \endcond
    )
    #: Number of basf2 processes to use in Basf2PathTasks
    num_processes = b2luigi.get_setting(
        #: \cond
        "basf2_processes_per_worker", default=0
        #: \cond
    )

    #: Dictionary with experiment numbers as keys and background directory paths as values
    bkgfiles_by_exp = b2luigi.get_setting("bkgfiles_by_exp")
    #: Transform dictionary keys (exp. numbers) from strings to int
    bkgfiles_by_exp = {int(key): val for (key, val) in bkgfiles_by_exp.items()}

    def requires(self):
        """
        Generate list of tasks that needs to be done for luigi to finish running
        this steering file.
        """

        fast_bdt_options = []
        fast_bdt_options.append([50, 8, 3, 0.1])
        fast_bdt_options.append([100, 8, 3, 0.1])
        fast_bdt_options.append([200, 8, 3, 0.1])

        experiment_numbers = b2luigi.get_setting("experiment_numbers")

        # iterate over all possible combinations of parameters from the above defined parameter lists
        for experiment_number, fast_bdt_option_state_filter, fast_bdt_option_result_filter in itertools.product(
                experiment_numbers, fast_bdt_options, fast_bdt_options
        ):

            state_filter_cuts = [0.01, 0.02, 0.03, 0.05, 0.1, 0.2]
            n_best_states_list = [3, 5, 10]
            result_filter_cuts = [0.05, 0.1, 0.2]
            n_best_results_list = [3, 5, 10]
            for state_filter_cut in state_filter_cuts:
                for n_best_states in n_best_states_list:
                    for result_filter_cut in result_filter_cuts:
                        for n_best_results in n_best_results_list:
                            yield self.clone(
                                ValidationAndOptimisationTask,
                                experiment_number=experiment_number,
                                n_events_training=self.n_events_training,
                                n_events_testing=self.n_events_testing,
                                state_filter_cut=state_filter_cut,
                                use_n_best_states=n_best_states,
                                result_filter_cut=result_filter_cut,
                                use_n_best_results=n_best_results,
                                fast_bdt_option_state_filter=fast_bdt_option_state_filter,
                                fast_bdt_option_result_filter=fast_bdt_option_result_filter,
                            )


if __name__ == "__main__":
    b2luigi.set_setting("env_script", "./setup_basf2.sh")
    b2luigi.set_setting("batch_system", "htcondor")
    workers = b2luigi.get_setting("workers", default=1)
    b2luigi.process(MainTask(), workers=workers, batch=True)
