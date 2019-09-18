#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
combined_module_quality_estimator_teacher
-----------------------------------------

Purpose of this script: The track quality estimators
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This python script is used for the combined training and validation of the
following track quality estimators (QE):

    - **MVA track quality estimator:** The final quality estimator for fully
      merged and fitted tracks.  Its classifier uses features from the track
      fitting, merger, hit pattern, ... But it also uses the outputs from
      respective intermediate quality estimators for the VXD and the CDC track
      finding as inputs.

    - **VXD track quality estimator:** Quality estimator for the VXDTF2 track
      finder.

    - **CDC track quality estimator:** Quality estimator for the CDC track
      finding.

The reason why separate quality estimators for the subdetectors is that some
variables are only available for the track finders in the subdetectors, e.g. are
available in ``CDCTrack`` objects (e.g. ADC counts), but not anymore in
``RecoTrack`` objects.  Therefore VXD and CDC quality estimators have to be
trained before the final, full track quality estimator can be trained with their
outputs as input.  The classifier outputs of the quality estimators are called
**quality indicators (QI)**

b2luigi: Understanding the steering file
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

All trainings and validations are done in the correct order in this steering
file. For the purpose of creating a dependency graph, the `b2luigi
<https://b2luigi.readthedocs.io>`_ python package is used, which extends the
`luigi <https://luigi.readthedocs.io>`_ packag developed by spotify.

Each task that has to be done is represented by a special class, which defines
which defines parameters, output files and which other tasks with which
parameters it depends on.  For example a teacher task, which runs
``basf2_mva_teacher.py`` to train the classifier, depends on a data collection
task which runs a reconstruction and writes out track-wise variables into a root
file for training.  An evaluation/validation task for testing the classifier
requires both the teacher task, as it needs the weightfile to be present, and
also a data collection task, because it needs a dataset for testing classifier.

The final task that defines which tasks need to be done for the steering file to
finish is the ``MasterTask``. When you only want to run parts of the
training/validation pipeline, you can comment out requirements in the Master
task or replace them by lower-level tasks during debugging.

Requirements
~~~~~~~~~~~~

This steering file relies on b2luigi_ for task scheduling and `uncertain_panda
<https://github.com/nils-braun/uncertain_panda>`_ for uncertainty calculations.
uncertain_panda is not in the externals and b2luigi is not upto v01-07-01. Both
can be installed via pip::

    python3 -m pip install [--user] b2luigi uncertain_panda

Use the ``--user`` option if you have not rights to install python packages into
your externals (e.g. because you are using cvmfs) and install them in
``$HOME/.local`` instead.

Configuration
~~~~~~~~~~~~~

Instead of command line arguments, the b2luigi script is configured via a
``settings.json`` file. Open it in your favorite text editor and modify it to
fit to your requirements. It should look like this (The contents in ``<...>``
represent placeholders):

.. code-block:: json
    {
        "result_path": "<Root path for the b2luigi outputs>"
        "bkgfiles_directory": "<Directory with overlay background root files>",
        "n_events_training": <Number of events to be used for training data>,
        "n_events_testing": <Number of events to be used for the validation/evaluation data>,
        "workers": <Number of luigi workers, which execute tasks in parallel.>,
        "basf2_processes_per_worker": <Number of basf2 processes per worker. 0 disables multiprocessing.>
    }

Usage
~~~~~

You can test the b2luigi without running it via::

    python3 combined_quality_estimator_teacher.py --dry-run
    python3 combined_quality_estimator_teacher.py --show-output

This will show the outputs and show potential errors in the definitions of the
luigi task dependencies.  To run the the steering file in normal (local) mode,
run::

    python3 combined_quality_estimator_teacher.py

I usually use the interactive luigi web interface via the central scheduler
which visualizes the task graph while it is running. Therefore, the scheduler
daemon ``luigid`` has to run in the background, which is located in
``~/.local/bin/luigid`` in case b2luigi had been installed with ``--user``. For
example, run::

    luigid --port 8886

Then, execute your steering (e.g. in another terminal) with::

    python3 combined_quality_estimator_teacher.py --scheduler-port 8886

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
directory tree encodes the used b2luigi parameters. This ensures reproducability
and makes parameter searches easy. Sometimes, it is hard to find the relevant
output files. You can view the whole directory structure by running ``tree
<result_path>``. Ise the unix ``find`` command to find the files that interest
you, e.g.::

    find <result_path> -name "*.pdf" # find all validation plot files
    find <result_path> -name "*.root" # find all ROOT files
