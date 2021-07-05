##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
from ipython_tools import handler
from root_pandas import read_root, to_root
import numpy as np
import os.path
from subprocess import check_output, CalledProcessError, STDOUT

from shutil import copy

from tracking.adjustments import adjust_module
from tracking.run.event_generation import ReadOrGenerateEventsRun
from tracking.validation.run import TrackingValidationRun

from ROOT import Belle2


class PDF:
    """
    Helper class to show a PDF file in a jupyter notebook.
    """

    def __init__(self, pdf, size=(600, 700)):
        """
        Show a PDF file.
        :param pdf: The filename of the PDF file.
        :param size: The size to use.
        """
        #: cached copy of the pdf filename
        self.pdf = pdf
        #: cached copy of the size
        self.size = size

    def _repr_html_(self):
        """HTML representation"""
        return '<iframe src={0} width={1[0]} height={1[1]}></iframe>'.format(self.pdf, self.size)

    def _repr_latex_(self):
        """LaTeX representation"""
        return r'\includegraphics[width=1.0\textwidth]{{{0}}}'.format(self.pdf)


class MVATeacherAndAnalyser:
    """
    Class for training and analysing a tracking module, which has a MVA filter in it.

    Works best, if you are on a jupyter ntoebook.

    You need to supply a run_class, which includes all needed settings, on how to
    train and execute the module. This class will be mixed in with the normal trackfindingcdc
    run classes, so you can add the setting (e.g. tracking_coverage etc.) as normal.

    One examples is:

    class TestClass:
        # This module will be trained
        recording_module = "FilterBasedVXDCDCTrackMerger"
        # This is the name of the parameter of this module, which will be set to "mva" etc.
        recording_parameter = "filter"

        # These mva cuts will be tested during evaluation.
        evaluation_cuts = [0.1, 0.2, ...]

        tracking_coverage = {
            'UsePXDHits': True,
            'UseSVDHits': True,
            'UseCDCHits': True,
        }

        # Some options, which will control the run classes
        fit_tracks = False
        generator_module = "EvtGenInput"

        # This will be added to the "normal" path, to record the training data (you do not have to set the module to
        # recording, as this is done automatically).
        def add_recording_modules(self, path):
            mctrackfinder = path.add_module('TrackFinderMCTruthRecoTracks',
                                    RecoTracksStoreArrayName='MCRecoTracks',
                                    WhichParticles=[])

            path.add_module('MCRecoTracksMatcher', mcRecoTracksStoreArrayName="MCRecoTracks",
                            prRecoTracksStoreArrayName="CDCRecoTracks", UseCDCHits=True, UsePXDHits=False, UseSVDHits=False)
            path.add_module('MCRecoTracksMatcher', mcRecoTracksStoreArrayName="MCRecoTracks",
                            prRecoTracksStoreArrayName="VXDRecoTracks", UseCDCHits=False, UsePXDHits=True, UseSVDHits=True)

            # Merge CDC and CXD tracks
            path.add_module('FilterBasedVXDCDCTrackMerger',
                            extrapolate=False,
                            CDCRecoTrackStoreArrayName="CDCRecoTracks",
                            VXDRecoTrackStoreArrayName="VXDRecoTracks",
                            MergedRecoTrackStoreArrayName="RecoTracks")

            return path

        # This will be added to the "normal" path, to evaluate the mva cuts. In most cases, this is the same as the
        # add_recording_modules (as the module parameters will be set automatically), but maybe you need
        # more here...
        def add_validation_modules(self, path):
            mctrackfinder = path.add_module('TrackFinderMCTruthRecoTracks',
                                    RecoTracksStoreArrayName='MCRecoTracks',
                                    WhichParticles=[])

            # Merge CDC and CXD tracks
            path.add_module('FilterBasedVXDCDCTrackMerger',
                            extrapolate=True,
                            CDCRecoTrackStoreArrayName="CDCRecoTracks",
                            VXDRecoTrackStoreArrayName="VXDRecoTracks",
                            MergedRecoTrackStoreArrayName="PrefitRecoTracks")

            path.add_module("SetupGenfitExtrapolation")

            path.add_module("DAFRecoFitter", recoTracksStoreArrayName="PrefitRecoTracks")

            path.add_module("TrackCreator", recoTrackColName="PrefitRecoTracks")

            path.add_module("FittedTracksStorer", inputRecoTracksStoreArrayName="PrefitRecoTracks",
                            outputRecoTracksStoreArrayName="RecoTracks")

            # We need to include the matching ourselves, as we have already a matching algorithm in place
            path.add_module('MCRecoTracksMatcher', mcRecoTracksStoreArrayName="MCRecoTracks",
                            prRecoTracksStoreArrayName="RecoTracks", UseCDCHits=True, UsePXDHits=True, UseSVDHits=True)

            return path
    """

    def __init__(self, run_class, use_jupyter=True):
        """Constructor"""

        #: cached copy of the run class
        self.run_class = run_class
        #: cached flag to use jupyter notebook
        self.use_jupyter = use_jupyter

        #: cached name of the output file
        self.recording_file_name = self.run_class.recording_module + ".root"

        #: cached path without extension of the output file
        self.file_name_path, ext = os.path.splitext(self.recording_file_name)

        #: cached path with extension of the training-output file
        self.training_file_name = self.file_name_path + "Training" + ext
        #: cached path with extension of the testing-output file
        self.test_file_name = self.file_name_path + "Testing" + ext

        #: cached identifier
        self.identifier_name = "FastBDT.weights.xml"
        #: cached name of the output PDF file
        self.evaluation_file_name = self.identifier_name + ".pdf"

        #: cached path with extension of the testing-export file
        self.expert_file_name = self.file_name_path + "TestingExport" + ext

        #: cached path of the weight input data
        self.weight_data_location = Belle2.FileSystem.findFile(os.path.join("tracking/data",
                                                                            self.run_class.weight_data_location))

    def train(self):
        """Record a training file, split it in two parts and call the training method of the mva package"""
        if not os.path.exists(self.recording_file_name):
            self._create_records_file()

        if not os.path.exists(self.training_file_name) or not os.path.exists(self.test_file_name):
            self._write_train_and_test_files()

        self._call_training_routine()

    def evaluate_tracking(self):
        """
        Use the trained weight file and call the path again using different mva cuts. Validation using the
        normal tracking validation modules.
        """
        copy(self.identifier_name, self.weight_data_location)

        try:
            os.mkdir("results")
        except FileExistsError:
            pass

        def create_path(mva_cut):
            class ValidationRun(self.run_class, TrackingValidationRun):

                def finder_module(self, path):
                    self.add_validation_modules(path)

                    if mva_cut != 999:
                        adjust_module(path, self.recording_module,
                                      **{self.recording_parameter + "Parameters": {"cut": mva_cut},
                                         self.recording_parameter: "mva"})
                    else:
                        adjust_module(path, self.recording_module, **{self.recording_parameter: "truth"})

                output_file_name = "results/validation_{mva_cut}.root".format(mva_cut=mva_cut)

            run = ValidationRun()

            if not os.path.exists(run.output_file_name):
                return {"path": run.create_path()}
            else:
                return {"path": None}

        assert self.use_jupyter

        calculations = handler.process_parameter_space(create_path, mva_cut=self.run_class.evaluation_cuts + [999])
        calculations.start()
        calculations.wait_for_end()

        return calculations

    def evaluate_classification(self):
        """
        Evaluate the classification power on the test data set and produce a PDF.
        """
        if not os.path.exists(self.expert_file_name) or not os.path.exists(self.evaluation_file_name):
            self._call_evaluation_routine()
            self._call_expert_routine()

        df = read_root(self.expert_file_name).merge(read_root(self.test_file_name), left_index=True, right_index=True)

        if self.use_jupyter:
            from IPython.display import display
            display(PDF(self.evaluation_file_name, size=(800, 800)))

        return df

    def _call_training_routine(self):
        """Call the mva training routine in the train file"""
        try:
            check_output(["trackfindingcdc_teacher", self.training_file_name])
        except CalledProcessError as e:
            raise RuntimeError(e.output)

    def _write_train_and_test_files(self):
        """Split the recorded file into two halves: training and test file and write it back"""
        # TODO: This seems to reorder the columns...
        df = read_root(self.recording_file_name)
        mask = np.random.rand(len(df)) < 0.5
        training_sample = df[mask]
        test_sample = df[~mask]

        to_root(training_sample, self.training_file_name, tree_key="records")
        to_root(test_sample, self.test_file_name, tree_key="records")

    def _create_records_file(self):
        """
        Create a path using the settings of the run_class and process it.
        This will create a ROOT file with the recorded data.
        """
        recording_file_name = self.recording_file_name

        class RecordRun(self.run_class, ReadOrGenerateEventsRun):

            def create_path(self):
                path = ReadOrGenerateEventsRun.create_path(self)

                self.add_recording_modules(path)

                adjust_module(path, self.recording_module,
                              **{self.recording_parameter + "Parameters": {"rootFileName": recording_file_name},
                                 self.recording_parameter: "recording"})

                return path

        run = RecordRun()
        path = run.create_path()

        if self.use_jupyter:
            calculation = handler.process(path)
            calculation.start()
            calculation.wait_for_end()

            return calculation
        else:
            run.execute()

    def _call_expert_routine(self):
        """Call the mva expert"""
        try:
            check_output(["basf2_mva_expert",
                          "--identifiers", self.identifier_name, self.weight_data_location,
                          "--datafiles", self.test_file_name,
                          "--outputfile", self.expert_file_name,
                          "--treename", "records"])
        except CalledProcessError as e:
            raise RuntimeError(e.output)

    def _call_evaluation_routine(self):
        """Call the mva evaluation routine"""
        try:
            check_output(["basf2_mva_evaluate.py",
                          "--identifiers", self.identifier_name, self.weight_data_location,
                          "--train_datafiles", self.training_file_name,
                          "--datafiles", self.test_file_name,
                          "--treename", "records",
                          "--outputfile", self.evaluation_file_name],
                         stderr=STDOUT)
        except CalledProcessError as e:
            raise RuntimeError(e.output)
