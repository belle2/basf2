##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
combined_cdc_to_svd_ckf_mva_training
-----------------------------------------

Purpose of this script
~~~~~~~~~~~~~~~~~~~~~~

This python script is used for the training and validation of the classifiers of
the three MVA-based state filters and one result filter of the CDCToSVDSpacePointCKF.
This CKF extraplates tracks found in the CDC into the SVD and adds SVD hits using a
combinatorial tree search and a Kalman filter based track fit in each step.

To avoid mistakes, b2luigi is used to create a task chain for a combined training and
validation of all classifiers.

The order of the b2luigi tasks in this script is as follows (top to bottom):
* Two tasks to create input samples for training and testing (``GenerateSimTask`` and
``SplitNMergeSimTask``). The ``SplitNMergeSimTask`` takes a number of events to be
generated and a number of events per task to reduce runtime. It then divides the total
number of events by the number of events per task and creates as ``GenerateSimTask`` as
needed, each with a specific random seed, so that in the end the total number of
training and testing events are simulated. The individual files are then combined
by the SplitNMergeSimTask into one file each for training and testing.
* The ``StateRecordingTask`` writes out the data required for training the state
filters.
* The ``CKFStateFilterTeacherTask`` trains the state filter MVAs, using FastBDT by
default, with a given set of options.
* The ``ResultRecordingTask`` writes out the data used for the training of the result
filter MVA. This task requires that the state filters have been trained before.
* The ``CKFResultFilterTeacherTask`` trains the MVA, FastBDT per default, with a
given set of FastBDT options. This requires that the result filter records have
been created with the ``ResultRecordingTask``.
* The ``ValidationAndOptimisationTask`` uses the trained weight files and cut values
provided to run the tracking chain with the weight file under test, and also
runs the tracking validation.
* Finally, the ``MainTask`` is the "brain" of the script. It invokes the
``ValidationAndOptimisationTask`` with the different combinations of FastBDT options
and cut values on the MVA classifier output.

Due to the dependencies, the calls of the task are reversed. The MainTask
calls the ``ValidationAndOptimisationTask`` with different FastBDT options and cut
values, and the ``ValidationAndOptimisationTask`` itself calls the required teacher,
training, and simulation tasks.

Each combination of FastBDT options and state filter cut values and candidate selection
is used to train the result filter, which includes that the ``ResultRecordingTask``
is executed multiple times with different combinations of FastBDT options and cut value
and candidate selection.

b2luigi: Understanding the steering file
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

All trainings and validations are done in the correct order in this steering
file. For the purpose of creating a dependency graph, the `b2luigi
<https://b2luigi.readthedocs.io>`_ python package is used, which extends the
`luigi <https://luigi.readthedocs.io>`_ package developed by spotify.

Each task that has to be done is represented by a special class, which defines
which defines parameters, output files and which other tasks with which
parameters it depends on.  For example a teacher task, which runs
``basf2_mva_teacher.py`` to train the classifier, depends on a data collection
task which runs a reconstruction and writes out track-wise variables into a root
file for training.  An evaluation/validation task for testing the classifier
requires both the teacher task, as it needs the weightfile to be present, and
also a data collection task, because it needs a dataset for testing classifier.

The final task that defines which tasks need to be done for the steering file to
finish is the ``MainTask``. When you only want to run parts of the
training/validation pipeline, you can comment out requirements in the Master
task or replace them by lower-level tasks during debugging.

Requirements
~~~~~~~~~~~~

This steering file relies on b2luigi_ for task scheduling. It can be installed
via pip::

    python3 -m pip install [--user] b2luigi

Use the ``--user`` option if you have not rights to install python packages into
your externals (e.g. because you are using cvmfs) and install them in
``$HOME/.local`` instead.

Configuration
~~~~~~~~~~~~~

Instead of command line arguments, the b2luigi script is configured via a
``settings.json`` file. Open it in your favorite text editor and modify it to
fit to your requirements.

Usage
~~~~~

You can test the b2luigi without running it via::

    python3 combined_cdc_to_svd_ckf_mva_training.py --dry-run
    python3 combined_cdc_to_svd_ckf_mva_training.py --show-output

This will show the outputs and show potential errors in the definitions of the
luigi task dependencies.  To run the the steering file in normal (local) mode,
run::

    python3 combined_cdc_to_svd_ckf_mva_training.py

One can use the interactive luigi web interface via the central scheduler
which visualizes the task graph while it is running. Therefore, the scheduler
daemon ``luigid`` has to run in the background, which is located in
``~/.local/bin/luigid`` in case b2luigi had been installed with ``--user``. For
example, run::

    luigid --port 8886

Then, execute your steering (e.g. in another terminal) with::

    python3 combined_cdc_to_svd_ckf_mva_training.py --scheduler-port 8886

To view the web interface, open your webbrowser enter into the url bar::

    localhost:8886

If you don't run the steering file on the same machine on which you run your web
browser, you have two options:

    1. Run both the steering file and ``luigid`` remotely and use
       ssh-port-forwarding to your local host. Therefore, run on your local
       machine::

           ssh -N -f -L 8886:localhost:8886 <remote_user>@<remote_host>

    2. Run the ``luigid`` scheduler locally and use the ``--scheduler-host <your
       local host>`` argument when calling the steering file

