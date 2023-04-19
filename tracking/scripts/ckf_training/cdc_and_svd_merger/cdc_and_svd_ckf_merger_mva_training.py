##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
cdc_and_svd_ckf_merger_mva_training
-----------------------------------------

Purpose of this script
~~~~~~~~~~~~~~~~~~~~~~

This python script is used for the training and validation of the classifier of
the MVA-based result filter of the CDCToSVDSeedCKF, which combines tracks that
were found by the CDC and SVD standalone tracking algorithms.

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
* The ``ResultRecordingTask`` writes out the data used for training of the MVA.
* The ``CKFResultFilterTeacherTask`` trains the MVA, FastBDT per default, with a
given set of FastBDT options.
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

    python3 cdc_and_svd_ckf_merger_mva_training.py --dry-run
    python3 cdc_and_svd_ckf_merger_mva_training.py --show-output

This will show the outputs and show potential errors in the definitions of the
luigi task dependencies.  To run the the steering file in normal (local) mode,
run::

    python3 cdc_and_svd_ckf_merger_mva_training.py

One can use the interactive luigi web interface via the central scheduler
which visualizes the task graph while it is running. Therefore, the scheduler
daemon ``luigid`` has to run in the background, which is located in
``~/.local/bin/luigid`` in case b2luigi had been installed with ``--user``. For
example, run::

    luigid --port 8886

Then, execute your steering (e.g. in another terminal) with::

    python3 cdc_and_svd_ckf_merger_mva_training.py --scheduler-port 8886

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

All output files are stored in a directory structure in the ``result_path``. The
directory tree encodes the used b2luigi parameters. This ensures reproducibility
and makes parameter searches easy. Sometimes, it is hard to find the relevant
output files. You can view the whole directory structure by running ``tree
<result_path>``. Ise the unix ``find`` command to find the files that interest
you, e.g.::

    find <result_path> -name "*.root" # find all ROOT files