"""

import errno
import glob
import os
import subprocess
import textwrap
from datetime import datetime
from typing import Iterable

import matplotlib.pyplot as plt
import numpy as np
import root_pandas
from matplotlib.backends.backend_pdf import PdfPages

import basf2
import basf2_mva
from packaging import version
import simulation
import tracking
import tracking.root_utils as root_utils
from tracking.harvesting_validation.combined_module import CombinedTrackingValidationModule

# wrap python modules that are used here but not in the externals into a try except block
install_helpstring_formatter = ("\nCould not find {module} python module.Try installing it via\n"
                                "  python3 -m pip install [--user] {module}\n")
try:
    import b2luigi
    from b2luigi.core.utils import get_serialized_parameters, get_log_file_dir
    from b2luigi.basf2_helper import Basf2PathTask, Basf2Task
    from b2luigi.core.task import Task
    from b2luigi.basf2_helper.utils import get_basf2_git_hash
except ModuleNotFoundError:
    print(install_helpstring_formatter.format(module="b2luigi"))
    raise
try:
    from uncertain_panda import pandas as upd
except ModuleNotFoundError:
    print(install_helpstring_formatter.format(module="uncertain_panda"))
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

# Utility functions


def my_basf2_mva_teacher(
    records_files,
    tree_name,
    weightfile_identifier,
    target_variable="truth",
    exclude_variables=[],
):
    """
    My custom wrapper for basf2 mva teacher. Adapted from code in ``trackfindingcdc_teacher``.
    """

    # extract names of all variables from one record file
    with root_utils.root_open(records_files[0]) as records_tfile:
        input_tree = records_tfile.Get(tree_name)
        feature_names = [leave.GetName() for leave in input_tree.GetListOfLeaves()]
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
    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector(*records_files)
    general_options.m_treename = tree_name
    general_options.m_weight_variable = weight_variable
    general_options.m_identifier = weightfile_identifier
    general_options.m_variables = basf2_mva.vector(*truth_free_variable_names)
    general_options.m_target_variable = target_variable
    fastbdt_options = basf2_mva.FastBDTOptions()
    # Train a MVA method and store the weightfile (MVAFastBDT.root) locally.
    basf2_mva.teacher(general_options, fastbdt_options)


def _my_uncertain_mean(series: upd.Series):
    """
    Temporary Workaround bug in ``uncertain_panda`` where a ``ValueError`` is
    thrown for ``Series.unc.mean`` if the series is empty.  Can be replaced by
    .unc.mean when the issue is fixed.
    https://github.com/nils-braun/uncertain_panda/issues/2
    """
    try:
        return series.unc.mean()
    except ValueError:
        if series.empty:
            return np.nan
        else:
            raise


def get_uncertain_means_for_qi_cuts(df: upd.DataFrame, column: str, qi_cuts: Iterable[float]):
    """
    Return a pandas series with an mean of the dataframe column and
    uncertainty for each quality indicator cut.

    :param df: Pandas dataframe with at least ``quality_indicator``
        and another numeric ``column``.
    :param column: Column of which we want to aggregate the means
        and uncertainties for different QI cuts
    :param qi_cuts: Iterable of quality indicator minimal thresholds.
    :returns: Series of of means and uncertainties with ``qi_cuts`` as index
    """

    uncertain_means = (_my_uncertain_mean(df.query(f"quality_indicator > {qi_cut}")[column])
                       for qi_cut in qi_cuts)
    uncertain_means_series = upd.Series(data=uncertain_means, index=qi_cuts)
    return uncertain_means_series


def plot_with_errobands(uncertain_series,
                        error_band_alpha=0.3,
                        plot_kwargs={},
                        fill_between_kwargs={},
                        ax=None):
    """
    Plot an uncertain series with error bands for y-errors
    """
    if ax is None:
        ax = plt.gca()
    uncertain_series = uncertain_series.dropna()
    ax.plot(uncertain_series.index.values, uncertain_series.nominal_value, **plot_kwargs)
    ax.fill_between(x=uncertain_series.index,
                    y1=uncertain_series.nominal_value - uncertain_series.std_dev,
                    y2=uncertain_series.nominal_value + uncertain_series.std_dev,
                    alpha=error_band_alpha,
                    **fill_between_kwargs)


def format_dictionary(adict, width=80, bullet="•"):
    """
    Helper function to format dictionary to string as a wrapped key-value bullet
    list.  Useful to print metadata from dictionaries.

    :param adict: Dictionary to format
    :param width: Characters after which to wrap a key-value line
    :param bullet: Character to begin a key-value line with, e.g. ``-`` for a
        yaml-like string
    """
    # It might be possible to replace this function yaml.dump, but the current
    # version in the externals does not allow to disable the sorting of the
    # dictionary yet and also I am not sure if it is wrappable
    return "\n".join(textwrap.fill(f"{bullet} {key}: {value}", width=width)
                     for (key, value) in adict.items())

# Begin definitions of b2luigi task classes


class GenerateSimTask(Basf2PathTask):
    """
    Generate simulated Monte Carlo with background overlay.

    Make sure to use different ``random_seed`` parameters for the training data
    format the classifier trainings and for the test data for the respective
    evaluation/validation tasks.
    """

    n_events = b2luigi.IntParameter()
    random_seed = b2luigi.Parameter()
    bkgfiles_dir = b2luigi.Parameter(hashed=True)
    output_file_name = "generated_mc.root"

    def output(self):
        yield self.add_to_output(self.output_file_name)

    def create_path(self):
        basf2.set_random_seed(self.random_seed)
        path = basf2.create_path()
        path.add_module(
            "EventInfoSetter", evtNumList=[self.n_events], runList=[0], expList=[0]
        )
        path.add_module("EvtGenInput")
        if not os.path.isdir(self.bkgfiles_dir):
            raise NotADirectoryError(errno.ENOTDIR, os.strerror(errno.ENOTDIR), self.bkgfiles_dir)
        bkg_files = glob.glob(os.path.join(self.bkgfiles_dir, "*.root"))
        if not bkg_files:
            raise FileNotFoundError(errno.ENOENT, "No *.root background files found in", self.bkgfiles_dir)
        simulation.add_simulation(path, bkgfiles=bkg_files, bkgOverlay=True)
        path.add_module(
            "RootOutput",
            outputFileName=self.get_output_file_name(self.output_file_name),
        )
        return path


class VXDQEDataCollectionTask(Basf2PathTask):
    """
    Collect variables/features from VXDTF2 tracking and write them to a ROOT
    file.

    These variables are to be used as labelled training data for the MVA
    classifier which is the VXD track quality estimator
    """
    n_events = b2luigi.IntParameter()
    random_seed = b2luigi.Parameter()
    records_file_name = "vxd_qe_records.root"

    def requires(self):
        yield GenerateSimTask(
            bkgfiles_dir=MasterTask.bkgfiles_dir,
            num_processes=self.num_processes,
            random_seed=self.random_seed,
            n_events=self.n_events,
        )

    def output(self):
        yield self.add_to_output(self.records_file_name)

    def create_path(self):
        path = basf2.create_path()
        path.add_module(
            "RootInput",
            inputFileNames=self.get_input_file_names(GenerateSimTask.output_file_name),
        )
        path.add_module("Gearbox")
        tracking.add_geometry_modules(path)
        tracking.add_hit_preparation_modules(path)  # only needed for simulated hits
        tracking.add_vxd_track_finding_vxdtf2(
            path, components=["SVD"], use_vxdtf2_quality_estimator=False
        )
        path.add_module(
            "TrackFinderMCTruthRecoTracks",
            RecoTracksStoreArrayName="MCRecoTracks",
            WhichParticles=[],
            UsePXDHits=False,
            UseSVDHits=True,
            UseCDCHits=False,
        )
        path.add_module(
            "VXDQETrainingDataCollector",
            TrainingDataOutputName=self.get_output_file_name(self.records_file_name),
            SpacePointTrackCandsStoreArrayName="SPTrackCands",
            EstimationMethod="tripletFit",
            UseTimingInfo=False,
            ClusterInformation="Average",
            MCStrictQualityEstimator=True,
            mva_target=False,
        )
        return path


class CDCQEDataCollectionTask(Basf2PathTask):
    """
    Collect variables/features from CDC tracking and write them to a ROOT file.

    These variables are to be used as labelled training data for the MVA
    classifier which is the CDC track quality estimator
    """
    n_events = b2luigi.IntParameter()
    random_seed = b2luigi.Parameter()
    records_file_name = "cdc_qe_records.root"

    def requires(self):
        yield GenerateSimTask(
            bkgfiles_dir=MasterTask.bkgfiles_dir,
            num_processes=self.num_processes,
            random_seed=self.random_seed,
            n_events=self.n_events,
        )

    def output(self):
        yield self.add_to_output(self.records_file_name)

    def create_path(self):
        path = basf2.create_path()
        path.add_module(
            "RootInput",
            inputFileNames=self.get_input_file_names(GenerateSimTask.output_file_name),
        )
        path.add_module("Gearbox")
        tracking.add_geometry_modules(path)
        tracking.add_hit_preparation_modules(path)  # only needed for simulated hits
        tracking.add_cdc_track_finding(path, use_cdc_quality_estimator=True)

        basf2.set_module_parameters(
            path,
            name="TFCDC_TrackQualityEstimator",
            filter="recording",
            filterParameters={
                "rootFileName": self.get_output_file_name(self.records_file_name)
            },
        )
        return path


class FullTrackQEDataCollectionTask(Basf2PathTask):
    """
    Collect variables/features from the full track reconstruction including the
    fit and write them to a ROOT file.

    These variables are to be used as labelled training data for the MVA
    classifier which is the MVA track quality estimator.  The collected
    variables include the classifier outputs from the VXD and CDC quality
    estimators, namely the CDC and VXD quality indicators, combined with fit,
    merger, timing, energy loss information etc.  This task requires the
    subdetector quality estimators to be trained.
    """

    n_events = b2luigi.IntParameter()
    random_seed = b2luigi.Parameter()
    records_file_name = "fulltrack_qe_records.root"
    cdc_training_target = b2luigi.Parameter()

    def requires(self):
        yield GenerateSimTask(
            bkgfiles_dir=MasterTask.bkgfiles_dir,
            num_processes=MasterTask.num_processes,
            random_seed=self.random_seed,
            n_events=self.n_events,
        )
        yield CDCQETeacherTask(
            n_events_training=MasterTask.n_events_training,
            training_target=self.cdc_training_target,
        )
        yield VXDQETeacherTask(n_events_training=MasterTask.n_events_training)

    def output(self):
        yield self.add_to_output(self.records_file_name)

    def create_path(self):
        path = basf2.create_path()
        path.add_module(
            "RootInput",
            inputFileNames=self.get_input_file_names(GenerateSimTask.output_file_name),
        )
        path.add_module("Gearbox")
        tracking.add_geometry_modules(path)
        tracking.add_hit_preparation_modules(path)  # only needed for simulated hits
        tracking.add_cdc_track_finding(
            path,
            output_reco_tracks="CDCRecoTracks",
            use_cdc_quality_estimator=True,
            cdc_quality_estimator_weightfile=self.get_input_file_names(
                CDCQETeacherTask.weightfile_identifier
            )[0],
        )
        tracking.add_svd_track_finding(
            path,
            components=None,
            input_reco_tracks="CDCRecoTracks",
            output_reco_tracks="SVDCDCRecoTracks",
            temporary_reco_tracks="SVDRecoTracks",
            temporary_svd_cdc_reco_tracks="SVDPlusCDCStandaloneRecoTracks",
            add_both_directions=True,
            use_vxdtf2_quality_estimator=True,
        )
        basf2.set_module_parameters(
            path,
            name="VXDQualityEstimatorMVA",
            WeightFileIdentifier=self.get_input_file_names(
                VXDQETeacherTask.weightfile_identifier
            )[0],
        )
        tracking.add_pxd_track_finding(
            path,
            components=None,
            input_reco_tracks="SVDCDCRecoTracks",
            output_reco_tracks="RecoTracks",
            temporary_reco_tracks="PXDRecoTracks",
            add_both_directions=True,
        )
        tracking.add_time_extraction(path)
        tracking.add_mc_matcher(path)
        tracking.add_track_fit_and_track_creator(path)
        path.add_module(
            "TrackQETrainingDataCollector",
            TrainingDataOutputName=self.get_output_file_name(self.records_file_name),
            collectEventFeatures=True,  # collect event features and then just don't train on them via ``exclude_features``
            SVDPlusCDCStandaloneRecoTracksStoreArrayName="SVDPlusCDCStandaloneRecoTracks",
        )
        return path


class TrackQETeacherBaseTask(Basf2Task):
    """
    A teacher task runs the basf2 mva teacher on the training data provided by a
    data collection task.

    Since teacher tasks are needed for all quality estimators covered by this
    steering file and the only thing that changes is the required data
    collection task and some training parameters, I decided to use inheritance
    and have the basic functionality in this base class/interface and have the
    specific teacher tasks inherit from it.
    """
    n_events_training = b2luigi.IntParameter()
    training_target = b2luigi.Parameter(default="truth")
    exclude_variables = b2luigi.ListParameter(hashed=True, default=[])

    @property
    def weightfile_identifier(self):
        """
        Property defining the name for the weightfile that is generated.  Has to
        be implemented by the inheriting specific teacher task class.
        """
        raise NotImplementedError(
            "Teacher Task must define a static weightfile_identifier"
        )

    @property
    def tree_name(self):
        """
        Property defining the name of their in the ROOT file from the
        ``dataCollectionTask`` that contains the recorded training data.  Must
        implemented by the inheriting specific teacher task class.
        """
        raise NotImplementedError("Teacher Task must define a static tree_name")

    @property
    def random_seed(self):
        """
        Property defining random seed to be used by the ``GenerateSimTask``.
        Should differ from the random seed in the test data samples.  Must
        implemented by the inheriting specific teacher task class.
        """
        raise NotImplementedError("Teacher Task must define a static random seed")

    @property
    def dataCollectionTask(self) -> Basf2PathTask:
        """
        Property defining the specific ``DataCollectionTask`` to require.  Must
        implemented by the inheriting specific teacher task class.
        """
        raise NotImplementedError(
            "Teacher Task must define a data collection task to require "
        )

    def requires(self):
        yield self.dataCollectionTask(
            num_processes=MasterTask.num_processes,
            n_events=self.n_events_training,
            random_seed=self.random_seed,
        )

    def output(self):
        yield self.add_to_output(self.weightfile_identifier)

    def process(self):
        records_files = self.get_input_file_names(
            self.dataCollectionTask.records_file_name
        )
        my_basf2_mva_teacher(
            records_files=records_files,
            tree_name=self.tree_name,
            weightfile_identifier=self.get_output_file_name(self.weightfile_identifier),
            target_variable=self.training_target,
            exclude_variables=self.exclude_variables,
        )


class VXDQETeacherTask(TrackQETeacherBaseTask):
    """
    Task to run basf2 mva teacher on collected data for VXDTF2 track quality estimator
    """
    weightfile_identifier = "trackfindingvxd_TrackQualityIndicator.weights.xml"
    tree_name = "tree"
    random_seed = "trainvxd_0"
    dataCollectionTask = VXDQEDataCollectionTask


class CDCQETeacherTask(TrackQETeacherBaseTask):
    """
    Task to run basf2 mva teacher on collected data for CDC track quality estimator
    """
    weightfile_identifier = "trackfindingcdc_TrackQualityIndicator.weights.xml"
    tree_name = "records"
    random_seed = "traincdc_0"
    dataCollectionTask = CDCQEDataCollectionTask


class FullTrackQETeacherTask(TrackQETeacherBaseTask):
    """
    Task to run basf2 mva teacher on collected data for the final, combined
    track quality estimator
    """
    weightfile_identifier = "fullTrackQualityIndicator.weights.xml"
    tree_name = "tree"
    random_seed = "trainingdata_0"
    dataCollectionTask = FullTrackQEDataCollectionTask
    cdc_training_target = b2luigi.Parameter()

    def requires(self):
        yield self.dataCollectionTask(
            cdc_training_target=self.cdc_training_target,
            num_processes=MasterTask.num_processes,
            n_events=self.n_events_training,
            random_seed=self.random_seed,
        )


class HarvestingValidationBaseTask(Basf2PathTask):
    """
    Run track reconstruction with MVA quality estimator and write out
    (="harvest") a root file with variables useful for the validation.
    """

    n_events_testing = b2luigi.IntParameter()
    n_events_training = b2luigi.IntParameter()
    validation_output_file_name = "harvesting_validation.root"
    reco_output_file_name = "reconstruction.root"
    components = None  # components for the MC matching and track fit creation

    @property
    def teacherTask(self) -> TrackQETeacherBaseTask:
        """
        Teacher task to require so that a quality estimator weightfile is
        available in ``add_tracking_with_quality_estimation``
        """
        raise NotImplementedError()

    def add_tracking_with_quality_estimation(self, path: basf2.Path) -> None:
        """
        Add modules for track reconstruction to basf2 ``path`` that are to be
        validated.  Besides track finding it should include MC matching, fitted
        track creation and a quality estimator module.
        """
        raise NotImplementedError()

    def requires(self):
        yield self.teacherTask(n_events_training=self.n_events_training)
        yield GenerateSimTask(
            bkgfiles_dir=MasterTask.bkgfiles_dir,
            num_processes=MasterTask.num_processes,
            n_events=self.n_events_testing,
            random_seed="testdata_0",
        )

    def output(self):
        yield self.add_to_output(self.validation_output_file_name)
        yield self.add_to_output(self.reco_output_file_name)

    def create_path(self):
        # prepare track finding
        path = basf2.create_path()
        path.add_module(
            "RootInput",
            inputFileNames=self.get_input_file_names(GenerateSimTask.output_file_name),
        )
        path.add_module("Gearbox")
        tracking.add_geometry_modules(path)
        tracking.add_hit_preparation_modules(path)  # only needed for simulated hits
        # add track finding module that needs to be validated
        self.add_tracking_with_quality_estimation(path)
        # add modules for validation
        path.add_module(
            CombinedTrackingValidationModule(
                name=None,
                contact=None,
                expert_level=200,
                output_file_name=self.get_output_file_name(
                    self.validation_output_file_name
                ),
            )
        )
        path.add_module(
            "RootOutput",
            outputFileName=self.get_output_file_name(self.reco_output_file_name),
        )
        return path


class VXDQEHarvestingValidationTask(HarvestingValidationBaseTask):
    """
    Run VXDTF2 track reconstruction and write out (="harvest") a root file with
    variables useful for validation of the VXD Quality Estimator.
    """

    validation_output_file_name = "vxd_qe_harvesting_validation.root"
    reco_output_file_name = "vxd_qe_reconstruction.root"

    teacherTask = VXDQETeacherTask

    def add_tracking_with_quality_estimation(self, path):
        tracking.add_vxd_track_finding_vxdtf2(
            path,
            components=["SVD"],
            reco_tracks="RecoTracks",
            use_vxdtf2_quality_estimator=True,
        )
        basf2.set_module_parameters(
            path,
            name="VXDQualityEstimatorMVA",
            WeightFileIdentifier=self.get_input_file_names(
                self.teacherTask.weightfile_identifier
            )[0],
        )
        tracking.add_mc_matcher(path, components=["SVD"])
        tracking.add_track_fit_and_track_creator(path, components=["SVD"])


class CDCQEHarvestingValidationTask(HarvestingValidationBaseTask):
    """
    Run CDC reconstruction and write out (="harvest") a root file with variables
    useful for validation of the CDC Quality Estimator.
    """
    training_target = b2luigi.Parameter()
    validation_output_file_name = "cdc_qe_harvesting_validation.root"
    reco_output_file_name = "cdc_qe_reconstruction.root"

    teacherTask = CDCQETeacherTask

    def requires(self):
        yield self.teacherTask(
            n_events_training=self.n_events_training,
            training_target=self.training_target,
        )
        yield GenerateSimTask(
            bkgfiles_dir=MasterTask.bkgfiles_dir,
            num_processes=MasterTask.num_processes,
            n_events=self.n_events_testing,
            random_seed="testdata_0",
        )

    def add_tracking_with_quality_estimation(self, path):
        tracking.add_cdc_track_finding(
            path,
            output_reco_tracks="RecoTracks",
            use_cdc_quality_estimator=True,
            cdc_quality_estimator_weightfile=self.get_input_file_names(
                self.teacherTask.weightfile_identifier
            )[0],
        )
        tracking.add_mc_matcher(path, components=["CDC"])
        tracking.add_track_fit_and_track_creator(path, components=["CDC"])


class FullTrackQEHarvestingValidationTask(HarvestingValidationBaseTask):
    """
    Run track reconstruction and write out (="harvest") a root file with variables
    useful for validation of the MVA track Quality Estimator.
    """
    n_events_testing = b2luigi.IntParameter()
    n_events_training = b2luigi.IntParameter()
    cdc_training_target = b2luigi.Parameter()
    exclude_variables = b2luigi.ListParameter(hashed=True)
    validation_output_file_name = "full_qe_harvesting_validation.root"
    reco_output_file_name = "full_qe_reconstruction.root"

    teacherTask = FullTrackQETeacherTask

    def requires(self):
        yield CDCQETeacherTask(
            n_events_training=self.n_events_training,
            training_target=self.cdc_training_target,
        )
        yield VXDQETeacherTask(n_events_training=self.n_events_training)
        yield self.teacherTask(
            n_events_training=self.n_events_training,
            exclude_variables=self.exclude_variables,
            cdc_training_target=self.cdc_training_target,
        )
        yield GenerateSimTask(
            bkgfiles_dir=MasterTask.bkgfiles_dir,
            num_processes=MasterTask.num_processes,
            n_events=self.n_events_testing,
            random_seed="testdata_0",
        )

    def add_tracking_with_quality_estimation(self, path):
        tracking.add_cdc_track_finding(
            path,
            output_reco_tracks="CDCRecoTracks",
            use_cdc_quality_estimator=True,
            cdc_quality_estimator_weightfile=self.get_input_file_names(
                CDCQETeacherTask.weightfile_identifier
            )[0],
        )
        tracking.add_svd_track_finding(
            path,
            components=None,
            input_reco_tracks="CDCRecoTracks",
            output_reco_tracks="SVDCDCRecoTracks",
            temporary_reco_tracks="SVDRecoTracks",
            temporary_reco_tracks_two="SVDplusRecoTracks",
            add_both_directions=True,
            use_vxdtf2_quality_estimator=True,
        )
        basf2.set_module_parameters(
            path,
            name="VXDQualityEstimatorMVA",
            WeightFileIdentifier=self.get_input_file_names(
                VXDQETeacherTask.weightfile_identifier
            )[0],
        )
        tracking.add_pxd_track_finding(
            path,
            components=None,
            input_reco_tracks="SVDCDCRecoTracks",
            output_reco_tracks="RecoTracks",
            temporary_reco_tracks="PXDRecoTracks",
            add_both_directions=True,
        )
        tracking.add_time_extraction(path)
        tracking.add_mc_matcher(path)
        tracking.add_track_fit_and_track_creator(path)
        path.add_module(
            "TrackQualityEstimatorMVA",
            WeightFileIdentifier=self.get_input_file_names(
                FullTrackQETeacherTask.weightfile_identifier
            )[0],
            collectEventFeatures=True,
        )


class TrackQEEvaluationBaseTask(Task):
    """
    Base class for evaluating a quality estimator ``basf2_mva_evaluate.py`` on a
    separate test data set.

    Evaluation tasks for VXD, CDC and combined QE can inherit from it.
    """
    git_hash = b2luigi.Parameter(default=get_basf2_git_hash())
    n_events_testing = b2luigi.IntParameter()
    n_events_training = b2luigi.IntParameter()
    training_target = b2luigi.Parameter(default="truth")

    @property
    def teacherTask(self) -> TrackQETeacherBaseTask:
        """
        Property defining specific teacher task to require.
        """
        raise NotImplementedError(
            "Evaluation Tasks must define a teacher task to require "
        )

    @property
    def dataCollectionTask(self) -> Basf2PathTask:
        """
        Property defining the specific ``DataCollectionTask`` to require.  Must
        implemented by the inheriting specific teacher task class.
        """
        raise NotImplementedError(
            "Evaluation Tasks must define a data collection task to require "
        )

    def requires(self):
        yield self.teacherTask(
            n_events_training=self.n_events_training,
            training_target=self.training_target,
        )
        yield self.dataCollectionTask(
            num_processes=MasterTask.num_processes,
            n_events=self.n_events_testing,
            random_seed="testdata_0",
        )

    def output(self):
        evaluation_pdf_output = (
            self.teacherTask.weightfile_identifier.rsplit(".", 1)[0] + ".pdf"
        )
        yield self.add_to_output(evaluation_pdf_output)

    @b2luigi.on_temporary_files
    def run(self):
        evaluation_pdf_output_basename = self.teacherTask.weightfile_identifier.rsplit(".", 1)[0] + ".pdf"
        evaluation_pdf_output_path = self.get_output_file_name(evaluation_pdf_output_basename)

        cmd = [
            "basf2_mva_evaluate.py",
            "--identifiers",
            self.get_input_file_names(self.teacherTask.weightfile_identifier)[0],
            "--datafiles",
            self.get_input_file_names(self.dataCollectionTask.records_file_name)[0],
            "--treename",
            self.teacherTask.tree_name,
            "--outputfile",
            evaluation_pdf_output_path,
            "--fillnan",  # fill NANs with actual values so that plots don't fail
        ]

        # Prepare log files
        log_file_dir = get_log_file_dir(self)
        stderr_log_file_path = log_file_dir + "stderr"
        stdout_log_file_path = log_file_dir + "stdout"
        with open(stdout_log_file_path, "w") as stdout_file:
            stdout_file.write("stdout output of the command:\n{}\n\n".format(" ".join(cmd)))
        if os.path.exists(stderr_log_file_path):
            # remove stderr file if it already exists b/c in the following it will be opened in appending mode
            os.remove(stderr_log_file_path)

        # Run evaluation via subprocess and write output into logfiles
        with open(stdout_log_file_path, "a") as stdout_file:
            with open(stderr_log_file_path, "a") as stderr_file:
                try:
                    subprocess.run(cmd, check=True, stdin=stdout_file, stderr=stderr_file)
                except subprocess.CalledProcessError as err:
                    stderr_file.write(f"Evaluation failed with error:\n{err}")
                    raise err


class VXDTrackQEEvaluationTask(TrackQEEvaluationBaseTask):
    """
    Run ``basf2_mva_evaluate.py`` for the VXD quality estimator on separate test data
    """
    teacherTask = VXDQETeacherTask
    dataCollectionTask = VXDQEDataCollectionTask


class CDCTrackQEEvaluationTask(TrackQEEvaluationBaseTask):
    """
    Run ``basf2_mva_evaluate.py`` for the CDC quality estimator on separate test data
    """
    teacherTask = CDCQETeacherTask
    dataCollectionTask = CDCQEDataCollectionTask


class FullTrackQEEvaluationTask(TrackQEEvaluationBaseTask):
    """
    Run ``basf2_mva_evaluate.py`` for the final, combined quality estimator on
    separate test data
    """
    teacherTask = FullTrackQETeacherTask
    dataCollectionTask = FullTrackQEDataCollectionTask
    exclude_variables = b2luigi.ListParameter(hashed=True)
    cdc_training_target = b2luigi.Parameter()

    def requires(self):
        yield self.teacherTask(
            exclude_variables=self.exclude_variables,
            n_events_training=self.n_events_training,
            training_target=self.training_target,
            cdc_training_target=self.cdc_training_target,
        )
        yield self.dataCollectionTask(
            num_processes=MasterTask.num_processes,
            n_events=self.n_events_testing,
            random_seed="testdata_0",
            cdc_training_target=self.cdc_training_target,
        )


class PlotsFromHarvestingValidationBaseTask(Basf2Task):
    """
    Create a PDF file with validation plots for a quality estimator produced
    from the ROOT ntuples produced by a harvesting validation task
    """
    primaries_only = b2luigi.BoolParameter(default=True)  # normalize finding efficiencies to primary MC-tracks

    @property
    def harvesting_validation_task_instance(self) -> HarvestingValidationBaseTask:
        """
        Specifies related harvesting validation task which produces the ROOT
        files with the data that is plotted by this task.
        """
        raise NotImplementedError("Must define a QI harvesting validation task for which to do the plots")

    @property
    def output_pdf_file_basename(self):
        """
        Name of the output PDF file containing the validation plots
        """
        validation_harvest_basename = self.harvesting_validation_task_instance.validation_output_file_name
        return validation_harvest_basename.replace(".root", "_plots.pdf")

    def requires(self):
        yield self.harvesting_validation_task_instance

    def output(self):
        yield self.add_to_output(self.output_pdf_file_basename)

    @b2luigi.on_temporary_files
    def process(self):
        # get the validation "harvest", which is the ROOT file with ntuples for validation
        validation_harvest_basename = self.harvesting_validation_task_instance.validation_output_file_name
        validation_harvest_path = self.get_input_file_names(validation_harvest_basename)[0]

        # Load "harvested" validation data from root files into dataframes (requires enough memory to hold data)
        pr_columns = [  # Restrict memory usage by only reading in columns that are used in the steering file
            'is_fake', 'is_clone', 'is_matched', 'quality_indicator',
            'experiment_number', 'run_number', 'event_number', 'pr_store_array_number',
            'pt_estimate', 'z0_estimate', 'd0_estimate', 'tan_lambda_estimate',
            'phi0_estimate', 'pt_truth', 'z0_truth', 'd0_truth', 'tan_lambda_truth',
            'phi0_truth',
        ]
        # In ``pr_df`` each row corresponds to a track from Pattern Recognition
        pr_df = root_pandas.read_root(validation_harvest_path, key='pr_tree/pr_tree', columns=pr_columns)
        mc_columns = [  # restrict mc_df to these columns
            'experiment_number',
            'run_number',
            'event_number',
            'pr_store_array_number',
            'is_missing',
            'is_primary',
        ]
        # In ``mc_df`` each row corresponds to an MC track
        mc_df = root_pandas.read_root(validation_harvest_path, key='mc_tree/mc_tree', columns=mc_columns)
        if self.primaries_only:
            mc_df = mc_df[mc_df.is_primary.eq(True)]

        # Define QI thresholds for the FOM plots and the ROC curves
        qi_cuts = np.linspace(0., 1, 20, endpoint=False)
        # # Add more points at the very end between the previous maximum and 1
        # qi_cuts = np.append(qi_cuts, np.linspace(np.max(qi_cuts), 1, 20, endpoint=False))

        # Create plots and append them to single output pdf

        output_pdf_file_path = self.get_output_file_name(self.output_pdf_file_basename)
        with PdfPages(output_pdf_file_path, keep_empty=False) as pdf:

            # Add a title page to validation plot PDF with some metadata
            # Remember that most metadata is in the xml file of the weightfile
            # and in the b2luigi directory structure
            titlepage_fig, titlepage_ax = plt.subplots()
            titlepage_ax.axis("off")
            title = f"Quality Estimator validation plots from {self.__class__.__name__}"
            titlepage_ax.set_title(title)
            meta_data = {
                "Date": datetime.today().strftime("%Y-%m-%d %H:%M"),
                "Created by steering file": os.path.realpath(__file__),
                "Created from data in": validation_harvest_path,
                "Background directory": MasterTask.bkgfiles_dir,
                "weight file": self.harvesting_validation_task_instance.teacherTask.weightfile_identifier
            }
            if hasattr(self, 'exclude_variables'):
                meta_data["Excluded variables"] = ", ".join(self.exclude_variables)
            meta_data_string = (format_dictionary(meta_data) +
                                "\n\n(For all MVA training parameters look into the produced weight file)")
            luigi_params = get_serialized_parameters(self)
            luigi_param_string = (f"\n\nb2luigi parameters for {self.__class__.__name__}\n" +
                                  format_dictionary(luigi_params))
            title_page_text = meta_data_string + luigi_param_string
            titlepage_ax.text(0, 1, title_page_text, ha="left", va="top", wrap=True, fontsize=8)
            pdf.savefig(titlepage_fig)
            plt.close(titlepage_fig)

            fake_rates = get_uncertain_means_for_qi_cuts(pr_df, "is_fake", qi_cuts)
            fake_fig, fake_ax = plt.subplots()
            fake_ax.set_title("Fake rate")
            plot_with_errobands(fake_rates, ax=fake_ax)
            fake_ax.set_ylabel("fake rate")
            fake_ax.set_xlabel("quality indicator requirement")
            pdf.savefig(fake_fig, bbox_inches="tight")
            plt.close(fake_fig)

            # Plot clone rates
            clone_rates = get_uncertain_means_for_qi_cuts(pr_df, "is_clone", qi_cuts)
            clone_fig, clone_ax = plt.subplots()
            clone_ax.set_title("Clone rate")
            plot_with_errobands(clone_rates, ax=clone_ax)
            clone_ax.set_ylabel("clone rate")
            clone_ax.set_xlabel("quality indicator requirement")
            pdf.savefig(clone_fig, bbox_inches="tight")
            plt.close(clone_fig)

            # Plot finding efficieny

            # The Quality Indicator is only avaiable in pr_tree and thus the
            # PR-track dataframe. To get the QI of the related PR track for an MC
            # track, merge the PR dataframe into the MC dataframe
            pr_track_identifiers = ['experiment_number', 'run_number', 'event_number', 'pr_store_array_number']
            mc_df = upd.merge(
                left=mc_df, right=pr_df[pr_track_identifiers + ['quality_indicator']],
                how='left',
                on=pr_track_identifiers
            )

            missing_fractions = (
                _my_uncertain_mean(mc_df[
                    mc_df.quality_indicator.isnull() | (mc_df.quality_indicator > qi_cut)]['is_missing'])
                for qi_cut in qi_cuts
            )

            findeff_fig, findeff_ax = plt.subplots()
            findeff_ax.set_title("Finding efficiency")
            finding_efficiencies = 1.0 - upd.Series(data=missing_fractions, index=qi_cuts)
            plot_with_errobands(finding_efficiencies, ax=findeff_ax)
            findeff_ax.set_ylabel("finding efficiency")
            findeff_ax.set_xlabel("quality indicator requirement")
            pdf.savefig(findeff_fig, bbox_inches="tight")
            plt.close(findeff_fig)

            # Plot ROC curves

            # Fake rate vs. finding efficiency ROC curve
            fake_roc_fig, fake_roc_ax = plt.subplots()
            fake_roc_ax.set_title("Fake rate vs. finding efficiency ROC curve")
            fake_roc_ax.errorbar(x=finding_efficiencies.nominal_value, y=fake_rates.nominal_value,
                                 xerr=finding_efficiencies.std_dev, yerr=fake_rates.std_dev, elinewidth=0.8)
            fake_roc_ax.set_xlabel('finding efficiency')
            fake_roc_ax.set_ylabel('fake rate')
            pdf.savefig(fake_roc_fig, bbox_inches="tight")
            plt.close(fake_roc_fig)

            # Clone rate vs. finding efficiency ROC curve
            clone_roc_fig, clone_roc_ax = plt.subplots()
            clone_roc_ax.set_title("Clone rate vs. finding efficiency ROC curve")
            clone_roc_ax.errorbar(x=finding_efficiencies.nominal_value, y=clone_rates.nominal_value,
                                  xerr=finding_efficiencies.std_dev, yerr=clone_rates.std_dev, elinewidth=0.8)
            clone_roc_ax.set_xlabel('finding efficiency')
            clone_roc_ax.set_ylabel('clone rate')
            pdf.savefig(clone_roc_fig, bbox_inches="tight")
            plt.close(clone_roc_fig)

            # Plot kinematic distributions

            # use fewer qi cuts as each cut will be it's own subplot now and not a point
            kinematic_qi_cuts = [0, 0.5, 0.9]

            # Define kinematic parameters which we want to histogram and define
            # dictionaries relating them to latex labels, units and binnings
            params = ['d0', 'z0', 'pt', 'tan_lambda', 'phi0']
            label_by_param = {
                "pt": "$p_T$",
                "z0": "$z_0$",
                "d0": "$d_0$",
                "tan_lambda": r"$\tan{\lambda}$",
                "phi0": r"$\phi_0$"
            }
            unit_by_param = {
                "pt": "GeV",
                "z0": "cm",
                "d0": "cm",
                "tan_lambda": "rad",
                "phi0": "rad"
            }
            n_kinematic_bins = 75  # number of bins per kinematic variable
            bins_by_param = {
                "pt": np.linspace(0, np.percentile(pr_df['pt_truth'].dropna(), 95), n_kinematic_bins),
                "z0": np.linspace(-0.1, 0.1, n_kinematic_bins),
                "d0": np.linspace(0, 0.01, n_kinematic_bins),
                "tan_lambda": np.linspace(-2, 3, n_kinematic_bins),
                "phi0": np.linspace(0, 2 * np.pi, n_kinematic_bins)
            }

            # Iterate over each parameter and for each make stacked histograms for different QI cuts
            kinematic_qi_cuts = [0, 0.5, 0.8]
            blue, yellow, green = plt.get_cmap("tab10").colors[0:3]
            for param in params:
                fig, axarr = plt.subplots(ncols=len(kinematic_qi_cuts), sharey=True, sharex=True, figsize=(14, 6))
                fig.suptitle(f"{label_by_param[param]}  distributions")
                for i, qi in enumerate(kinematic_qi_cuts):
                    ax = axarr[i]
                    ax.set_title(f"QI > {qi}")
                    incut = pr_df[(pr_df['quality_indicator'] > qi)]
                    incut_matched = incut[incut.is_matched.eq(True)]
                    incut_clones = incut[incut.is_clone.eq(True)]
                    incut_fake = incut[incut.is_fake.eq(True)]

                    # if any series is empty, break ouf loop and don't draw try to draw a stacked histogram
                    if any(series.empty for series in (incut, incut_matched, incut_clones, incut_fake)):
                        ax.text(0.5, 0.5, "Not enough data in bin", ha="center", va="center", transform=ax.transAxes)
                        continue

                    bins = bins_by_param[param]
                    stacked_histogram_series_tuple = (
                        incut_matched[f'{param}_estimate'],
                        incut_clones[f'{param}_estimate'],
                        incut_fake[f'{param}_estimate'],
                    )
                    histvals, _, _ = ax.hist(stacked_histogram_series_tuple,
                                             stacked=True,
                                             bins=bins, range=(bins.min(), bins.max()),
                                             color=(blue, green, yellow),
                                             label=("matched", "clones", "fakes"))
                    ax.set_xlabel(f'{label_by_param[param]} estimate / ({unit_by_param[param]})')
                    ax.set_ylabel('# tracks')
                axarr[0].legend(loc="upper center", bbox_to_anchor=(0, -0.15))
                pdf.savefig(fig, bbox_inches="tight")
                plt.close(fig)


class VXDQEValidationPlotsTask(PlotsFromHarvestingValidationBaseTask):
    """
    Create a PDF file with validation plots for the VXDTF2 track quality
    estimator produced from the ROOT ntuples produced by a VXDTF2 track QE
    harvesting validation task
    """
    n_events_testing = b2luigi.IntParameter()
    n_events_training = b2luigi.IntParameter()

    @property
    def harvesting_validation_task_instance(self):
        return VXDQEHarvestingValidationTask(
            n_events_testing=self.n_events_testing,
            n_events_training=self.n_events_training,
            num_processes=MasterTask.num_processes,
        )


class CDCQEValidationPlotsTask(PlotsFromHarvestingValidationBaseTask):
    """
    Create a PDF file with validation plots for the CDC track quality estimator
    produced from the ROOT ntuples produced by a CDC track QE harvesting
    validation task
    """
    n_events_testing = b2luigi.IntParameter()
    n_events_training = b2luigi.IntParameter()
    training_target = b2luigi.Parameter()

    @property
    def harvesting_validation_task_instance(self):
        return CDCQEHarvestingValidationTask(
            n_events_testing=self.n_events_testing,
            n_events_training=self.n_events_training,
            training_target=self.training_target,
            num_processes=MasterTask.num_processes,
        )


class FullTrackQEValidationPlotsTask(PlotsFromHarvestingValidationBaseTask):
    """
    Create a PDF file with validation plots for the full MVA track quality
    estimator produced from the ROOT ntuples produced by a full track QE
    harvesting validation task
    """
    n_events_testing = b2luigi.IntParameter()
    n_events_training = b2luigi.IntParameter()
    cdc_training_target = b2luigi.Parameter()
    exclude_variables = b2luigi.ListParameter(hashed=True)

    @property
    def harvesting_validation_task_instance(self):
        return FullTrackQEHarvestingValidationTask(
            n_events_testing=self.n_events_testing,
            n_events_training=self.n_events_training,
            cdc_training_target=self.cdc_training_target,
            exclude_variables=self.exclude_variables,
            num_processes=MasterTask.num_processes,
        )


class MasterTask(b2luigi.WrapperTask):
    """
    Entry point: Task that defines the configurations that shall be tested.
    """

    n_events_training = b2luigi.get_setting("n_events_training", default=3000)
    n_events_testing = b2luigi.get_setting("n_events_testing", default=1000)
    num_processes = b2luigi.get_setting("basf2_processes_per_worker", default=0)
    # directory with MC overlay background root files
    bkgfiles_dir = b2luigi.get_setting("bkgfiles_directory")
    # Choose whether to run basf2_mva_evaluate tasks on weightfiles. These will
    # fail if no LaTeX is installed, but we still have our own independent
    # validation tasks.
    run_mva_evaluate = b2luigi.get_setting("run_mva_evaluate", default=True)

    def requires(self):

        # eventwise n_track_variables should not be used by teacher tasks in training
        ntrack_variables = [
            "N_RecoTracks",
            "N_PXDRecoTracks",
            "N_SVDRecoTracks",
            "N_CDCRecoTracks",
            "N_diff_PXD_SVD_RecoTracks",
            "N_diff_SVD_CDC_RecoTracks",
        ]

        for exclude_variables in [ntrack_variables]:
            for cdc_training_target in [
                "truth_track_is_matched",
                "truth"  # truth includes clones as signal
            ]:
                yield FullTrackQEValidationPlotsTask(
                    cdc_training_target=cdc_training_target,
                    exclude_variables=exclude_variables,
                    n_events_training=self.n_events_training,
                    n_events_testing=self.n_events_testing,
                )

                yield CDCQEValidationPlotsTask(
                    training_target=cdc_training_target,
                    n_events_training=self.n_events_training,
                    n_events_testing=self.n_events_testing,
                )

                yield VXDQEValidationPlotsTask(
                    n_events_training=self.n_events_training,
                    n_events_testing=self.n_events_testing,
                )

                if self.run_mva_evaluate:
                    yield FullTrackQEEvaluationTask(
                        exclude_variables=exclude_variables,
                        cdc_training_target=cdc_training_target,
                        n_events_training=self.n_events_training,
                        n_events_testing=self.n_events_testing,
                    )
                    yield CDCTrackQEEvaluationTask(
                        training_target=cdc_training_target,
                        n_events_training=self.n_events_training,
                        n_events_testing=self.n_events_testing,
                    )
                    yield VXDTrackQEEvaluationTask(
                        n_events_training=self.n_events_training,
                        n_events_testing=self.n_events_testing,
                    )


if __name__ == "__main__":
    workers = b2luigi.get_setting("workers", default=1)
    b2luigi.process(MasterTask(), workers=workers)