Accessing the results / output files
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

All output files are stored in a directory structure in the ``result_path`` set in
``settings.json``. The directory tree encodes the used b2luigi parameters. This
ensures reproducibility and makes parameter searches easy. Sometimes, it is hard to
find the relevant output files. You can view the whole directory structure by
running ``tree <result_path>``. Ise the unix ``find`` command to find the files
that interest you, e.g.::

    find <result_path> -name "*.root" # find all ROOT files
"""

import itertools
import subprocess
import os

import basf2
from tracking import add_track_finding
from tracking.path_utils import add_hit_preparation_modules
from tracking.harvesting_validation.combined_module import CombinedTrackingValidationModule
import background
import simulation

from ckf_training import my_basf2_mva_teacher, create_fbdt_option_string

# wrap python modules that are used here but not in the externals into a try except block
install_helpstring_formatter = ("\nCould not find {module} python module.Try installing it via\n"
                                "  python3 -m pip install [--user] {module}\n")
try:
    import b2luigi
    from b2luigi.core.utils import create_output_dirs
    from b2luigi.basf2_helper import Basf2PathTask, Basf2Task
except ModuleNotFoundError:
    print(install_helpstring_formatter.format(module="b2luigi"))
    raise


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

        :param n_events: Number of events to simulate.
        :param random_seed: Random seed to use for the simulation to create independent samples.
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
        path.add_module(
            "EventInfoSetter", evtNumList=[self.n_events], runList=[0], expList=[self.experiment_number]
        )
        path.add_module("EvtGenInput")
        bkg_files = ""
        if self.experiment_number == 0:
            bkg_files = background.get_background_files()
        else:
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

        :param n_events: Number of events to simulate.
        :param random_seed: Random seed to use for the simulation to create independent samples.
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

        file_list = [item for sublist in self.get_input_file_names().values() for item in sublist]
        print("Merge the following files:")
        print(file_list)
        cmd = ["b2file-merge", "-f"]
        args = cmd + [self.get_output_file_name(self.output_file_name())] + file_list
        subprocess.check_call(args)
        print("Finished merging. Now remove the input files to save space.")
        for input_file in file_list:
            try:
                os.remove(input_file)
            except FileNotFoundError:
                pass


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

        :param layer: The layer for which the data are recorded.
        :param records1_fname: Name of the records1 file.
        :param records2_fname: Name of the records2 file.
        :param records3_fname: Name of the records3 file.
        """
        path = basf2.create_path()

        # get all the file names from the list of input files that are meant for training
        file_list = [fname for sublist in self.get_input_file_names().values()
                     for fname in sublist if "generated_mc_N" in fname and "training" in fname and fname.endswith(".root")]
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
    fast_bdt_option_state_filter = b2luigi.ListParameter(
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

        :param fast_bdt_option: FastBDT option that is used to train this MVA
        :param filter_number: Filter number (first=1, second=2, third=3) to be trained
        """
        if fast_bdt_option is None:
            fast_bdt_option = self.fast_bdt_option_state_filter
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
            fast_bdt_option=self.fast_bdt_option_state_filter,
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
    fast_bdt_option_state_filter = b2luigi.ListParameter(
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
                fast_bdt_option=self.fast_bdt_option_state_filter
            )

    def create_result_recording_path(self, result_filter_records_name):
        """
        Create a path for the recording of the result filter. This file is then used to train the result filter.

        :param result_filter_records_name: Name of the recording file.
        """

        path = basf2.create_path()

        # get all the file names from the list of input files that are meant for training
        file_list = [fname for sublist in self.get_input_file_names().values()
                     for fname in sublist if "generated_mc_N" in fname and "training" in fname and fname.endswith(".root")]
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

        fast_bdt_string = create_fbdt_option_string(self.fast_bdt_option_state_filter)
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

        :param fast_bdt_option: FastBDT option that is used to train this MVA
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
                fast_bdt_option_state_filter=self.fast_bdt_option_state_filter,
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

        # get all the file names from the list of input files that are meant for optimisation / validation
        file_list = [fname for sublist in self.get_input_file_names().values()
                     for fname in sublist if "generated_mc_N" in fname and "optimisation" in fname and fname.endswith(".root")]
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
        #: \endcond
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

        fast_bdt_options = [
            [50, 8, 3, 0.1],
            [100, 8, 3, 0.1],
            [200, 8, 3, 0.1],
        ]

        experiment_numbers = b2luigi.get_setting("experiment_numbers")

        # iterate over all possible combinations of parameters from the above defined parameter lists
        for experiment_number, fast_bdt_option_state_filter, fast_bdt_option_result_filter in itertools.product(
                experiment_numbers, fast_bdt_options, fast_bdt_options
        ):

            state_filter_cuts = [0.01, 0.02, 0.03, 0.05, 0.1, 0.2]
            n_best_states_list = [3, 5, 10]
            result_filter_cuts = [0.05, 0.1, 0.2]
            n_best_results_list = [3, 5, 10]
            for state_filter_cut, n_best_states, result_filter_cut, n_best_results in \
                    itertools.product(state_filter_cuts, n_best_states_list, result_filter_cuts, n_best_results_list):
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
