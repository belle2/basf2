##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


from pybasf2 import B2ERROR
import basf2 as b2
import ROOT
from ROOT import Belle2, addressof
from tracking.root_utils import root_cd
import numpy as np


from vtx_bgr.var_set import target, var_set, extract_features, extract_target


variables = var_set + [target]
ROOT.gInterpreter.Declare("struct TrackData {" + ''.join("float %s; " % var for var in variables) + "};")


class VTXTrackCollector(b2.Module):
    """
    Module to extract input features from RecoTracks and collect them in a root file for training
    of the VTX background remover expert.
    """

    def __init__(
        self,
        trainingDataOutputName="train.root",
        trackCandidatesColumnName="VTXRecoTracks",
        max_vtx_hits=3
    ):
        """Constructor"""

        super(VTXTrackCollector, self).__init__()

        #: cached value of the output file
        self.output_file_name = trainingDataOutputName
        #: cached name of the RecoTracks StoreArray
        self.trackCandidatesColumnName = trackCandidatesColumnName
        #: Collect training data from tracks with this maximum number of hits
        self.max_vtx_hits = max_vtx_hits
        #: cached probability to sample background (value depends on BG level)
        self.bg_sample_fraction = 0.3

    def initialize(self):
        """Receive signal at the start of event processing"""

        #: Output file to store output tre
        self.rfile = ROOT.TFile(self.output_file_name, "RECREATE")
        #: TTree for output data
        self.tree = ROOT.TTree('tree', 'tree')
        #: Instance of TrackData class
        self.trackData = ROOT.TrackData()

        for key in ROOT.TrackData.__dict__:
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.trackData.__getattribute__(key), int):
                    formstring = '/I'
                self.tree.Branch(key, addressof(self.trackData, key), key + formstring)

    def event(self):
        """Event method"""

        trackCands = Belle2.PyStoreArray(self.trackCandidatesColumnName)

        for trackCand in trackCands:

            # Get truth label for track
            truth = extract_target(trackCand)

            if self.select_track(trackCand, truth):

                features = extract_features(trackCand)

                for i, name in enumerate(var_set):
                    setattr(self.trackData, name, features[i])

                setattr(self.trackData, target, truth)

                with root_cd(self.rfile) as tfile:
                    tfile.tree.Fill()

    def select_track(self, trackCand, target):
        """
        Returns a bool to decide which tracks are picked for training.

        :param trackCand: Belle2.RecoTrack that could be used for training
        :param target: Truth label of trackCand
        """

        nhits = trackCand.getNumberOfVTXHits()

        if nhits < 3:
            B2ERROR("VXDTF2 cannot create tracks with less than 3 vertex detector hits. This is funny")
            return False

        # Currently we do not want to apply BG remover to tracks with more than 3 hits, we pass them through.
        # We should not collect training data from them.
        if nhits > self.max_vtx_hits:
            return False

        # We have more bg tracks than signal tracks. One option to handle this is to downsample the bg tracks.
        if target == 0:
            return np.random.uniform() < self.bg_sample_fraction
        else:
            return True

    def terminate(self):
        """ Close the output file."""

        self.rfile.cd()
        self.rfile.Write()
        self.rfile.Close()