"""

import itertools
import os
import subprocess

import basf2
# from tracking import add_track_finding
from tracking.path_utils import add_hit_preparation_modules, add_cdc_track_finding, add_svd_standalone_tracking
from tracking.harvesting_validation.combined_module import CombinedTrackingValidationModule
import background
import simulation
from packaging import version

from ckf_training import my_basf2_mva_teacher, create_fbdt_option_string

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


class GenerateSimTask(Basf2PathTask):
    """
    Generate simulated Monte Carlo with background overlay.

    Make sure to use different ``random_seed`` parameters for the training data
    format the classifier trainings and for the test data for the respective
    evaluation/validation tasks.
    """

    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
    #: Number of events to generate.
    n_events = b2luigi.IntParameter()
    #: Random basf2 seed. It is further used to read of the production process to preserve
    # clearness in the b2luigi output.
    random_seed = b2luigi.Parameter()
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

        :param n_events Number of events to simulate.
        :param random_seed Random seed to use for the simulation to create independent samples.
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
    #: Number of events to generate.
    n_events = b2luigi.IntParameter()
    #: Random basf2 seed. It is further used to read of the production process to preserve
    # clearness in the b2luigi output.
    random_seed = b2luigi.Parameter()
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

        :param n_events Number of events to simulate.
        :param random_seed Random seed to use for the simulation to create independent samples.
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


class ResultRecordingTask(Basf2PathTask):
    """
    Task to record data for the final result filter. This only requires found and MC-matched SVD and CDC tracks that need to be
    merged, all state filters are set to "all"
    """

    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
    #: Number of events to generate.
    n_events_training = b2luigi.IntParameter()
    #: Random basf2 seed. It is further used to read of the production process to preserve
    # clearness in the b2luigi output.
    random_seed = b2luigi.Parameter()

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
        This task requires that the training SplitMergeSimTask is finished.
        """
        yield SplitNMergeSimTask(
            bkgfiles_dir=MainTask.bkgfiles_by_exp[self.experiment_number],
            random_seed=self.random_seed,
            n_events=self.n_events_training,
            experiment_number=self.experiment_number,
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

        # MCTrackFinding
        mc_reco_tracks = "MCRecoTracks"
        path.add_module('TrackFinderMCTruthRecoTracks',
                        RecoTracksStoreArrayName=mc_reco_tracks)

        # CDC track finding and MC matching
        cdc_reco_tracks = "CDCRecoTracks"
        add_cdc_track_finding(path, output_reco_tracks=cdc_reco_tracks)
        path.add_module("MCRecoTracksMatcher", UsePXDHits=False, UseSVDHits=False, UseCDCHits=True,
                        mcRecoTracksStoreArrayName=mc_reco_tracks,
                        prRecoTracksStoreArrayName=cdc_reco_tracks)

        path.add_module("DAFRecoFitter", recoTracksStoreArrayName=cdc_reco_tracks)

        # SVD track finding and MC matching
        svd_reco_tracks = "SVDRecoTracks"
        add_svd_standalone_tracking(path, reco_tracks=svd_reco_tracks)
        path.add_module("MCRecoTracksMatcher", UsePXDHits=False, UseSVDHits=True, UseCDCHits=False,
                        mcRecoTracksStoreArrayName=mc_reco_tracks,
                        prRecoTracksStoreArrayName=svd_reco_tracks)

        direction = "backward"
        path.add_module("CDCToSVDSeedCKF",
                        inputRecoTrackStoreArrayName=cdc_reco_tracks,

                        fromRelationStoreArrayName=cdc_reco_tracks,
                        toRelationStoreArrayName=svd_reco_tracks,

                        relatedRecoTrackStoreArrayName=svd_reco_tracks,
                        cdcTracksStoreArrayName=cdc_reco_tracks,
                        vxdTracksStoreArrayName=svd_reco_tracks,

                        relationCheckForDirection=direction,
                        reverseSeed=False,
                        firstHighFilterParameters={"direction": direction},
                        advanceHighFilterParameters={"direction": direction},

                        writeOutDirection=direction,
                        endEarly=False,

                        filter="recording_with_relations",
                        filterParameters={"rootFileName": result_filter_records_name})

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
    #: Number of events to generate for the training data set.
    n_events_training = b2luigi.IntParameter()
    #: Random basf2 seed. It is further used to read of the production process to preserve
    # clearness in the b2luigi output.
    random_seed = b2luigi.Parameter()
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
    #: Hyperparameter option of the FastBDT algorithm. default are the FastBDT default values.
    fast_bdt_option = b2luigi.ListParameter(
        #: \cond
        hashed=True, default=[200, 8, 3, 0.1]
        #: \endcond
    )

    def get_weightfile_xml_identifier(self, fast_bdt_option=None):
        """
        Name of the xml weightfile that is created by the teacher task.
        It is subsequently used as a local weightfile in the following validation tasks.

        :param fast_bdt_option FastBDT option that is used to train this MVA
        """
        if fast_bdt_option is None:
            fast_bdt_option = self.fast_bdt_option
        fast_bdt_string = create_fbdt_option_string(fast_bdt_option)
        weightfile_name = "trk_CDCToSVDSeedResultFilter" + fast_bdt_string
        return weightfile_name + ".xml"

    def requires(self):
        """
        Generate list of luigi Tasks that this Task depends on.
        """
        yield ResultRecordingTask(
                experiment_number=self.experiment_number,
                n_events_training=self.n_events_training,
                result_filter_records_name=self.result_filter_records_name,
                random_seed=self.random_seed
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

        my_basf2_mva_teacher(
            records_files=records_files,
            tree_name="records",
            weightfile_identifier=self.get_output_file_name(self.get_weightfile_xml_identifier()),
            target_variable=self.training_target,
            exclude_variables=self.exclude_variables,
            fast_bdt_option=self.fast_bdt_option,
        )


class ValidationAndOptimisationTask(Basf2PathTask):
    """
    Validate the performance of the trained filters by trying various combinations of FastBDT options, as well as cut values for
    the states, the number of best candidates kept after each filter, and similar for the result filter.
    """
    #: Experiment number of the conditions database, e.g. defines simulation geometry.
    experiment_number = b2luigi.IntParameter()
    #: Number of events to generate for the training data set.
    n_events_training = b2luigi.IntParameter()
    #: FastBDT option to use to train the StateFilters
    fast_bdt_option = b2luigi.ListParameter(
        # #: \cond
        hashed=True, default=[200, 8, 3, 0.1]
        # #: \endcond
    )
    #: Number of events to generate for the testing, validation, and optimisation data set.
    n_events_testing = b2luigi.IntParameter()
    #: Value of the cut on the MVA classifier output for a result candidate.
    result_filter_cut = b2luigi.FloatParameter()

    def output(self):
        """
        Generate list of output files that the task should produce.
        The task is considered finished if and only if the outputs all exist.
        """
        fbdt_string = create_fbdt_option_string(self.fast_bdt_option)
        yield self.add_to_output(
            f"cdc_svd_merger_ckf_validation{fbdt_string}_{self.result_filter_cut}.root")

    def requires(self):
        """
        This task requires trained result filters, and that an independent data set for validation was created using the
        ``SplitMergeSimTask`` with the random seed optimisation.
        """
        yield CKFResultFilterTeacherTask(
            result_filter_records_name="filter_records.root",
            experiment_number=self.experiment_number,
            n_events_training=self.n_events_training,
            fast_bdt_option=self.fast_bdt_option,
            random_seed='training'
        )
        yield SplitNMergeSimTask(
            bkgfiles_dir=MainTask.bkgfiles_by_exp[self.experiment_number],
            experiment_number=self.experiment_number,
            n_events=self.n_events_testing,
            random_seed="optimisation",
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

        cdc_reco_tracks = "CDCRecoTracks"
        svd_reco_tracks = "SVDRecoTracks"
        reco_tracks = "RecoTracks"
        mc_reco_tracks = "MCRecoTracks"

        # CDC track finding and MC matching
        add_cdc_track_finding(path, output_reco_tracks=cdc_reco_tracks)

        path.add_module("DAFRecoFitter", recoTracksStoreArrayName=cdc_reco_tracks)

        # SVD track finding and MC matching
        add_svd_standalone_tracking(path, reco_tracks=svd_reco_tracks)

        direction = "backward"
        fbdt_string = create_fbdt_option_string(self.fast_bdt_option)
        path.add_module(
            "CDCToSVDSeedCKF",
            inputRecoTrackStoreArrayName=cdc_reco_tracks,
            fromRelationStoreArrayName=cdc_reco_tracks,
            toRelationStoreArrayName=svd_reco_tracks,
            relatedRecoTrackStoreArrayName=svd_reco_tracks,
            cdcTracksStoreArrayName=cdc_reco_tracks,
            vxdTracksStoreArrayName=svd_reco_tracks,
            relationCheckForDirection=direction,
            reverseSeed=False,
            firstHighFilterParameters={
                "direction": direction},
            advanceHighFilterParameters={
                "direction": direction},
            writeOutDirection=direction,
            endEarly=False,
            filter='mva_with_relations',
            filterParameters={
                "identifier": self.get_input_file_names(f"trk_CDCToSVDSeedResultFilter{fbdt_string}.xml")[0],
                "cut": self.result_filter_cut})

        path.add_module('RelatedTracksCombiner',
                        VXDRecoTracksStoreArrayName=svd_reco_tracks,
                        CDCRecoTracksStoreArrayName=cdc_reco_tracks,
                        recoTracksStoreArrayName=reco_tracks)

        path.add_module('TrackFinderMCTruthRecoTracks',
                        RecoTracksStoreArrayName=mc_reco_tracks,
                        WhichParticles=[],
                        UsePXDHits=True,
                        UseSVDHits=True,
                        UseCDCHits=True)

        path.add_module("MCRecoTracksMatcher", UsePXDHits=False, UseSVDHits=True, UseCDCHits=True,
                        mcRecoTracksStoreArrayName=mc_reco_tracks,
                        prRecoTracksStoreArrayName=reco_tracks)

        path.add_module(
            CombinedTrackingValidationModule(
                output_file_name=self.get_output_file_name(
                    f"cdc_svd_merger_ckf_validation{fbdt_string}_{self.result_filter_cut}.root"),
                reco_tracks_name=reco_tracks,
                mc_reco_tracks_name=mc_reco_tracks,
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
        cut_values = []
        for i in range(4):
            cut_values.append((i+1) * 0.2)

        experiment_numbers = b2luigi.get_setting("experiment_numbers")

        # iterate over all possible combinations of parameters from the above defined parameter lists
        for experiment_number, fast_bdt_option, cut_value in itertools.product(
                experiment_numbers, fast_bdt_options, cut_values
        ):
            yield ValidationAndOptimisationTask(
                experiment_number=experiment_number,
                n_events_training=self.n_events_training,
                fast_bdt_option=fast_bdt_option,
                n_events_testing=self.n_events_testing,
                result_filter_cut=cut_value,
            )


if __name__ == "__main__":
    b2luigi.set_setting("env_script", "./setup_basf2.sh")
    b2luigi.set_setting("batch_system", "htcondor")
    workers = b2luigi.get_setting("workers", default=1)
    b2luigi.process(MainTask(), workers=workers, batch=True)
