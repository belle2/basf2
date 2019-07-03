import glob
import os
import subprocess
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
        if ("truth" not in name) and (name != target_variable)
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
                yield VXDQEHarvestingValidationTask(
                    n_events_training=self.n_events_training,
                    n_events_testing=self.n_events_testing,
                    num_processes=self.num_processes,
                )
                yield CDCQEHarvestingValidationTask(
                    training_target=cdc_training_target,
                    n_events_training=self.n_events_training,
                    n_events_testing=self.n_events_testing,
                    num_processes=self.num_processes,
                )
                yield FullTrackQEHarvestingValidationTask(
                    cdc_training_target=cdc_training_target,
                    exclude_variables=exclude_variables,
                    n_events_training=self.n_events_training,
                    n_events_testing=self.n_events_testing,
                    num_processes=self.num_processes,
                )
                yield VXDTrackQEEvaluationTask(
                    n_events_training=self.n_events_training,
                    n_events_testing=self.n_events_testing,
                )
                yield CDCTrackQEEvaluationTask(
                    training_target=cdc_training_target,
                    n_events_training=self.n_events_training,
                    n_events_testing=self.n_events_testing,
                )
                yield FullTrackQEEvaluationTask(
                    exclude_variables=exclude_variables,
                    cdc_training_target=cdc_training_target,
                    n_events_training=self.n_events_training,
                    n_events_testing=self.n_events_testing,
                )


if __name__ == "__main__":
    # TODO Add comment block with usage instructions
    # TODO complete docstrings for all tasks
    workers = luigi.get_setting("workers", default=1)
    luigi.process(MasterTask(), workers=workers)
