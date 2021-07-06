#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
combined_module_quality_estimator_teacher
-----------------------------------------

Information on the MVA Track Quality Indicator / Estimator can be found
on `Confluence
<https://confluence.desy.de/display/BI/MVA+Track+Quality+Indicator>`_.

Purpose of this script
~~~~~~~~~~~~~~~~~~~~~~

This python script is used for the combined training and validation of three
classifiers, the actual final MVA track quality estimator and the two quality
estimators for the intermediate standalone track finders that it depends on.

    - Final MVA track quality estimator:
      The final quality estimator for fully merged and fitted tracks (RecoTracks).
      Its classifier uses features from the track fitting, merger, hit pattern, ...
      But it also uses the outputs from respective intermediate quality
      estimators for the VXD and the CDC track finding as inputs. It provides
      the final quality indicator (QI) exported to the track objects.

    - VXDTF2 track quality estimator:
      MVA quality estimator for the VXD standalone track finding.

    - CDC track quality estimator:
      MVA quality estimator for the CDC standalone track finding.

Each classifier requires for its training a different training data set and they
need to be validated on a separate testing data set. Further, the final quality
estimator can only be trained, when the trained weights for the intermediate
quality estimators are available. If the final estimator shall be trained without
one or both previous estimators, the requirements have to be commented out in the
__init__.py file of tracking.
For all estimators, a list of variables to be ignored is specified in the MasterTask.
The current choice is mainly based on pure data MC agreement in these quantities or
on outdated implementations. It was decided to leave them in the hardcoded "ugly" way
in here to remind future generations that they exist in principle and they should and
could be added to the estimator, once their modelling becomes better in future or an
alternative implementation is programmed.
To avoid mistakes, b2luigi is used to create a task chain for a combined training and
validation of all classifiers.

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
fit to your requirements.

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

import itertools
import os
from pathlib import Path
import shutil
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
import background
import simulation
import tracking
import tracking.root_utils as root_utils
from tracking.harvesting_validation.combined_module import CombinedTrackingValidationModule

# wrap python modules that are used here but not in the externals into a try except block
install_helpstring_formatter = ("\nCould not find {module} python module.Try installing it via\n"
                                "  python3 -m pip install [--user] {module}\n")
try:
    import b2luigi
    from b2luigi.core.utils import get_serialized_parameters, get_log_file_dir, create_output_dirs
    from b2luigi.basf2_helper import Basf2PathTask, Basf2Task
    from b2luigi.core.task import Task, ExternalTask
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


def create_fbdt_option_string(fast_bdt_option):
    """
    returns a readable string created by the fast_bdt_option array
    """
    return "_nTrees" + str(fast_bdt_option[0]) + "_nCuts" + str(fast_bdt_option[1]) + "_nLevels" + \
        str(fast_bdt_option[2]) + "_shrin" + str(int(round(100*fast_bdt_option[3], 0)))


