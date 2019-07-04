import glob
import os
import subprocess

import matplotlib.pyplot as plt
import numpy as np
import root_pandas
from matplotlib.backends.backend_pdf import PdfPages

import basf2
import basf2_mva
import simulation
import tracking
import tracking.root_utils as root_utils
from tracking.harvesting_validation.combined_module import CombinedTrackingValidationModule

# wrap python modules that are used here but not in the externals into a try except block
install_helpstring_formatter = ("\nCould not find {module} python module.Try installing it via\n"
                                "  python3 -m pip install [--user] {module}\n")
try:
    import b2luigi as luigi
    from b2luigi.basf2_helper import Basf2PathTask, Basf2Task
except ModuleNotFoundError:
    print(install_helpstring_formatter.format(module="b2luigi"))
    raise
try:
    from uncertain_panda import pandas as upd
except ModuleNotFoundError:
    print(install_helpstring_formatter.format(module="uncertain_panda"))
    raise


def my_basf2_mva_teacher(
    records_files,
    tree_name,
    weightfile_identifier,
    target_variable="truth",
    exclude_variables=[],
):
    """
    My custom wrapper for basf2 mva teacher
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


def plot_with_errobands(uncertain_series,
                        error_band_alpha=0.3,
                        plot_kwargs={},
                        fill_between_kwargs={},
                        ax=None):
    """
    Plot an uncertain series with error bands in y
    """
    if ax is None:
        ax = plt.gca()
    ax.plot(uncertain_series.index, uncertain_series.nominal_value, **plot_kwargs)
    ax.fill_between(x=uncertain_series.index,
                    y1=uncertain_series.nominal_value - uncertain_series.std_dev,
                    y2=uncertain_series.nominal_value + uncertain_series.std_dev,
                    alpha=error_band_alpha,
                    **fill_between_kwargs)


class GenerateSimTask(Basf2PathTask):
    """
    Generate simulated Monte Carlo with background overlay
    """

    n_events = luigi.IntParameter()
    random_seed = luigi.Parameter()
    bkgfiles_dir = luigi.Parameter(hashed=True)
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
        bkg_files = glob.glob(os.path.join(self.bkgfiles_dir, "*"))
        simulation.add_simulation(path, bkgfiles=bkg_files, bkgOverlay=True)
        path.add_module(
            "RootOutput",
            outputFileName=self.get_output_file_name(self.output_file_name),
        )
        return path


class CDCQEDataCollectionTask(Basf2PathTask):
    n_events = luigi.IntParameter()
    random_seed = luigi.Parameter()
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
        path.add_module(
            "SetupGenfitExtrapolation", energyLossBrems=False, noiseBrems=False
        )
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


class VXDQEDataCollectionTask(Basf2PathTask):
    n_events = luigi.IntParameter()
    random_seed = luigi.Parameter()
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
        path.add_module(
            "SetupGenfitExtrapolation", energyLossBrems=False, noiseBrems=False
        )
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


class TrackQETeacherBaseTask(Basf2Task):
    n_events_training = luigi.IntParameter()
    training_target = luigi.Parameter(default="truth")
    exclude_variables = luigi.ListParameter(hashed=True, default=[])

    @property
    def weightfile_identifier(self):
        return NotImplementedError(
            "Teacher Task must define a static weightfile_identifier"
        )

    @property
    def tree_name(self):
        return NotImplementedError("Teacher Task must define a static tree_name")

    @property
    def random_seed(self):
        return NotImplementedError("Teacher Task must define a static random seed")

    @property
    def dataCollectionTask(self):
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

    def run(self):
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


class CDCQETeacherTask(TrackQETeacherBaseTask):
    weightfile_identifier = "trackfindingcdc_TrackQualityIndicator.weights.xml"
    tree_name = "records"
    random_seed = "traincdc_0"
    dataCollectionTask = CDCQEDataCollectionTask


class VXDQETeacherTask(TrackQETeacherBaseTask):
    weightfile_identifier = "trackfindingvxd_TrackQualityIndicator.weights.xml"
    tree_name = "tree"
    random_seed = "trainvxd_0"
    dataCollectionTask = VXDQEDataCollectionTask


class VXDQEHarvestingValidationTask(Basf2PathTask):
    n_events_testing = luigi.IntParameter()
    n_events_training = luigi.IntParameter()
    validation_output_file_name = "vxd_qe_harvesting_validation.root"
    reco_output_file_name = "vxd_qe_reconstruction.root"

    def requires(self):
        yield VXDQETeacherTask(n_events_training=self.n_events_training)
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
        path = basf2.create_path()
        path.add_module(
            "RootInput",
            inputFileNames=self.get_input_file_names(GenerateSimTask.output_file_name),
        )
        path.add_module("Gearbox")
        tracking.add_geometry_modules(path)
        tracking.add_hit_preparation_modules(path)  # only needed for simulated hits
        path.add_module(
            "SetupGenfitExtrapolation", energyLossBrems=False, noiseBrems=False
        )
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
                VXDQETeacherTask.weightfile_identifier
            )[0],
        )
        tracking.add_mc_matcher(path, components=["SVD"])
        tracking.add_track_fit_and_track_creator(path, components=["SVD"])
        path.add_module(
            CombinedTrackingValidationModule(
                name="Harvesting Validation",
                contact="michael.eliachevitch@kit.edu",
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


class CDCQEHarvestingValidationTask(Basf2PathTask):
    n_events_testing = luigi.IntParameter()
    n_events_training = luigi.IntParameter()
    training_target = luigi.Parameter()
    validation_output_file_name = "cdc_qe_harvesting_validation.root"
    reco_output_file_name = "cdc_qe_reconstruction.root"

    def requires(self):
        yield CDCQETeacherTask(
            n_events_training=self.n_events_training,
            training_target=self.training_target,
        )
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
        path = basf2.create_path()
        path.add_module(
            "RootInput",
            inputFileNames=self.get_input_file_names(GenerateSimTask.output_file_name),
        )
        path.add_module("Gearbox")
        tracking.add_geometry_modules(path)
        tracking.add_hit_preparation_modules(path)  # only needed for simulated hits
        path.add_module(
            "SetupGenfitExtrapolation", energyLossBrems=False, noiseBrems=False
        )
        tracking.add_cdc_track_finding(
            path,
            output_reco_tracks="RecoTracks",
            use_cdc_quality_estimator=True,
            cdc_quality_estimator_weightfile=self.get_input_file_names(
                CDCQETeacherTask.weightfile_identifier
            )[0],
        )
        tracking.add_mc_matcher(path, components=["CDC"])
        tracking.add_track_fit_and_track_creator(path, components=["CDC"])
        path.add_module(
            CombinedTrackingValidationModule(
                name="Harvesting Validation",
                contact="michael.eliachevitch@kit.edu",
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


class FullTrackQEDataCollectionTask(Basf2PathTask):
    n_events = luigi.IntParameter()
    random_seed = luigi.Parameter()
    records_file_name = "fulltrack_qe_records.root"
    cdc_training_target = luigi.Parameter()

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
        path.add_module(
            "SetupGenfitExtrapolation", energyLossBrems=False, noiseBrems=False
        )
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
            collectEventFeatures=True,
        )
        return path


class FullTrackQETeacherTask(TrackQETeacherBaseTask):
    weightfile_identifier = "fullTrackQualityIndicator.weights.xml"
    tree_name = "tree"
    random_seed = "trainingdata_0"
    dataCollectionTask = FullTrackQEDataCollectionTask
    cdc_training_target = luigi.Parameter()

    def requires(self):
        yield self.dataCollectionTask(
            cdc_training_target=self.cdc_training_target,
            num_processes=MasterTask.num_processes,
            n_events=self.n_events_training,
            random_seed=self.random_seed,
        )


class TrackQEEvaluationBaseTask(Basf2Task):
    """
    Base class for quality estimator MVA evaluation task, from which evaluation
    tasks for vxd, cdc and combined QE can inherit from.
    """

    n_events_testing = luigi.IntParameter()
    n_events_training = luigi.IntParameter()
    training_target = luigi.Parameter(default="truth")

    @property
    def teacherTask(self):
        raise NotImplementedError(
            "Evaluation Tasks must define a teacher task to require "
        )

    @property
    def dataCollectionTask(self):
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

    def run(self):
        evaluation_pdf_output = (
            self.teacherTask.weightfile_identifier.rsplit(".", 1)[0] + ".pdf"
        )
        cmd = [
            "basf2_mva_evaluate.py",
            "--identifier",
            self.get_input_file_names(self.teacherTask.weightfile_identifier)[0],
            "-d",
            self.get_input_file_names(self.dataCollectionTask.records_file_name)[0],
            "--treename",
            self.teacherTask.tree_name,
            "-o",
            self.get_output_file_name(evaluation_pdf_output),
        ]
        print(" ".join(cmd))
        subprocess.check_call(cmd)


class VXDTrackQEEvaluationTask(TrackQEEvaluationBaseTask):
    teacherTask = VXDQETeacherTask
    dataCollectionTask = VXDQEDataCollectionTask


class CDCTrackQEEvaluationTask(TrackQEEvaluationBaseTask):
    teacherTask = CDCQETeacherTask
    dataCollectionTask = CDCQEDataCollectionTask


class FullTrackQEEvaluationTask(TrackQEEvaluationBaseTask):
    teacherTask = FullTrackQETeacherTask
    dataCollectionTask = FullTrackQEDataCollectionTask
    exclude_variables = luigi.ListParameter(hashed=True)
    cdc_training_target = luigi.Parameter()

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


class FullTrackQEHarvestingValidationTask(Basf2PathTask):
    n_events_testing = luigi.IntParameter()
    n_events_training = luigi.IntParameter()
    cdc_training_target = luigi.Parameter()
    exclude_variables = luigi.ListParameter(hashed=True)
    validation_output_file_name = "full_qe_harvesting_validation.root"
    reco_output_file_name = "full_qe_reconstruction.root"

    def requires(self):
        yield CDCQETeacherTask(
            n_events_training=self.n_events_training,
            training_target=self.cdc_training_target,
        )
        yield VXDQETeacherTask(n_events_training=self.n_events_training)
        yield FullTrackQETeacherTask(
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

    def output(self):
        yield self.add_to_output(self.validation_output_file_name)
        yield self.add_to_output(self.reco_output_file_name)

    def create_path(self):
        path = basf2.create_path()
        path.add_module(
            "RootInput",
            inputFileNames=self.get_input_file_names(GenerateSimTask.output_file_name),
        )
        path.add_module("Gearbox")
        tracking.add_geometry_modules(path)
        tracking.add_hit_preparation_modules(path)  # only needed for simulated hits
        path.add_module(
            "SetupGenfitExtrapolation", energyLossBrems=False, noiseBrems=False
        )
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
        )
        path.add_module(
            CombinedTrackingValidationModule(
                name="Harvesting Validation",
                contact="michael.eliachevitch@kit.edu",
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


class PlotsFromHarvestingValidationBaseTask(Basf2Task):
    """
    Create PDF with QI validation plots from ROOT file produced by harvesting validation task
    """
    primaries_only = luigi.BoolParameter(default=True)  # normalize finding efficiencies to primary MC-tracks

    # harvesting task static variable can be used to adapt plotting task to different harvesting validation tasks
    @property
    def harvesting_validation_task_instance(self):
        raise NotImplementedError("Must define a QI harvesting validation task for which to do the plots")

    @property
    def output_pdf_file_basename(self):
        validation_harvest_basename = self.harvesting_validation_task_instance.validation_output_file_name
        return validation_harvest_basename.replace(".root", "_plots.pdf")

    def requires(self):
        yield self.harvesting_validation_task_instance

    def output(self):
        yield self.add_to_output(self.output_pdf_file_basename)

    def run(self):
        validation_harvest_basename = self.harvesting_validation_task_instance.validation_output_file_name
        validation_harvest_path = self.get_input_file_names(validation_harvest_basename)[0]
        output_pdf_file_path = self.get_output_file_name(self.output_pdf_file_basename)
        # Load "harvested" validation data from root files into dataframes (might require a lot of RAM)
        pr_df = root_pandas.read_root(validation_harvest_path, key='pr_tree/pr_tree')
        mc_df = root_pandas.read_root(validation_harvest_path, key='mc_tree/mc_tree')
        if self.primaries_only:
            mc_df = mc_df[mc_df.is_primary.eq(True)]

        # Define QI thresholds for the FOM plots and the ROC curves
        qi_cuts = np.linspace(0., 1, 10, endpoint=False)
        # # Add more points at the very end between the previous maximum and 1
        # qi_cuts = np.append(qi_cuts, np.linspace(np.max(qi_cuts), 1, 20, endpoint=False))

        # Create plots and append them to single output pdf

        with PdfPages(output_pdf_file_path) as pdf:
            # Plot fake rates by using the
            fake_rate_list = [pr_df[pr_df["quality_indicator"] > cut]['is_fake'].unc.mean() for cut in qi_cuts]
            fake_rate_useries = upd.Series(data=fake_rate_list, index=qi_cuts)
            fake_fig, fake_ax = plt.subplots()
            plot_with_errobands(fake_rate_useries, ax=fake_ax)
            fake_ax.set_ylabel("fake rate")
            fake_ax.set_xlabel("quality indicator requirement")
            pdf.savefig(fake_fig, bbox_inches="tight")

            # Plot clone rates
            clone_rate_list = [pr_df[pr_df["quality_indicator"] > cut]['is_clone'].unc.mean() for cut in qi_cuts]
            clone_rate_useries = upd.Series(data=clone_rate_list, index=qi_cuts)
            clone_fig, clone_ax = plt.subplots()
            plot_with_errobands(clone_rate_useries, ax=clone_ax)
            clone_ax.set_ylabel("clone rate")
            clone_ax.set_xlabel("quality indicator requirement")
            pdf.savefig(clone_fig, bbox_inches="tight")

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

            missing_fraction_list = [
                mc_df[mc_df.quality_indicator.isnull() | (mc_df.quality_indicator > i)]['is_missing'].unc.mean()
                for i in qi_cuts
            ]

            findeff_fig, findeff_ax = plt.subplots()
            findeff_useries = 1 - upd.Series(data=missing_fraction_list, index=qi_cuts)
            plot_with_errobands(findeff_useries, ax=findeff_ax)
            findeff_ax.set_ylabel("finding efficiency")
            findeff_ax.set_xlabel("quality indicator requirement")
            pdf.savefig(findeff_fig, bbox_inches="tight")

            # Plot ROC curves

            # Fake rate vs. finding efficiency ROC curve
            fake_roc_fig, fake_roc_ax = plt.subplots()
            fake_roc_ax.errorbar(x=findeff_useries.nominal_value, y=fake_rate_useries.nominal_value,
                                 xerr=findeff_useries.std_dev, yerr=fake_rate_useries.std_dev, elinewidth=0.8)
            fake_roc_ax.set_xlabel('finding efficiency')
            fake_roc_ax.set_ylabel('fake rate')
            pdf.savefig(fake_roc_fig, bbox_inches="tight")

            # Clone rate vs. finding efficiency ROC curve
            clone_roc_fig, clone_roc_ax = plt.subplots()
            clone_roc_ax.errorbar(x=findeff_useries.nominal_value, y=clone_rate_useries.nominal_value,
                                  xerr=findeff_useries.std_dev, yerr=clone_rate_useries.std_dev, elinewidth=0.8)
            clone_roc_ax.set_xlabel('finding efficiency')
            clone_roc_ax.set_ylabel('clone rate')
            pdf.savefig(clone_roc_fig, bbox_inches="tight")

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
                "tan_lambda": "$\\tan{\lambda}$",
                "phi0": "$\phi_0$"
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
            kinematic_qi_cuts = [0, 0.5, 0.9]
            blue, yellow, green = plt.get_cmap("tab10").colors[0:3]
            for param in params:
                fig, axarr = plt.subplots(ncols=len(kinematic_qi_cuts), sharey=True, sharex=True, figsize=(14, 6))
                for i, qi in enumerate(kinematic_qi_cuts):
                    ax = axarr[i]
                    incut = pr_df[(pr_df['quality_indicator'] > qi)]
                    incut_matched = incut[incut.is_matched.eq(True)]
                    incut_clones = incut[incut.is_clone.eq(True)]
                    incut_fake = incut[incut.is_fake.eq(True)]
                    bins = bins_by_param[param]
                    stacked_histogram_series_tuple = (
                        incut_matched[f'{param}_estimate'],
                        incut_clones[f'{param}_estimate'],
                        incut_fake[f'{param}_estimate'],
                    )
                    histvals, _, _ = ax.hist(stacked_histogram_series_tuple,
                                             stacked=True,
                                             bins=bins, range=(bins.min(), bins.max()),
                                             # color=[blue, green, yellow],
                                             label=["matched", "clones", "fakes"])
                    ax.set_title(f"QI > {qi}")
                    ax.set_xlabel(f'{label_by_param[param]} estimate / ({unit_by_param[param]})')
                    ax.set_ylabel('# tracks')
                ax.legend(loc="upper center", bbox_to_anchor=(-1, -0.15))
                pdf.savefig(fig, bbox_inches="tight")


class FullTrackQEValidationPlotsTask(PlotsFromHarvestingValidationBaseTask):
    n_events_testing = luigi.IntParameter()
    n_events_training = luigi.IntParameter()
    cdc_training_target = luigi.Parameter()
    exclude_variables = luigi.ListParameter(hashed=True)

    @property
    def harvesting_validation_task_instance(self):
        return FullTrackQEHarvestingValidationTask(
            n_events_testing=self.n_events_testing,
            n_events_training=self.n_events_training,
            cdc_training_target=self.cdc_training_target,
            exclude_variables=self.exclude_variables,
            num_processes=MasterTask.num_processes,
        )


class CDCQEValidationPlotsTask(PlotsFromHarvestingValidationBaseTask):
    n_events_testing = luigi.IntParameter()
    n_events_training = luigi.IntParameter()
    training_target = luigi.Parameter()

    @property
    def harvesting_validation_task_instance(self):
        return CDCQEHarvestingValidationTask(
            n_events_testing=self.n_events_testing,
            n_events_training=self.n_events_training,
            training_target=self.training_target,
            num_processes=MasterTask.num_processes,
        )


class VXDQEValidationPlotsTask(PlotsFromHarvestingValidationBaseTask):
    n_events_testing = luigi.IntParameter()
    n_events_training = luigi.IntParameter()

    @property
    def harvesting_validation_task_instance(self):
        return VXDQEHarvestingValidationTask(
            n_events_testing=self.n_events_testing,
            n_events_training=self.n_events_training,
            num_processes=MasterTask.num_processes,
        )


class MasterTask(luigi.WrapperTask):
    """
    Entry point: Task that defines the configurations that shall be tested.
    """

    n_events_training = luigi.get_setting("n_events_training", default=3000)
    n_events_testing = luigi.get_setting("n_events_testing", default=1000)
    num_processes = luigi.get_setting("basf2_processes_per_worker", default=0)
    bkgfiles_dir = luigi.get_setting("bkgfiles_directory")

    def requires(self):

        # reco track difference variables,
        # calculated in eventwise extractor -> eventwise?
        rt_diff_variables = [
            "RTs_Min_Mom_diff_Mag",
            "RTs_Min_Mom_diff_Mag_idx",
            "RTs_Min_Mom_diff_Pt",
            "RTs_Min_Mom_diff_Pt_idx",
            "RTs_Min_Pos_diff_Theta",
            "RTs_Min_Pos_diff_Theta_idx",
            "RTs_Min_Pos_diff_Phi",
            "RTs_Min_Pos_diff_Phi_idx",
        ]

        # eventwise n_track_variables
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
                # "truth" # truth includes clones as signal
            ]:
                yield FullTrackQEValidationPlotsTask(
                    cdc_training_target=cdc_training_target,
                    exclude_variables=exclude_variables,
                    n_events_training=self.n_events_training,
                    n_events_testing=self.n_events_testing,
                )
                yield FullTrackQEEvaluationTask(
                    exclude_variables=exclude_variables,
                    cdc_training_target=cdc_training_target,
                    n_events_training=self.n_events_training,
                    n_events_testing=self.n_events_testing,
                )
                yield CDCQEValidationPlotsTask(
                    training_target=cdc_training_target,
                    n_events_training=self.n_events_training,
                    n_events_testing=self.n_events_testing,
                )
                yield CDCTrackQEEvaluationTask(
                    training_target=cdc_training_target,
                    n_events_training=self.n_events_training,
                    n_events_testing=self.n_events_testing,
                )
                yield VXDQEValidationPlotsTask(
                    n_events_training=self.n_events_training,
                    n_events_testing=self.n_events_testing,
                )
                yield VXDTrackQEEvaluationTask(
                    n_events_training=self.n_events_training,
                    n_events_testing=self.n_events_testing,
                )


if __name__ == "__main__":
    # TODO Add comment block with usage instructions
    # TODO complete docstrings for all tasks
    workers = luigi.get_setting("workers", default=1)
    luigi.process(MasterTask(), workers=workers)