def createV0momenta(x, mu, beta):
    """
    Copied from Biancas K_S0 particle gun code: Returns a realistiv V0 momentum distribution
    when running over x. Mu and Beta are properties of the function that define center and tails.
    Used for the particle gun simulation code for K_S0 and Lambda_0
    """
    return (1/beta)*np.exp(-(x - mu)/beta) * np.exp(-np.exp(-(x - mu) / beta))


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
    :param fast_bdt_option: specified fast BDT options, defaut: [200, 8, 3, 0.1] [nTrees, nCuts, nLevels, shrinkage]
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

    # Set options for MVA trainihng
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

    #: Number of events to generate.
    n_events = b2luigi.IntParameter()
    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
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
        if self.experiment_number in [0, 1002, 1003]:
            runNo = 0
        else:
            runNo = 0
            raise ValueError(
                f"Simulating events with experiment_number {self.experiment_number} is not implemented yet.")
        path.add_module(
            "EventInfoSetter", evtNumList=[self.n_events], runList=[runNo], expList=[self.experiment_number]
        )
        if "BBBAR" in self.random_seed:
            path.add_module("EvtGenInput")
        elif "V0BBBAR" in self.random_seed:
            path.add_module("EvtGenInput")
            path.add_module("InclusiveParticleChecker", particles=[310, 3122], includeConjugates=True)
        else:
            import generators as ge
            # WARNING: There are a few differences in the production of MC13a and b like the following lines
            # as well as ActivatePXD.. and the beamparams for bhabha... I use these from MC13b, not a... :/
            # import beamparameters as bp
            # beamparameters = bp.add_beamparameters(path, "Y4S")
            # beamparameters.param("covVertex", [(14.8e-4)**2, (1.5e-4)**2, (360e-4)**2])
            if "V0STUDY" in self.random_seed:
                if "V0STUDYKS" in self.random_seed:
                    # Bianca looked at the Ks dists and extracted these values:
                    mu = 0.5
                    beta = 0.2
                    pdgs = [310]  # Ks (has no antiparticle, Klong is different)
                if "V0STUDYL0" in self.random_seed:
                    # I just made the lamdba values up, such that they peak at 0.35 and are slightly shifted to lower values
                    mu = 0.35
                    beta = 0.15  # if this is chosen higher, one needs to make sure not to get values >0 for 0
                    pdgs = [3122, -3122]  # Lambda0
                else:
                    # also these values are made up
                    mu = 0.43
                    beta = 0.18
                    pdgs = [310, 3122, -3122]  # Ks and Lambda0
                # create realistic momentum distribution
                myx = [i*0.01 for i in range(321)]
                myy = []
                for x in myx:
                    y = createV0momenta(x, mu, beta)
                    myy.append(y)
                polParams = myx + myy
                # define particles that are produced
                pdg_list = pdgs

                particlegun = basf2.register_module('ParticleGun')
                particlegun.param('pdgCodes', pdg_list)
                particlegun.param('nTracks', 8)  # number of particles (not tracks!) that is created in each event
                particlegun.param('momentumGeneration', 'polyline')
                particlegun.param('momentumParams', polParams)
                particlegun.param('thetaGeneration', 'uniformCos')
                particlegun.param('thetaParams', [17, 150])  # [0, 180]) #[17, 150]
                particlegun.param('phiGeneration', 'uniform')
                particlegun.param('phiParams', [0, 360])
                particlegun.param('vertexGeneration', 'fixed')
                particlegun.param('xVertexParams', [0])
                particlegun.param('yVertexParams', [0])
                particlegun.param('zVertexParams', [0])
                path.add_module(particlegun)
            if "BHABHA" in self.random_seed:
                ge.add_babayaganlo_generator(path=path, finalstate='ee', minenergy=0.15, minangle=10.0)
            elif "MUMU" in self.random_seed:
                ge.add_kkmc_generator(path=path, finalstate='mu+mu-')
            elif "YY" in self.random_seed:
                babayaganlo = basf2.register_module('BabayagaNLOInput')
                babayaganlo.param('FinalState', 'gg')
                babayaganlo.param('MaxAcollinearity', 180.0)
                babayaganlo.param('ScatteringAngleRange', [0., 180.])
                babayaganlo.param('FMax', 75000)
                babayaganlo.param('MinEnergy', 0.01)
                babayaganlo.param('Order', 'exp')
                babayaganlo.param('DebugEnergySpread', 0.01)
                babayaganlo.param('Epsilon', 0.00005)
                path.add_module(babayaganlo)
                generatorpreselection = basf2.register_module('GeneratorPreselection')
                generatorpreselection.param('nChargedMin', 0)
                generatorpreselection.param('nChargedMax', 999)
                generatorpreselection.param('MinChargedPt', 0.15)
                generatorpreselection.param('MinChargedTheta', 17.)
                generatorpreselection.param('MaxChargedTheta', 150.)
                generatorpreselection.param('nPhotonMin', 1)
                generatorpreselection.param('MinPhotonEnergy', 1.5)
                generatorpreselection.param('MinPhotonTheta', 15.0)
                generatorpreselection.param('MaxPhotonTheta', 165.0)
                generatorpreselection.param('applyInCMS', True)
                path.add_module(generatorpreselection)
                empty = basf2.create_path()
                generatorpreselection.if_value('!=11', empty)
            elif "EEEE" in self.random_seed:
                ge.add_aafh_generator(path=path, finalstate='e+e-e+e-', preselection=False)
            elif "EEMUMU" in self.random_seed:
                ge.add_aafh_generator(path=path, finalstate='e+e-mu+mu-', preselection=False)
            elif "TAUPAIR" in self.random_seed:
                ge.add_kkmc_generator(path, finalstate='tau+tau-')
            elif "DDBAR" in self.random_seed:
                ge.add_continuum_generator(path, finalstate='ddbar')
            elif "UUBAR" in self.random_seed:
                ge.add_continuum_generator(path, finalstate='uubar')
            elif "SSBAR" in self.random_seed:
                ge.add_continuum_generator(path, finalstate='ssbar')
            elif "CCBAR" in self.random_seed:
                ge.add_continuum_generator(path, finalstate='ccbar')
        # activate simulation of dead/masked pixel and reproduce detector gain, which will be
        # applied at reconstruction level when the data GT is present in the DB chain
        # path.add_module("ActivatePXDPixelMasker")
        # path.add_module("ActivatePXDGainCalibrator")
        bkg_files = background.get_background_files(self.bkgfiles_dir)
        if self.experiment_number == 1002:
            # remove KLM because of bug in backround files with release 4
            components = ['PXD', 'SVD', 'CDC', 'ECL', 'TOP', 'ARICH', 'TRG']
        else:
            components = None
        simulation.add_simulation(path, bkgfiles=bkg_files, bkgOverlay=True, components=components)  # , usePXDDataReduction=False)

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

    #: Number of events to generate.
    n_events = b2luigi.IntParameter()
    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
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
        Generate list of luigi Tasks that this Task depends on.
        """
        n_events_per_task = MasterTask.n_events_per_task
        quotient, remainder = divmod(self.n_events, n_events_per_task)
        for i in range(quotient):
            yield GenerateSimTask(
                bkgfiles_dir=self.bkgfiles_dir,
                num_processes=MasterTask.num_processes,
                random_seed=self.random_seed + '_' + str(i).zfill(3),
                n_events=n_events_per_task,
                experiment_number=self.experiment_number,
                )
        if remainder > 0:
            yield GenerateSimTask(
                bkgfiles_dir=self.bkgfiles_dir,
                num_processes=MasterTask.num_processes,
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


class CheckExistingFile(ExternalTask):
    """
    Task to check if the given file really exists.
    """
    #: filename to check
    filename = b2luigi.Parameter()

    def output(self):
        """
        Specify the output to be the file that was just checked.
        """
        from luigi import LocalTarget
        return LocalTarget(self.filename)


class VXDQEDataCollectionTask(Basf2PathTask):
    """
    Collect variables/features from VXDTF2 tracking and write them to a ROOT
    file.

    These variables are to be used as labelled training data for the MVA
    classifier which is the VXD track quality estimator
    """
    #: Number of events to generate.
    n_events = b2luigi.IntParameter()
    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
    #: Random basf2 seed used by the GenerateSimTask. It is further used to read of the
    # production process to preserve clearness in the b2luigi output.
    random_seed = b2luigi.Parameter()
    #: specify queue. E.g. choose between 'l' (long), 's' (short) or 'sx' (short, extra ram)
    queue = 'l'

    #: Filename of the recorded/collected data for the final QE MVA training.
    def get_records_file_name(self, n_events=None, random_seed=None):
        """
        Create output file name depending on number of events and production
        mode that is specified in the random_seed string.
        """
        if n_events is None:
            n_events = self.n_events
        if random_seed is None:
            random_seed = self.random_seed
        if 'vxd' not in random_seed:
            random_seed += '_vxd'
        if 'DATA' in random_seed:
            return 'qe_records_DATA_vxd.root'
        else:
            if 'USESIMBB' in random_seed:
                random_seed = 'BBBAR_' + random_seed.split("_", 1)[1]
            elif 'USESIMEE' in random_seed:
                random_seed = 'BHABHA_' + random_seed.split("_", 1)[1]
            return 'qe_records_N' + str(n_events) + '_' + random_seed + '.root'

    def get_input_files(self, n_events=None, random_seed=None):
        """
        Get input file names depending on the use case: If they already exist, search in
        the corresponding folders, for data check the specified list and if they are created
        in the same run, check for the task that produced them.
        """
        if n_events is None:
            n_events = self.n_events
        if random_seed is None:
            random_seed = self.random_seed
        if "USESIM" in random_seed:
            if 'USESIMBB' in random_seed:
                random_seed = 'BBBAR_' + random_seed.split("_", 1)[1]
            elif 'USESIMEE' in random_seed:
                random_seed = 'BHABHA_' + random_seed.split("_", 1)[1]
            return ['datafiles/' + GenerateSimTask.output_file_name(GenerateSimTask,
                                                                    n_events=n_events, random_seed=random_seed)]
        elif "DATA" in random_seed:
            return MasterTask.datafiles
        else:
            return self.get_input_file_names(GenerateSimTask.output_file_name(
                GenerateSimTask, n_events=n_events, random_seed=random_seed))

    def requires(self):
        """
        Generate list of luigi Tasks that this Task depends on.
        """
        if "USESIM" in self.random_seed or "DATA" in self.random_seed:
            for filename in self.get_input_files():
                yield CheckExistingFile(
                    filename=filename,
                    )
        else:
            yield SplitNMergeSimTask(
                bkgfiles_dir=MasterTask.bkgfiles_by_exp[self.experiment_number],
                random_seed=self.random_seed,
                n_events=self.n_events,
                experiment_number=self.experiment_number,
                )

    def output(self):
        """
        Generate list of output files that the task should produce.
        The task is considered finished if and only if the outputs all exist.
        """
        yield self.add_to_output(self.get_records_file_name())

    def create_path(self):
        """
        Create basf2 path with VXDTF2 tracking and VXD QE data collection.
        """
        path = basf2.create_path()
        inputFileNames = self.get_input_files()
        path.add_module(
            "RootInput",
            inputFileNames=inputFileNames,
        )
        path.add_module("Gearbox")
        tracking.add_geometry_modules(path)
        if 'DATA' in self.random_seed:
            from rawdata import add_unpackers
            add_unpackers(path, components=['SVD', 'PXD'])
        tracking.add_hit_preparation_modules(path)
        tracking.add_vxd_track_finding_vxdtf2(
            path, components=["SVD"], add_mva_quality_indicator=False
        )
        if 'DATA' in self.random_seed:
            path.add_module(
                "VXDQETrainingDataCollector",
                TrainingDataOutputName=self.get_output_file_name(self.get_records_file_name()),
                SpacePointTrackCandsStoreArrayName="SPTrackCands",
                EstimationMethod="tripletFit",
                UseTimingInfo=False,
                ClusterInformation="Average",
                MCStrictQualityEstimator=False,
                mva_target=False,
                MCInfo=False,
                )
        else:
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
                TrainingDataOutputName=self.get_output_file_name(self.get_records_file_name()),
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
    #: Number of events to generate.
    n_events = b2luigi.IntParameter()
    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
    #: Random basf2 seed used by the GenerateSimTask. It is further used to read of the
    # production process to preserve clearness in the b2luigi output.
    random_seed = b2luigi.Parameter()
    #: specify queue. E.g. choose between 'l' (long), 's' (short) or 'sx' (short, extra ram)
    queue = 'l'

    #: Filename of the recorded/collected data for the final QE MVA training.
    def get_records_file_name(self,  n_events=None, random_seed=None):
        """
        Create output file name depending on number of events and production
        mode that is specified in the random_seed string.
        """
        if n_events is None:
            n_events = self.n_events
        if random_seed is None:
            random_seed = self.random_seed
        if 'cdc' not in random_seed:
            random_seed += '_cdc'
        if 'DATA' in random_seed:
            return 'qe_records_DATA_cdc.root'
        else:
            if 'USESIMBB' in random_seed:
                random_seed = 'BBBAR_' + random_seed.split("_", 1)[1]
            elif 'USESIMEE' in random_seed:
                random_seed = 'BHABHA_' + random_seed.split("_", 1)[1]
            return 'qe_records_N' + str(n_events) + '_' + random_seed + '.root'

    def get_input_files(self, n_events=None, random_seed=None):
        """
        Get input file names depending on the use case: If they already exist, search in
        the corresponding folders, for data check the specified list and if they are created
        in the same run, check for the task that produced them.
        """
        if n_events is None:
            n_events = self.n_events
        if random_seed is None:
            random_seed = self.random_seed
        if "USESIM" in random_seed:
            if 'USESIMBB' in random_seed:
                random_seed = 'BBBAR_' + random_seed.split("_", 1)[1]
            elif 'USESIMEE' in random_seed:
                random_seed = 'BHABHA_' + random_seed.split("_", 1)[1]
            return ['datafiles/' + GenerateSimTask.output_file_name(GenerateSimTask,
                                                                    n_events=n_events, random_seed=random_seed)]
        elif "DATA" in random_seed:
            return MasterTask.datafiles
        else:
            return self.get_input_file_names(GenerateSimTask.output_file_name(
                GenerateSimTask, n_events=n_events, random_seed=random_seed))

    def requires(self):
        """
        Generate list of luigi Tasks that this Task depends on.
        """
        if "USESIM" in self.random_seed or "DATA" in self.random_seed:
            for filename in self.get_input_files():
                yield CheckExistingFile(
                    filename=filename,
                    )
        else:
            yield SplitNMergeSimTask(
                bkgfiles_dir=MasterTask.bkgfiles_by_exp[self.experiment_number],
                random_seed=self.random_seed,
                n_events=self.n_events,
                experiment_number=self.experiment_number,
                )

    def output(self):
        """
        Generate list of output files that the task should produce.
        The task is considered finished if and only if the outputs all exist.
        """
        yield self.add_to_output(self.get_records_file_name())

    def create_path(self):
        """
        Create basf2 path with CDC standalone tracking and CDC QE with recording filter for MVA feature collection.
        """
        path = basf2.create_path()
        inputFileNames = self.get_input_files()
        path.add_module(
            "RootInput",
            inputFileNames=inputFileNames,
        )
        path.add_module("Gearbox")
        tracking.add_geometry_modules(path)
        if 'DATA' in self.random_seed:
            filter_choice = "recording_data"
            from rawdata import add_unpackers
            add_unpackers(path, components=['CDC'])
        else:
            filter_choice = "recording"
            # tracking.add_hit_preparation_modules(path)  # only needed for SVD and
            # PXD hit preparation. Does not change the CDC output.
        tracking.add_cdc_track_finding(path, with_ca=False, add_mva_quality_indicator=True)

        basf2.set_module_parameters(
            path,
            name="TFCDC_TrackQualityEstimator",
            filter=filter_choice,
            filterParameters={
                "rootFileName": self.get_output_file_name(self.get_records_file_name())
            },
        )
        return path


class RecoTrackQEDataCollectionTask(Basf2PathTask):
    """
    Collect variables/features from the reco track reconstruction including the
    fit and write them to a ROOT file.

    These variables are to be used as labelled training data for the MVA
    classifier which is the MVA track quality estimator.  The collected
    variables include the classifier outputs from the VXD and CDC quality
    estimators, namely the CDC and VXD quality indicators, combined with fit,
    merger, timing, energy loss information etc.  This task requires the
    subdetector quality estimators to be trained.
    """

    #: Number of events to generate.
    n_events = b2luigi.IntParameter()
    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
    #: Random basf2 seed used by the GenerateSimTask. It is further used to read of the
    # production process to preserve clearness in the b2luigi output.
    random_seed = b2luigi.Parameter()
    #: Feature/variable to use as truth label for the CDC track quality estimator.
    cdc_training_target = b2luigi.Parameter()
    #: RecoTrack option, use string that is additive: deleteCDCQI0XY (= deletes CDCTracks with
    # CDC-QI below 0.XY), useCDC (= uses trained CDC stored in datafiles/), useVXD (uses trained
    # VXD stored in datafiles/), noVXD (= doesn't use the VXD MVA at all)
    recotrack_option = b2luigi.Parameter(
        #: \cond
        default='deleteCDCQI080'
        #: \endcond
    )
    #: Hyperparameter option of the FastBDT algorithm. default are the FastBDT default values.
    fast_bdt_option = b2luigi.ListParameter(
        #: \cond
        hashed=True, default=[200, 8, 3, 0.1]
        #: \endcond
    )
    #: specify queue. E.g. choose between 'l' (long), 's' (short) or 'sx' (short, extra ram)
    queue = 'l'

    #: Filename of the recorded/collected data for the final QE MVA training.
    def get_records_file_name(self, n_events=None, random_seed=None, recotrack_option=None):
        """
        Create output file name depending on number of events and production
        mode that is specified in the random_seed string.
        """
        if n_events is None:
            n_events = self.n_events
        if random_seed is None:
            random_seed = self.random_seed
        if recotrack_option is None:
            recotrack_option = self.recotrack_option
        if 'rec' not in random_seed:
            random_seed += '_rec'
        if 'DATA' in random_seed:
            return 'qe_records_DATA_rec.root'
        else:
            if 'USESIMBB' in random_seed:
                random_seed = 'BBBAR_' + random_seed.split("_", 1)[1]
            elif 'USESIMEE' in random_seed:
                random_seed = 'BHABHA_' + random_seed.split("_", 1)[1]
            return 'qe_records_N' + str(n_events) + '_' + random_seed + '_' + recotrack_option + '.root'

    def get_input_files(self, n_events=None, random_seed=None):
        """
        Get input file names depending on the use case: If they already exist, search in
        the corresponding folders, for data check the specified list and if they are created
        in the same run, check for the task that produced them.
        """
        if n_events is None:
            n_events = self.n_events
        if random_seed is None:
            random_seed = self.random_seed
        if "USESIM" in random_seed:
            if 'USESIMBB' in random_seed:
                random_seed = 'BBBAR_' + random_seed.split("_", 1)[1]
            elif 'USESIMEE' in random_seed:
                random_seed = 'BHABHA_' + random_seed.split("_", 1)[1]
            return ['datafiles/' + GenerateSimTask.output_file_name(GenerateSimTask,
                                                                    n_events=n_events, random_seed=random_seed)]
        elif "DATA" in random_seed:
            return MasterTask.datafiles
        else:
            return self.get_input_file_names(GenerateSimTask.output_file_name(
                GenerateSimTask, n_events=n_events, random_seed=random_seed))

    def requires(self):
        """
        Generate list of luigi Tasks that this Task depends on.
        """
        if "USESIM" in self.random_seed or "DATA" in self.random_seed:
            for filename in self.get_input_files():
                yield CheckExistingFile(
                    filename=filename,
                    )
        else:
            yield SplitNMergeSimTask(
                bkgfiles_dir=MasterTask.bkgfiles_by_exp[self.experiment_number],
                random_seed=self.random_seed,
                n_events=self.n_events,
                experiment_number=self.experiment_number,
                )
        if "DATA" not in self.random_seed:
            if 'useCDC' not in self.recotrack_option and 'noCDC' not in self.recotrack_option:
                yield CDCQETeacherTask(
                    n_events_training=MasterTask.n_events_training,
                    experiment_number=self.experiment_number,
                    training_target=self.cdc_training_target,
                    process_type=self.random_seed.split("_", 1)[0],
                    exclude_variables=MasterTask.exclude_variables_cdc,
                    fast_bdt_option=self.fast_bdt_option,
                    )
            if 'useVXD' not in self.recotrack_option and 'noVXD' not in self.recotrack_option:
                yield VXDQETeacherTask(
                    n_events_training=MasterTask.n_events_training,
                    experiment_number=self.experiment_number,
                    process_type=self.random_seed.split("_", 1)[0],
                    exclude_variables=MasterTask.exclude_variables_vxd,
                    fast_bdt_option=self.fast_bdt_option,
                    )

    def output(self):
        """
        Generate list of output files that the task should produce.
        The task is considered finished if and only if the outputs all exist.
        """
        yield self.add_to_output(self.get_records_file_name())

    def create_path(self):
        """
        Create basf2 reconstruction path that should mirror the default path
        from ``add_tracking_reconstruction()``, but with modules for the VXD QE
        and CDC QE application and for collection of variables for the reco
        track quality estimator.
        """
        path = basf2.create_path()
        inputFileNames = self.get_input_files()
        path.add_module(
            "RootInput",
            inputFileNames=inputFileNames,
        )
        path.add_module("Gearbox")

        # First add tracking reconstruction with default quality estimation modules
        mvaCDC = True
        mvaVXD = True
        if 'noCDC' in self.recotrack_option:
            mvaCDC = False
        if 'noVXD' in self.recotrack_option:
            mvaVXD = False
        if 'DATA' in self.random_seed:
            from rawdata import add_unpackers
            add_unpackers(path)
        tracking.add_tracking_reconstruction(path, add_cdcTrack_QI=mvaCDC, add_vxdTrack_QI=mvaVXD, add_recoTrack_QI=True)

        # if data shall be processed check if newly trained mva files are available. Otherwise use default ones (CDB payloads):
        # if useCDC/VXD is specified, use the identifier lying in datafiles/ Otherwise, replace weightfile identifiers from defaults
        # (CDB payloads) to new weightfiles created by this b2luigi script
        if ('DATA' in self.random_seed or 'useCDC' in self.recotrack_option) and 'noCDC' not in self.recotrack_option:
            cdc_identifier = 'datafiles/' + \
                             CDCQETeacherTask.get_weightfile_xml_identifier(CDCQETeacherTask, fast_bdt_option=self.fast_bdt_option)
            if os.path.exists(cdc_identifier):
                replace_cdc_qi = True
            elif 'useCDC' in self.recotrack_option:
                raise ValueError(f"CDC QI Identifier not found: {cdc_identifier}")
            else:
                replace_cdc_qi = False
        elif 'noCDC' in self.recotrack_option:
            replace_cdc_qi = False
        else:
            cdc_identifier = self.get_input_file_names(
                CDCQETeacherTask.get_weightfile_xml_identifier(
                    CDCQETeacherTask, fast_bdt_option=self.fast_bdt_option))[0]
            replace_cdc_qi = True
        if ('DATA' in self.random_seed or 'useVXD' in self.recotrack_option) and 'noVXD' not in self.recotrack_option:
            vxd_identifier = 'datafiles/' + \
                VXDQETeacherTask.get_weightfile_xml_identifier(VXDQETeacherTask, fast_bdt_option=self.fast_bdt_option)
            if os.path.exists(vxd_identifier):
                replace_vxd_qi = True
            elif 'useVXD' in self.recotrack_option:
                raise ValueError(f"VXD QI Identifier not found: {vxd_identifier}")
            else:
                replace_vxd_qi = False
        elif 'noVXD' in self.recotrack_option:
            replace_vxd_qi = False
        else:
            vxd_identifier = self.get_input_file_names(
                VXDQETeacherTask.get_weightfile_xml_identifier(
                    VXDQETeacherTask, fast_bdt_option=self.fast_bdt_option))[0]
            replace_vxd_qi = True

        cdc_qe_mva_filter_parameters = None
        # if tracks below a certain CDC QI index shall be deleted online, this needs to be specified in the filter parameters.
        # this is also possible in case of the default (CBD) payloads.
        if 'deleteCDCQI' in self.recotrack_option:
            cut_index = self.recotrack_option.find('deleteCDCQI') + len('deleteCDCQI')
            cut = int(self.recotrack_option[cut_index:cut_index+3])/100.
            if replace_cdc_qi:
                cdc_qe_mva_filter_parameters = {
                    "identifier": cdc_identifier, "cut": cut}
            else:
                cdc_qe_mva_filter_parameters = {
                    "cut": cut}
        elif replace_cdc_qi:
            cdc_qe_mva_filter_parameters = {
                "identifier": cdc_identifier}
        if cdc_qe_mva_filter_parameters is not None:
            # if no cut is specified, the default value is at zero and nothing is deleted.
            basf2.set_module_parameters(
                path,
                name="TFCDC_TrackQualityEstimator",
                filterParameters=cdc_qe_mva_filter_parameters,
                deleteTracks=True,
                resetTakenFlag=True
                )
        if replace_vxd_qi:
            basf2.set_module_parameters(
                path,
                name="VXDQualityEstimatorMVA",
                WeightFileIdentifier=vxd_identifier)

        # Replace final quality estimator module by training data collector module
        track_qe_module_name = "TrackQualityEstimatorMVA"
        module_found = False
        new_path = basf2.create_path()
        for module in path.modules():
            if module.name() != track_qe_module_name:
                if not module.name == 'TrackCreator':
                    new_path.add_module(module)
            else:
                # the TrackCreator needs to be conducted before the Collector such that
                # MDSTTracks are related to RecoTracks and d0 and z0 can be read out
                new_path.add_module(
                    'TrackCreator',
                    pdgCodes=[
                        211,
                        321,
                        2212],
                    recoTrackColName='RecoTracks',
                    trackColName='MDSTTracks')  # , useClosestHitToIP=True, useBFieldAtHit=True)
                new_path.add_module(
                    "TrackQETrainingDataCollector",
                    TrainingDataOutputName=self.get_output_file_name(self.get_records_file_name()),
                    collectEventFeatures=True,
                    SVDPlusCDCStandaloneRecoTracksStoreArrayName="SVDPlusCDCStandaloneRecoTracks",
                )
                module_found = True
        if not module_found:
            raise KeyError(f"No module {track_qe_module_name} found in path")
        path = new_path
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
    #: Number of events to generate for the training data set.
    n_events_training = b2luigi.IntParameter()
    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
    #: Define which kind of process shall be used. Decide between simulating BBBAR or BHABHA,
    # MUMU, YY, DDBAR, UUBAR, SSBAR, CCBAR, reconstructing DATA or already simulated
    # files (USESIMBB/EE) or running on existing reconstructed files (USERECBB/EE)
    process_type = b2luigi.Parameter(
        #: \cond
        default="BBBAR"
        #: \endcond
    )
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

    @property
    def weightfile_identifier_basename(self):
        """
        Property defining the basename for the .xml and .root weightfiles that are created.
        Has to be implemented by the inheriting teacher task class.
        """
        raise NotImplementedError(
            "Teacher Task must define a static weightfile_identifier"
        )

    def get_weightfile_xml_identifier(self, fast_bdt_option=None, recotrack_option=None):
        """
        Name of the xml weightfile that is created by the teacher task.
        It is subsequently used as a local weightfile in the following validation tasks.
        """
        if fast_bdt_option is None:
            fast_bdt_option = self.fast_bdt_option
        if recotrack_option is None and hasattr(self, 'recotrack_option'):
            recotrack_option = self.recotrack_option
        else:
            recotrack_option = ''
        weightfile_details = create_fbdt_option_string(fast_bdt_option)
        weightfile_name = self.weightfile_identifier_basename + weightfile_details
        if recotrack_option != '':
            weightfile_name = weightfile_name + '_' + recotrack_option
        return weightfile_name + ".weights.xml"

    @property
    def tree_name(self):
        """
        Property defining the name of the tree in the ROOT file from the
        ``data_collection_task`` that contains the recorded training data.  Must
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
    def data_collection_task(self) -> Basf2PathTask:
        """
        Property defining the specific ``DataCollectionTask`` to require.  Must
        implemented by the inheriting specific teacher task class.
        """
        raise NotImplementedError(
            "Teacher Task must define a data collection task to require "
        )

    def requires(self):
        """
        Generate list of luigi Tasks that this Task depends on.
        """
        if 'USEREC' in self.process_type:
            if 'USERECBB' in self.process_type:
                process = 'BBBAR'
            elif 'USERECEE' in self.process_type:
                process = 'BHABHA'
            yield CheckExistingFile(
                filename='datafiles/qe_records_N' + str(self.n_events_training) + '_' + process + '_' + self.random_seed + '.root',
                )
        else:
            yield self.data_collection_task(
                num_processes=MasterTask.num_processes,
                n_events=self.n_events_training,
                experiment_number=self.experiment_number,
                random_seed=self.process_type + '_' + self.random_seed,
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
        if 'USEREC' in self.process_type:
            if 'USERECBB' in self.process_type:
                process = 'BBBAR'
            elif 'USERECEE' in self.process_type:
                process = 'BHABHA'
            records_files = ['datafiles/qe_records_N' + str(self.n_events_training) +
                             '_' + process + '_' + self.random_seed + '.root']
        else:
            if hasattr(self, 'recotrack_option'):
                records_files = self.get_input_file_names(
                    self.data_collection_task.get_records_file_name(
                        self.data_collection_task,
                        n_events=self.n_events_training,
                        random_seed=self.process_type + '_' + self.random_seed,
                        recotrack_option=self.recotrack_option))
            else:
                records_files = self.get_input_file_names(
                    self.data_collection_task.get_records_file_name(
                        self.data_collection_task,
                        n_events=self.n_events_training,
                        random_seed=self.process_type + '_' + self.random_seed))

        my_basf2_mva_teacher(
            records_files=records_files,
            tree_name=self.tree_name,
            weightfile_identifier=self.get_output_file_name(self.get_weightfile_xml_identifier()),
            target_variable=self.training_target,
            exclude_variables=self.exclude_variables,
            fast_bdt_option=self.fast_bdt_option,
        )


class VXDQETeacherTask(TrackQETeacherBaseTask):
    """
    Task to run basf2 mva teacher on collected data for VXDTF2 track quality estimator
    """
    #: Name of the weightfile that is created.
    weightfile_identifier_basename = "vxdtf2_mva_qe"
    #: Name of the TTree in the ROOT file from the ``data_collection_task`` that
    # contains the training data for the MVA teacher.
    tree_name = "tree"
    #: Random basf2 seed used to create the training data set.
    random_seed = "train_vxd"
    #: Defines DataCollectionTask to require by tha base class to collect
    # features for the MVA training.
    data_collection_task = VXDQEDataCollectionTask


class CDCQETeacherTask(TrackQETeacherBaseTask):
    """
    Task to run basf2 mva teacher on collected data for CDC track quality estimator
    """
    #: Name of the weightfile that is created.
    weightfile_identifier_basename = "cdc_mva_qe"
    #: Name of the TTree in the ROOT file from the ``data_collection_task`` that
    # contains the training data for the MVA teacher.
    tree_name = "records"
    #: Random basf2 seed used to create the training data set.
    random_seed = "train_cdc"
    #: Defines DataCollectionTask to require by tha base class to collect
    # features for the MVA training.
    data_collection_task = CDCQEDataCollectionTask


class RecoTrackQETeacherTask(TrackQETeacherBaseTask):
    """
    Task to run basf2 mva teacher on collected data for the final, combined
    track quality estimator
    """
    #: RecoTrack option, use string that is additive: deleteCDCQI0XY (= deletes CDCTracks with
    # CDC-QI below 0.XY), useCDC (= uses trained CDC stored in datafiles/), useVXD (uses trained
    # VXD stored in datafiles/), noVXD (= doesn't use the VXD MVA at all)
    recotrack_option = b2luigi.Parameter(
        #: \cond
        default='deleteCDCQI080'
        #: \endcond
    )

    #: Name of the weightfile that is created.
    weightfile_identifier_basename = "recotrack_mva_qe"
    #: Name of the TTree in the ROOT file from the ``data_collection_task`` that
    # contains the training data for the MVA teacher.
    tree_name = "tree"
    #: Random basf2 seed used to create the training data set.
    random_seed = "train_rec"
    #: Defines DataCollectionTask to require by tha base class to collect
    # features for the MVA training.
    data_collection_task = RecoTrackQEDataCollectionTask
    #: Feature/variable to use as truth label for the CDC track quality estimator.
    cdc_training_target = b2luigi.Parameter()

    def requires(self):
        """
        Generate list of luigi Tasks that this Task depends on.
        """
        if 'USEREC' in self.process_type:
            if 'USERECBB' in self.process_type:
                process = 'BBBAR'
            elif 'USERECEE' in self.process_type:
                process = 'BHABHA'
            yield CheckExistingFile(
                filename='datafiles/qe_records_N' + str(self.n_events_training) + '_' + process + '_' + self.random_seed + '.root',
                )
        else:
            yield self.data_collection_task(
                cdc_training_target=self.cdc_training_target,
                num_processes=MasterTask.num_processes,
                n_events=self.n_events_training,
                experiment_number=self.experiment_number,
                random_seed=self.process_type + '_' + self.random_seed,
                recotrack_option=self.recotrack_option,
                fast_bdt_option=self.fast_bdt_option,
                )


class HarvestingValidationBaseTask(Basf2PathTask):
    """
    Run track reconstruction with MVA quality estimator and write out
    (="harvest") a root file with variables useful for the validation.
    """

    #: Number of events to generate for the test data set.
    n_events_testing = b2luigi.IntParameter()
    #: Number of events to generate for the training data set.
    n_events_training = b2luigi.IntParameter()
    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
    #: Define which kind of process shall be used. Decide between simulating BBBAR or BHABHA,
    # MUMU, YY, DDBAR, UUBAR, SSBAR, CCBAR, reconstructing DATA or already simulated
    # files (USESIMBB/EE) or running on existing reconstructed files (USERECBB/EE)
    process_type = b2luigi.Parameter(
        #: \cond
        default="BBBAR"
        #: \endcond
    )
    #: List of collected variables to not use in the training of the QE MVA classifier.
    # In addition to variables containing the "truth" substring, which are excluded by default.
    exclude_variables = b2luigi.ListParameter(
        #: \cond
        hashed=True
        #: \endcond
    )
    #: Hyperparameter option of the FastBDT algorithm. default are the FastBDT default values.
    fast_bdt_option = b2luigi.ListParameter(
        #: \cond
        hashed=True, default=[200, 8, 3, 0.1]
        #: \endcond
    )
    #: Name of the "harvested" ROOT output file with variables that can be used for validation.
    validation_output_file_name = "harvesting_validation.root"
    #: Name of the output of the RootOutput module with reconstructed events.
    reco_output_file_name = "reconstruction.root"
    #: Components for the MC matching and track fit creation.
    components = None

    @property
    def teacher_task(self) -> TrackQETeacherBaseTask:
        """
        Teacher task to require to provide a quality estimator weightfile for ``add_tracking_with_quality_estimation``
        """
        raise NotImplementedError()

    def add_tracking_with_quality_estimation(self, path: basf2.Path) -> None:
        """
        Add modules for track reconstruction to basf2 path that are to be
        validated.  Besides track finding it should include MC matching, fitted
        track creation and a quality estimator module.
        """
        raise NotImplementedError()

    def requires(self):
        """
        Generate list of luigi Tasks that this Task depends on.
        """
        yield self.teacher_task(
            n_events_training=self.n_events_training,
            experiment_number=self.experiment_number,
            process_type=self.process_type,
            exclude_variables=self.exclude_variables,
            fast_bdt_option=self.fast_bdt_option,
        )
        if 'USE' in self.process_type:  # USESIM and USEREC
            if 'BB' in self.process_type:
                process = 'BBBAR'
            elif 'EE' in self.process_type:
                process = 'BHABHA'
            yield CheckExistingFile(
                filename='datafiles/generated_mc_N' + str(self.n_events_testing) + '_' + process + '_test.root'
                )
        else:
            yield SplitNMergeSimTask(
                bkgfiles_dir=MasterTask.bkgfiles_by_exp[self.experiment_number],
                random_seed=self.process_type + '_test',
                n_events=self.n_events_testing,
                experiment_number=self.experiment_number,
                )

    def output(self):
        """
        Generate list of output files that the task should produce.
        The task is considered finished if and only if the outputs all exist.
        """
        yield self.add_to_output(self.validation_output_file_name)
        yield self.add_to_output(self.reco_output_file_name)

    def create_path(self):
        """
        Create a basf2 path that uses ``add_tracking_with_quality_estimation()``
        and adds the ``CombinedTrackingValidationModule`` to write out variables
        for validation.
        """
        # prepare track finding
        path = basf2.create_path()
        if 'USE' in self.process_type:
            if 'BB' in self.process_type:
                process = 'BBBAR'
            elif 'EE' in self.process_type:
                process = 'BHABHA'
            inputFileNames = ['datafiles/generated_mc_N' + str(self.n_events_testing) + '_' + process + '_test.root']
        else:
            inputFileNames = self.get_input_file_names(GenerateSimTask.output_file_name(
                GenerateSimTask, n_events=self.n_events_testing, random_seed=self.process_type + '_test'))
        path.add_module(
            "RootInput",
            inputFileNames=inputFileNames,
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

    #: Name of the "harvested" ROOT output file with variables that can be used for validation.
    validation_output_file_name = "vxd_qe_harvesting_validation.root"
    #: Name of the output of the RootOutput module with reconstructed events.
    reco_output_file_name = "vxd_qe_reconstruction.root"
    #: Teacher task to require to provide a quality estimator weightfile for ``add_tracking_with_quality_estimation``
    teacher_task = VXDQETeacherTask

    def add_tracking_with_quality_estimation(self, path):
        """
        Add modules for VXDTF2 tracking with VXD quality estimator to basf2 path.
        """
        tracking.add_vxd_track_finding_vxdtf2(
            path,
            components=["SVD"],
            reco_tracks="RecoTracks",
            add_mva_quality_indicator=True,
        )
        # Replace the weightfiles of all quality estimator module by those
        # produced in this training by b2luigi
        basf2.set_module_parameters(
            path,
            name="VXDQualityEstimatorMVA",
            WeightFileIdentifier=self.get_input_file_names(
                self.teacher_task.get_weightfile_xml_identifier(self.teacher_task, fast_bdt_option=self.fast_bdt_option)
            )[0],
        )
        tracking.add_mc_matcher(path, components=["SVD"])
        tracking.add_track_fit_and_track_creator(path, components=["SVD"])


class CDCQEHarvestingValidationTask(HarvestingValidationBaseTask):
    """
    Run CDC reconstruction and write out (="harvest") a root file with variables
    useful for validation of the CDC Quality Estimator.
    """
    #: Feature/variable to use as truth label in the quality estimator MVA classifier.
    training_target = b2luigi.Parameter()
    #: Name of the "harvested" ROOT output file with variables that can be used for validation.
    validation_output_file_name = "cdc_qe_harvesting_validation.root"
    #: Name of the output of the RootOutput module with reconstructed events.
    reco_output_file_name = "cdc_qe_reconstruction.root"
    #: Teacher task to require to provide a quality estimator weightfile for ``add_tracking_with_quality_estimation``
    teacher_task = CDCQETeacherTask

    # overload needed due to specific training target
    def requires(self):
        """
        Generate list of luigi Tasks that this Task depends on.
        """
        yield self.teacher_task(
            n_events_training=self.n_events_training,
            experiment_number=self.experiment_number,
            process_type=self.process_type,
            training_target=self.training_target,
            exclude_variables=self.exclude_variables,
            fast_bdt_option=self.fast_bdt_option,
        )
        if 'USE' in self.process_type:  # USESIM and USEREC
            if 'BB' in self.process_type:
                process = 'BBBAR'
            elif 'EE' in self.process_type:
                process = 'BHABHA'
            yield CheckExistingFile(
                filename='datafiles/generated_mc_N' + str(self.n_events_testing) + '_' + process + '_test.root'
                )
        else:
            yield SplitNMergeSimTask(
                bkgfiles_dir=MasterTask.bkgfiles_by_exp[self.experiment_number],
                random_seed=self.process_type + '_test',
                n_events=self.n_events_testing,
                experiment_number=self.experiment_number,
                )

    def add_tracking_with_quality_estimation(self, path):
        """
        Add modules for CDC standalone tracking with CDC quality estimator to basf2 path.
        """
        tracking.add_cdc_track_finding(
            path,
            output_reco_tracks="RecoTracks",
            add_mva_quality_indicator=True,
        )
        # change weightfile of quality estimator to the one produced by this training script
        cdc_qe_mva_filter_parameters = {
            "identifier": self.get_input_file_names(
                CDCQETeacherTask.get_weightfile_xml_identifier(
                    CDCQETeacherTask,
                    fast_bdt_option=self.fast_bdt_option))[0]}
        basf2.set_module_parameters(
            path,
            name="TFCDC_TrackQualityEstimator",
            filterParameters=cdc_qe_mva_filter_parameters,
        )
        tracking.add_mc_matcher(path, components=["CDC"])
        tracking.add_track_fit_and_track_creator(path, components=["CDC"])


class RecoTrackQEHarvestingValidationTask(HarvestingValidationBaseTask):
    """
    Run track reconstruction and write out (="harvest") a root file with variables
    useful for validation of the MVA track Quality Estimator.
    """
    #: Feature/variable to use as truth label for the CDC track quality estimator.
    cdc_training_target = b2luigi.Parameter()
    #: Name of the "harvested" ROOT output file with variables that can be used for validation.
    validation_output_file_name = "reco_qe_harvesting_validation.root"
    #: Name of the output of the RootOutput module with reconstructed events.
    reco_output_file_name = "reco_qe_reconstruction.root"
    #: Teacher task to require to provide a quality estimator weightfile for ``add_tracking_with_quality_estimation``
    teacher_task = RecoTrackQETeacherTask

    def requires(self):
        """
        Generate list of luigi Tasks that this Task depends on.
        """
        yield CDCQETeacherTask(
            n_events_training=self.n_events_training,
            experiment_number=self.experiment_number,
            process_type=self.process_type,
            training_target=self.cdc_training_target,
            exclude_variables=MasterTask.exclude_variables_cdc,
            fast_bdt_option=self.fast_bdt_option,
            )
        yield VXDQETeacherTask(
            n_events_training=self.n_events_training,
            experiment_number=self.experiment_number,
            process_type=self.process_type,
            exclude_variables=MasterTask.exclude_variables_vxd,
            fast_bdt_option=self.fast_bdt_option,
            )

        yield self.teacher_task(
            n_events_training=self.n_events_training,
            experiment_number=self.experiment_number,
            process_type=self.process_type,
            exclude_variables=self.exclude_variables,
            cdc_training_target=self.cdc_training_target,
            fast_bdt_option=self.fast_bdt_option,
            )
        if 'USE' in self.process_type:  # USESIM and USEREC
            if 'BB' in self.process_type:
                process = 'BBBAR'
            elif 'EE' in self.process_type:
                process = 'BHABHA'
            yield CheckExistingFile(
                filename='datafiles/generated_mc_N' + str(self.n_events_testing) + '_' + process + '_test.root'
                )
        else:
            yield SplitNMergeSimTask(
                bkgfiles_dir=MasterTask.bkgfiles_by_exp[self.experiment_number],
                random_seed=self.process_type + '_test',
                n_events=self.n_events_testing,
                experiment_number=self.experiment_number,
                )

    def add_tracking_with_quality_estimation(self, path):
        """
        Add modules for reco tracking with all track quality estimators to basf2 path.
        """

        # add tracking recontonstruction with quality estimator modules added
        tracking.add_tracking_reconstruction(
            path,
            add_cdcTrack_QI=True,
            add_vxdTrack_QI=True,
            add_recoTrack_QI=True,
            skipGeometryAdding=True,
            skipHitPreparerAdding=False,
        )

        # Replace the weightfiles of all quality estimator modules by those
        # produced in the training by b2luigi
        cdc_qe_mva_filter_parameters = {
            "identifier": self.get_input_file_names(
                CDCQETeacherTask.get_weightfile_xml_identifier(
                    CDCQETeacherTask,
                    fast_bdt_option=self.fast_bdt_option))[0]}
        basf2.set_module_parameters(
            path,
            name="TFCDC_TrackQualityEstimator",
            filterParameters=cdc_qe_mva_filter_parameters,
        )
        basf2.set_module_parameters(
            path,
            name="VXDQualityEstimatorMVA",
            WeightFileIdentifier=self.get_input_file_names(
                VXDQETeacherTask.get_weightfile_xml_identifier(VXDQETeacherTask, fast_bdt_option=self.fast_bdt_option)
            )[0],
        )
        basf2.set_module_parameters(
            path,
            name="TrackQualityEstimatorMVA",
            WeightFileIdentifier=self.get_input_file_names(
                RecoTrackQETeacherTask.get_weightfile_xml_identifier(RecoTrackQETeacherTask, fast_bdt_option=self.fast_bdt_option)
            )[0],
        )


class TrackQEEvaluationBaseTask(Task):
    """
    Base class for evaluating a quality estimator ``basf2_mva_evaluate.py`` on a
    separate test data set.

    Evaluation tasks for VXD, CDC and combined QE can inherit from it.
    """

    #: Use git hash / release of basf2 version as additional luigi parameter.
    # This parameter is already set in all other tasks that inherit from
    # ``Basf2Task``. For this task, I decided against inheriting from
    # ``Basf2Task`` because it already calls a subprocess and therefore does not
    # need a dispatchable ``process`` method.
    git_hash = b2luigi.Parameter(
        #: \cond
        default=get_basf2_git_hash()
        #: \endcond
    )
    #: Number of events to generate for the test data set.
    n_events_testing = b2luigi.IntParameter()
    #: Number of events to generate for the training data set.
    n_events_training = b2luigi.IntParameter()
    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
    #: Define which kind of process shall be used. Decide between simulating BBBAR or BHABHA,
    # MUMU, YY, DDBAR, UUBAR, SSBAR, CCBAR, reconstructing DATA or already simulated
    # files (USESIMBB/EE) or running on existing reconstructed files (USERECBB/EE)
    process_type = b2luigi.Parameter(
        #: \cond
        default="BBBAR"
        #: \endcond
    )
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
        hashed=True
        #: \endcond
    )
    #: Hyperparameter options for the FastBDT algorithm.
    fast_bdt_option = b2luigi.ListParameter(
        #: \cond
        hashed=True, default=[200, 8, 3, 0.1]
        #: \endcond
    )

    @property
    def teacher_task(self) -> TrackQETeacherBaseTask:
        """
        Property defining specific teacher task to require.
        """
        raise NotImplementedError(
            "Evaluation Tasks must define a teacher task to require "
        )

    @property
    def data_collection_task(self) -> Basf2PathTask:
        """
        Property defining the specific ``DataCollectionTask`` to require.  Must
        implemented by the inheriting specific teacher task class.
        """
        raise NotImplementedError(
            "Evaluation Tasks must define a data collection task to require "
        )

    @property
    def task_acronym(self):
        """
        Acronym to distinguish between cdc, vxd and rec(o) MVA
        """
        raise NotImplementedError(
            "Evalutation Tasks must define a task acronym."
        )

    def requires(self):
        """
        Generate list of luigi Tasks that this Task depends on.
        """
        yield self.teacher_task(
            n_events_training=self.n_events_training,
            experiment_number=self.experiment_number,
            process_type=self.process_type,
            training_target=self.training_target,
            exclude_variables=self.exclude_variables,
            fast_bdt_option=self.fast_bdt_option,
        )
        if 'USEREC' in self.process_type:
            if 'USERECBB' in self.process_type:
                process = 'BBBAR'
            elif 'USERECEE' in self.process_type:
                process = 'BHABHA'
            yield CheckExistingFile(
                filename='datafiles/qe_records_N' + str(self.n_events_testing) + '_' + process + '_test_' +
                         self.task_acronym + '.root'
                )
        else:
            yield self.data_collection_task(
                num_processes=MasterTask.num_processes,
                n_events=self.n_events_testing,
                experiment_number=self.experiment_number,
                random_seed=self.process_type + '_test',
                )

    def output(self):
        """
        Generate list of output files that the task should produce.
        The task is considered finished if and only if the outputs all exist.
        """
        weightfile_details = create_fbdt_option_string(self.fast_bdt_option)
        evaluation_pdf_output = self.teacher_task.weightfile_identifier_basename + weightfile_details + ".pdf"
        yield self.add_to_output(evaluation_pdf_output)

    @b2luigi.on_temporary_files
    def run(self):
        """
        Run ``basf2_mva_evaluate.py`` subprocess to evaluate QE MVA.

        The MVA weight file created from training on the training data set is
        evaluated on separate test data.
        """
        weightfile_details = create_fbdt_option_string(self.fast_bdt_option)
        evaluation_pdf_output_basename = self.teacher_task.weightfile_identifier_basename + weightfile_details + ".pdf"

        evaluation_pdf_output_path = self.get_output_file_name(evaluation_pdf_output_basename)

        if 'USEREC' in self.process_type:
            if 'USERECBB' in self.process_type:
                process = 'BBBAR'
            elif 'USERECEE' in self.process_type:
                process = 'BHABHA'
            datafiles = 'datafiles/qe_records_N' + str(self.n_events_testing) + '_' + \
                process + '_test_' + self.task_acronym + '.root'
        else:
            datafiles = self.get_input_file_names(
                self.data_collection_task.get_records_file_name(
                    self.data_collection_task,
                    n_events=self.n_events_testing,
                    random_seed=self.process + '_test_' +
                    self.task_acronym))[0]
        cmd = [
            "basf2_mva_evaluate.py",
            "--identifiers",
            self.get_input_file_names(
                self.teacher_task.get_weightfile_xml_identifier(
                    self.teacher_task,
                    fast_bdt_option=self.fast_bdt_option))[0],
            "--datafiles",
            datafiles,
            "--treename",
            self.teacher_task.tree_name,
            "--outputfile",
            evaluation_pdf_output_path,
        ]

        # Prepare log files
        log_file_dir = get_log_file_dir(self)
        # check if directory already exists, if not, create it. I think this is necessary as this task does not
        # inherit properly from b2luigi and thus does not do it automatically??
        try:
            os.makedirs(log_file_dir, exist_ok=True)
        # the following should be unnecessary as exist_ok=True should take care that no FileExistError rises. I
        # might ask about a permission error...
        except FileExistsError:
            print('Directory ' + log_file_dir + 'already exists.')
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
    #: Task that is required by the evaluation base class to create the MVA
    # weightfile that needs to be evaluated.
    teacher_task = VXDQETeacherTask
    #: Task that is required by the evaluation base class to collect the test
    # data for the evaluation.
    data_collection_task = VXDQEDataCollectionTask
    #: Acronym that is required by the evaluation base class to find the correct
    # collection task file
    task_acronym = 'vxd'


class CDCTrackQEEvaluationTask(TrackQEEvaluationBaseTask):
    """
    Run ``basf2_mva_evaluate.py`` for the CDC quality estimator on separate test data
    """
    #: Task that is required by the evaluation base class to create the MVA
    # weightfile that needs to be evaluated.
    teacher_task = CDCQETeacherTask
    #: Task that is required by the evaluation base class to collect the test
    # data for the evaluation.
    data_collection_task = CDCQEDataCollectionTask
    #: Acronym that is required by the evaluation base class to find the correct
    # collection task file
    task_acronym = 'cdc'


class RecoTrackQEEvaluationTask(TrackQEEvaluationBaseTask):
    """
    Run ``basf2_mva_evaluate.py`` for the final, combined quality estimator on
    separate test data
    """
    #: Task that is required by the evaluation base class to create the MVA
    # weightfile that needs to be evaluated.
    teacher_task = RecoTrackQETeacherTask
    #: Task that is required by the evaluation base class to collect the test
    # data for the evaluation.
    data_collection_task = RecoTrackQEDataCollectionTask
    #: Acronym that is required by the evaluation base class to find the correct
    # collection task file
    task_acronym = 'rec'
    #: Feature/variable to use as truth label for the CDC track quality estimator.
    cdc_training_target = b2luigi.Parameter()

    def requires(self):
        """
        Generate list of luigi Tasks that this Task depends on.
        """
        yield self.teacher_task(
            n_events_training=self.n_events_training,
            experiment_number=self.experiment_number,
            process_type=self.process_type,
            training_target=self.training_target,
            exclude_variables=self.exclude_variables,
            cdc_training_target=self.cdc_training_target,
            fast_bdt_option=self.fast_bdt_option,
        )
        if 'USEREC' in self.process_type:
            if 'USERECBB' in self.process_type:
                process = 'BBBAR'
            elif 'USERECEE' in self.process_type:
                process = 'BHABHA'
            yield CheckExistingFile(
                filename='datafiles/qe_records_N' + str(self.n_events_testing) + '_' + process + '_test_' +
                         self.task_acronym + '.root'
                )
        else:
            yield self.data_collection_task(
                num_processes=MasterTask.num_processes,
                n_events=self.n_events_testing,
                experiment_number=self.experiment_number,
                random_seed=self.process_type + "_test",
                cdc_training_target=self.cdc_training_target,
                )


class PlotsFromHarvestingValidationBaseTask(Basf2Task):
    """
    Create a PDF file with validation plots for a quality estimator produced
    from the ROOT ntuples produced by a harvesting validation task
    """
    #: Number of events to generate for the test data set.
    n_events_testing = b2luigi.IntParameter()
    #: Number of events to generate for the training data set.
    n_events_training = b2luigi.IntParameter()
    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
    #: Define which kind of process shall be used. Decide between simulating BBBAR or BHABHA,
    # MUMU, YY, DDBAR, UUBAR, SSBAR, CCBAR, reconstructing DATA or already simulated
    # files (USESIMBB/EE) or running on existing reconstructed files (USERECBB/EE)
    process_type = b2luigi.Parameter(
        #: \cond
        default="BBBAR"
        #: \endcond
    )
    #: List of collected variables to not use in the training of the QE MVA classifier.
    # In addition to variables containing the "truth" substring, which are excluded by default.
    exclude_variables = b2luigi.ListParameter(
        #: \cond
        hashed=True
        #: \endcond
    )
    #: Hyperparameter option of the FastBDT algorithm. default are the FastBDT default values.
    fast_bdt_option = b2luigi.ListParameter(
        #: \cond
        hashed=True, default=[200, 8, 3, 0.1]
        #: \endcond
    )
    #: Whether to normalize the track finding efficiencies to primary particles only.
    primaries_only = b2luigi.BoolParameter(
        #: \cond
        default=True
        #: \endcond
    )  # normalize finding efficiencies to primary MC-tracks

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
        """
        Generate list of luigi Tasks that this Task depends on.
        """
        yield self.harvesting_validation_task_instance

    def output(self):
        """
        Generate list of output files that the task should produce.
        The task is considered finished if and only if the outputs all exist.
        """
        yield self.add_to_output(self.output_pdf_file_basename)

    @b2luigi.on_temporary_files
    def process(self):
        """
        Use basf2_mva teacher to create MVA weightfile from collected training
        data variables.

        Main process that is dispatched by the ``run`` method that is inherited
        from ``Basf2Task``.
        """
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
            teacher_task = self.harvesting_validation_task_instance.teacher_task
            weightfile_identifier = teacher_task.get_weightfile_xml_identifier(teacher_task, fast_bdt_option=self.fast_bdt_option)
            meta_data = {
                "Date": datetime.today().strftime("%Y-%m-%d %H:%M"),
                "Created by steering file": os.path.realpath(__file__),
                "Created from data in": validation_harvest_path,
                "Background directory": MasterTask.bkgfiles_by_exp[self.experiment_number],
                "weight file": weightfile_identifier,
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

    @property
    def harvesting_validation_task_instance(self):
        """
        Harvesting validation task to require, which produces the ROOT files
        with variables to produce the VXD QE validation plots.
        """
        return VXDQEHarvestingValidationTask(
            n_events_testing=self.n_events_testing,
            n_events_training=self.n_events_training,
            process_type=self.process_type,
            experiment_number=self.experiment_number,
            exclude_variables=self.exclude_variables,
            num_processes=MasterTask.num_processes,
            fast_bdt_option=self.fast_bdt_option,
        )


class CDCQEValidationPlotsTask(PlotsFromHarvestingValidationBaseTask):
    """
    Create a PDF file with validation plots for the CDC track quality estimator
    produced from the ROOT ntuples produced by a CDC track QE harvesting
    validation task
    """
    #: Feature/variable to use as truth label in the quality estimator MVA classifier.
    training_target = b2luigi.Parameter()

    @property
    def harvesting_validation_task_instance(self):
        """
        Harvesting validation task to require, which produces the ROOT files
        with variables to produce the CDC QE validation plots.
        """
        return CDCQEHarvestingValidationTask(
            n_events_testing=self.n_events_testing,
            n_events_training=self.n_events_training,
            process_type=self.process_type,
            experiment_number=self.experiment_number,
            training_target=self.training_target,
            exclude_variables=self.exclude_variables,
            num_processes=MasterTask.num_processes,
            fast_bdt_option=self.fast_bdt_option,
        )


class RecoTrackQEValidationPlotsTask(PlotsFromHarvestingValidationBaseTask):
    """
    Create a PDF file with validation plots for the reco MVA track quality
    estimator produced from the ROOT ntuples produced by a reco track QE
    harvesting validation task
    """
    #: Feature/variable to use as truth label for the CDC track quality estimator.
    cdc_training_target = b2luigi.Parameter()

    @property
    def harvesting_validation_task_instance(self):
        """
        Harvesting validation task to require, which produces the ROOT files
        with variables to produce the final MVA track QE validation plots.
        """
        return RecoTrackQEHarvestingValidationTask(
            n_events_testing=self.n_events_testing,
            n_events_training=self.n_events_training,
            process_type=self.process_type,
            experiment_number=self.experiment_number,
            cdc_training_target=self.cdc_training_target,
            exclude_variables=self.exclude_variables,
            num_processes=MasterTask.num_processes,
            fast_bdt_option=self.fast_bdt_option,
        )


class QEWeightsLocalDBCreatorTask(Basf2Task):
    """
    Collect weightfile identifiers from different teacher tasks and merge them
    into a local database for testing.
    """
    #: Number of events to generate for the training data set.
    n_events_training = b2luigi.IntParameter()
    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
    #: Define which kind of process shall be used. Decide between simulating BBBAR or BHABHA,
    # MUMU, YY, DDBAR, UUBAR, SSBAR, CCBAR, reconstructing DATA or already simulated
    # files (USESIMBB/EE) or running on existing reconstructed files (USERECBB/EE)
    process_type = b2luigi.Parameter(
        #: \cond
        default="BBBAR"
        #: \endcond
    )
    #: Feature/vaiable to use as truth label for the CDC track quality estimator.
    cdc_training_target = b2luigi.Parameter()
    #: Hyperparameter option of the FastBDT algorithm. default are the FastBDT default values.
    fast_bdt_option = b2luigi.ListParameter(
        #: \cond
        hashed=True, default=[200, 8, 3, 0.1]
        #: \endcond
    )

    def requires(self):
        """
        Required teacher tasks
        """
        yield VXDQETeacherTask(
            n_events_training=self.n_events_training,
            process_type=self.process_type,
            experiment_number=self.experiment_number,
            exclude_variables=MasterTask.exclude_variables_vxd,
            fast_bdt_option=self.fast_bdt_option,
        )
        yield CDCQETeacherTask(
            n_events_training=self.n_events_training,
            process_type=self.process_type,
            experiment_number=self.experiment_number,
            training_target=self.cdc_training_target,
            exclude_variables=MasterTask.exclude_variables_cdc,
            fast_bdt_option=self.fast_bdt_option,
        )
        yield RecoTrackQETeacherTask(
            n_events_training=self.n_events_training,
            process_type=self.process_type,
            experiment_number=self.experiment_number,
            cdc_training_target=self.cdc_training_target,
            exclude_variables=MasterTask.exclude_variables_rec,
            fast_bdt_option=self.fast_bdt_option,
        )

    def output(self):
        """
        Local database
        """
        yield self.add_to_output("localdb.tar")

    def process(self):
        """
        Create local database
        """
        current_path = Path.cwd()
        localdb_archive_path = Path(self.get_output_file_name("localdb.tar")).absolute()
        output_dir = localdb_archive_path.parent

        # remove existing local databases in output directories
        self._clean()
        # "Upload" the weightfiles of all 3 teacher tasks into the same localdb
        for task in (VXDQETeacherTask, CDCQETeacherTask, RecoTrackQETeacherTask):
            # Extract xml identifier input file name before switching working directories, as it returns relative paths
            weightfile_xml_identifier_path = os.path.abspath(self.get_input_file_names(
                task.get_weightfile_xml_identifier(task, fast_bdt_option=self.fast_bdt_option))[0])
            # As localdb is created in working directory, chdir into desired output path
            try:
                os.chdir(output_dir)
                # Same as basf2_mva_upload on the command line, creates localdb directory in current working dir
                basf2_mva.upload(
                    weightfile_xml_identifier_path,
                    task.weightfile_identifier_basename,
                    self.experiment_number, 0,
                    self.experiment_number, -1,
                )
            finally:  # Switch back to working directory of b2luigi, even if upload failed
                os.chdir(current_path)

        # Pack localdb into tar archive, so that we can have on single output file instead
        shutil.make_archive(
            base_name=localdb_archive_path.as_posix().split('.')[0],
            format="tar",
            root_dir=output_dir,
            base_dir="localdb",
            verbose=True,
        )

    def _clean(self):
        """
        Remove local database and tar archives in output directory
        """
        localdb_archive_path = Path(self.get_output_file_name("localdb.tar"))
        localdb_path = localdb_archive_path.parent / "localdb"

        if localdb_path.exists():
            print(f"Deleting localdb\n{localdb_path}\nwith contents\n ",
                  "\n  ".join(f.name for f in localdb_path.iterdir()))
            shutil.rmtree(localdb_path, ignore_errors=False)  # recursively delete localdb

        if localdb_archive_path.is_file():
            print(f"Deleting {localdb_archive_path}")
            os.remove(localdb_archive_path)

    def on_failure(self, exception):
        """
        Cleanup: Remove local database to prevent existing outputs when task did not finish successfully
        """
        self._clean()
        # Run existing on_failure from parent class
        super().on_failure(exception)


class MasterTask(b2luigi.WrapperTask):
    """
    Wrapper task that needs to finish for b2luigi to finish running this steering file.

    It is done if the outputs of all required subtasks exist.  It is thus at the
    top of the luigi task graph.  Edit the ``requires`` method to steer which
    tasks and with which parameters you want to run.
    """
    #: Define which kind of process shall be used. Decide between simulating BBBAR or BHABHA,
    # MUMU, YY, DDBAR, UUBAR, SSBAR, CCBAR, reconstructing DATA or already simulated
    # files (USESIMBB/EE) or running on existing reconstructed files (USERECBB/EE)
    process_type = b2luigi.get_setting(
        #: \cond
        "process_type", default='BBBAR'
        #: \endcond
    )
    #: Number of events to generate for the training data set.
    n_events_training = b2luigi.get_setting(
        #: \cond
        "n_events_training", default=20000
        #: \endcond
    )
    #: Number of events to generate for the test data set.
    n_events_testing = b2luigi.get_setting(
        #: \cond
        "n_events_testing", default=5000
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
    #: list of raw-datafiles to be used in the case of processing data.
    datafiles = b2luigi.get_setting("datafiles")
    #: Dictionary with experiment numbers as keys and background directory paths as values
    bkgfiles_by_exp = b2luigi.get_setting("bkgfiles_by_exp")
    #: Transform dictionary keys (exp. numbers) from strings to int
    bkgfiles_by_exp = {int(key): val for (key, val) in bkgfiles_by_exp.items()}
    #: list of variables to exclude for the cdc mva.
    exclude_variables_cdc = [
        "has_matching_segment",
        "size",
        "n_tracks",  # not written out per default anyway
        "avg_hit_dist",
        "cont_layer_mean",
        "cont_layer_variance",
        "cont_layer_max",
        "cont_layer_min",
        "cont_layer_first",
        "cont_layer_last",
        "cont_layer_max_vs_last",
        "cont_layer_first_vs_min",
        "cont_layer_count",
        "cont_layer_occupancy",
        "super_layer_mean",
        "super_layer_variance",
        "super_layer_max_vs_last",
        "super_layer_first_vs_min",
        "super_layer_occupancy",
        "drift_length_mean",
        "drift_length_variance",
        "drift_length_max",
        "drift_length_min",
        "drift_length_sum",
        "norm_drift_length_mean",
        "norm_drift_length_variance",
        "norm_drift_length_max",
        "norm_drift_length_min",
        "norm_drift_length_sum",
        "adc_mean",
        "adc_variance",
        "adc_max",
        "adc_min",
        "adc_sum",
        "tot_mean",
        "tot_variance",
        "tot_max",
        "tot_min",
        "tot_sum",
        "empty_s_mean",
        "empty_s_variance",
        "empty_s_max"]
    #: list of variables to exclude for the vxd mva:
    exclude_variables_vxd = [
        'energyLoss_max', 'energyLoss_min', 'energyLoss_mean', 'energyLoss_std', 'energyLoss_sum',
        'size_max', 'size_min', 'size_mean', 'size_std', 'size_sum',
        'seedCharge_max', 'seedCharge_min', 'seedCharge_mean', 'seedCharge_std', 'seedCharge_sum',
        'tripletFit_P_Mag', 'tripletFit_P_Eta', 'tripletFit_P_Phi', 'tripletFit_P_X', 'tripletFit_P_Y', 'tripletFit_P_Z']
    #: list of variables to exclude for the recotrack mva:
    exclude_variables_rec = [
        'background',
        'ghost',
        'fake',
        'clone',
        '__experiment__',
        '__run__',
        '__event__',
        'N_RecoTracks',
        'N_PXDRecoTracks',
        'N_SVDRecoTracks',
        'N_CDCRecoTracks',
        'N_diff_PXD_SVD_RecoTracks',
        'N_diff_SVD_CDC_RecoTracks',
        'Fit_Successful',
        'Fit_NFailedPoints',
        'Fit_Chi2',
        'N_TrackPoints_without_KalmanFitterInfo',
        'N_Hits_without_TrackPoint',
        'SVD_CDC_CDCwall_Chi2',
        'SVD_CDC_CDCwall_Pos_diff_Z',
        'SVD_CDC_CDCwall_Pos_diff_Pt',
        'SVD_CDC_CDCwall_Pos_diff_Theta',
        'SVD_CDC_CDCwall_Pos_diff_Phi',
        'SVD_CDC_CDCwall_Pos_diff_Mag',
        'SVD_CDC_CDCwall_Pos_diff_Eta',
        'SVD_CDC_CDCwall_Mom_diff_Z',
        'SVD_CDC_CDCwall_Mom_diff_Pt',
        'SVD_CDC_CDCwall_Mom_diff_Theta',
        'SVD_CDC_CDCwall_Mom_diff_Phi',
        'SVD_CDC_CDCwall_Mom_diff_Mag',
        'SVD_CDC_CDCwall_Mom_diff_Eta',
        'SVD_CDC_POCA_Pos_diff_Z',
        'SVD_CDC_POCA_Pos_diff_Pt',
        'SVD_CDC_POCA_Pos_diff_Theta',
        'SVD_CDC_POCA_Pos_diff_Phi',
        'SVD_CDC_POCA_Pos_diff_Mag',
        'SVD_CDC_POCA_Pos_diff_Eta',
        'SVD_CDC_POCA_Mom_diff_Z',
        'SVD_CDC_POCA_Mom_diff_Pt',
        'SVD_CDC_POCA_Mom_diff_Theta',
        'SVD_CDC_POCA_Mom_diff_Phi',
        'SVD_CDC_POCA_Mom_diff_Mag',
        'SVD_CDC_POCA_Mom_diff_Eta',
        'POCA_Pos_Pt',
        'POCA_Pos_Mag',
        'POCA_Pos_Phi',
        'POCA_Pos_Z',
        'POCA_Pos_Theta',
        'PXD_QI',
        'SVD_FitSuccessful',
        'CDC_FitSuccessful',
        'pdg_ID',
        'pdg_ID_Mother',
        'is_Vzero_Daughter',
        'is_Primary',
        'z0',
        'd0',
        'seed_Charge',
        'Fit_Charge',
        'weight_max',
        'weight_min',
        'weight_mean',
        'weight_std',
        'weight_median',
        'weight_n_zeros',
        'weight_firstCDCHit',
        'weight_lastSVDHit',
        'smoothedChi2_max',
        'smoothedChi2_min',
        'smoothedChi2_mean',
        'smoothedChi2_std',
        'smoothedChi2_median',
        'smoothedChi2_n_zeros',
        'smoothedChi2_firstCDCHit',
        'smoothedChi2_lastSVDHit']

    def requires(self):
        """
        Generate list of tasks that needs to be done for luigi to finish running
        this steering file.
        """
        cdc_training_targets = [
            "truth",  # treats clones as backround, only best matched CDC tracks are true
            # "truth_track_is_matched" # treats clones as signal
        ]

        fast_bdt_options = []
        # possible to run over a chosen hyperparameter space if wanted
        # in principle this can be extended to specific options for the three different MVAs
        # for i in range(250, 400, 50):
        #     for j in range(6, 10, 2):
        #         for k in range(2, 6):
        #             for l in range(0, 5):
        #                 fast_bdt_options.append([100 + i, j, 3+k, 0.025+l*0.025])
        # fast_bdt_options.append([200, 8, 3, 0.1]) # default FastBDT option
        fast_bdt_options.append([350, 6, 5, 0.1])

        experiment_numbers = b2luigi.get_setting("experiment_numbers")

        # iterate over all possible combinations of parameters from the above defined parameter lists
        for experiment_number, cdc_training_target, fast_bdt_option in itertools.product(
                experiment_numbers, cdc_training_targets, fast_bdt_options
        ):
            # if test_selected_task is activated, only run the following tasks:
            if b2luigi.get_setting("test_selected_task", default=False):
                # for process_type in ['BHABHA', 'MUMU', 'TAUPAIR', 'YY', 'EEEE', 'EEMUMU', 'UUBAR', \
                #    'DDBAR', 'CCBAR', 'SSBAR', 'BBBAR', 'V0BBBAR', 'V0STUDY']:
                for cut in ['000', '070', '090', '095']:
                    yield RecoTrackQEDataCollectionTask(
                        num_processes=self.num_processes,
                        n_events=self.n_events_testing,
                        experiment_number=experiment_number,
                        random_seed=self.process_type + '_test',
                        recotrack_option='useCDC_noVXD_deleteCDCQI'+cut,
                        cdc_training_target=cdc_training_target,
                        fast_bdt_option=fast_bdt_option,
                    )
                yield CDCQEDataCollectionTask(
                    num_processes=self.num_processes,
                    n_events=self.n_events_testing,
                    experiment_number=experiment_number,
                    random_seed=self.process_type + '_test',
                )
                yield CDCQETeacherTask(
                    n_events_training=self.n_events_training,
                    process_type=self.process_type,
                    experiment_number=experiment_number,
                    exclude_variables=self.exclude_variables_cdc,
                    training_target=cdc_training_target,
                    fast_bdt_option=fast_bdt_option,
                )
            else:
                # if data shall be processed, it can neither be trained nor evaluated
                if 'DATA' in self.process_type:
                    yield VXDQEDataCollectionTask(
                        num_processes=self.num_processes,
                        n_events=self.n_events_testing,
                        experiment_number=experiment_number,
                        random_seed=self.process_type + '_test',
                        )
                    yield CDCQEDataCollectionTask(
                        num_processes=self.num_processes,
                        n_events=self.n_events_testing,
                        experiment_number=experiment_number,
                        random_seed=self.process_type + '_test',
                        )
                    yield RecoTrackQEDataCollectionTask(
                        num_processes=self.num_processes,
                        n_events=self.n_events_testing,
                        experiment_number=experiment_number,
                        random_seed=self.process_type + '_test',
                        recotrack_option='deleteCDCQI080',
                        cdc_training_target=cdc_training_target,
                        fast_bdt_option=fast_bdt_option,
                        )
                else:
                    yield QEWeightsLocalDBCreatorTask(
                        n_events_training=self.n_events_training,
                        process_type=self.process_type,
                        experiment_number=experiment_number,
                        cdc_training_target=cdc_training_target,
                        fast_bdt_option=fast_bdt_option,
                        )

                    if b2luigi.get_setting("run_validation_tasks", default=True):
                        yield RecoTrackQEValidationPlotsTask(
                            n_events_training=self.n_events_training,
                            n_events_testing=self.n_events_testing,
                            process_type=self.process_type,
                            experiment_number=experiment_number,
                            cdc_training_target=cdc_training_target,
                            exclude_variables=self.exclude_variables_rec,
                            fast_bdt_option=fast_bdt_option,
                            )
                        yield CDCQEValidationPlotsTask(
                            n_events_training=self.n_events_training,
                            n_events_testing=self.n_events_testing,
                            process_type=self.process_type,
                            experiment_number=experiment_number,
                            exclude_variables=self.exclude_variables_cdc,
                            training_target=cdc_training_target,
                            fast_bdt_option=fast_bdt_option,
                            )
                        yield VXDQEValidationPlotsTask(
                            n_events_training=self.n_events_training,
                            n_events_testing=self.n_events_testing,
                            process_type=self.process_type,
                            exclude_variables=self.exclude_variables_vxd,
                            experiment_number=experiment_number,
                            fast_bdt_option=fast_bdt_option,
                            )

                        if b2luigi.get_setting("run_mva_evaluate", default=True):
                            # Evaluate trained weightfiles via basf2_mva_evaluate.py on separate testdatasets
                            #  requires a latex installation to work
                            yield RecoTrackQEEvaluationTask(
                                n_events_training=self.n_events_training,
                                n_events_testing=self.n_events_testing,
                                process_type=self.process_type,
                                experiment_number=experiment_number,
                                cdc_training_target=cdc_training_target,
                                exclude_variables=self.exclude_variables_rec,
                                fast_bdt_option=fast_bdt_option,
                                )
                            yield CDCTrackQEEvaluationTask(
                                n_events_training=self.n_events_training,
                                n_events_testing=self.n_events_testing,
                                process_type=self.process_type,
                                experiment_number=experiment_number,
                                exclude_variables=self.exclude_variables_cdc,
                                fast_bdt_option=fast_bdt_option,
                                training_target=cdc_training_target,
                                )
                            yield VXDTrackQEEvaluationTask(
                                n_events_training=self.n_events_training,
                                n_events_testing=self.n_events_testing,
                                process_type=self.process_type,
                                experiment_number=experiment_number,
                                exclude_variables=self.exclude_variables_vxd,
                                fast_bdt_option=fast_bdt_option,
                                )


if __name__ == "__main__":
    # if n_events_test_on_data is specified to be different from -1 in the settings,
    # then stop after N events (mainly useful to test data reconstruction):
    nEventsTestOnData = b2luigi.get_setting("n_events_test_on_data", default=-1)
    if nEventsTestOnData > 0 and 'DATA' in b2luigi.get_setting("process_type", default="BBBAR"):
        from ROOT import Belle2
        environment = Belle2.Environment.Instance()
        environment.setNumberEventsOverride(nEventsTestOnData)
    # if global tags are specified in the settings, use them:
    # e.g. for data use ["data_reprocessing_prompt", "online"]. Make sure to be up to date here
    globaltags = b2luigi.get_setting("globaltags", default=[])
    if len(globaltags) > 0:
        basf2.conditions.reset()
        for gt in globaltags:
            basf2.conditions.prepend_globaltag(gt)
    workers = b2luigi.get_setting("workers", default=1)
    b2luigi.process(MasterTask(), workers=workers)
