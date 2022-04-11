#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from pybasf2 import B2ERROR
from ROOT import Belle2
import basf2 as b2
from vtx_bgr.var_set import extract_features, var_set

import numpy as np


class VTXBgRemover(b2.Module):
    """
    Module to remove background tracks from store array using an MVA classifier.
    """

    def __init__(
            self,
            trackCandidatesColumnName="RecoTracks",
            weightFileIdentifier='vxdtf2_vtx_bgr_mva',
            max_vtx_hits=3
            ):
        """Create a member to weighfiles for classifier from DB"""
        super().__init__()  # don't forget to call parent constructor

        #: cached name of the RecoTracks StoreArray
        self.trackCandidatesColumnName = trackCandidatesColumnName

        #: cached name of the database identifier for weightfile
        self.m_identifier = weightFileIdentifier

        #: cached serialized weightfile representation from condDB
        self.m_weightfile_representation = None

        #: cached maximum number of hits
        self.max_vtx_hits = max_vtx_hits

    def initialize(self):
        """
        Initialize the module.
        """

        if self.m_identifier.endswith(".root") or self.m_identifier.endswith(".xml"):
            self.m_weightfile_representation = None
        else:
            self.m_weightfile_representation = Belle2.PyDBObj(self.m_identifier)

        Belle2.MVA.AbstractInterface.initSupportedInterfaces()

    def beginRun(self):
        """
        Called at the beginning of a new run.
        """

        if self.m_weightfile_representation:
            if self.m_weightfile_representation.hasChanged():
                weightfile = Belle2.MVA.Weightfile.loadFromDatabase(self.m_identifier)
                self.init_mva(weightfile)
        else:
            weightfile = Belle2.MVA.Weightfile.loadFromFile(self.m_identifier)
            self.init_mva(weightfile)

    def init_mva(self, weightfile):
        """
        Initialize mva expert, dataset and features
        Called everytime the weightfile in the database changes in begin run

        :param weightfile: instance of Belle2.MVA.Weightfile
        """

        supported_interfaces = Belle2.MVA.AbstractInterface.getSupportedInterfaces()
        general_options = Belle2.MVA.GeneralOptions()
        weightfile.getOptions(general_options)

        #: cached MVA expert for classifying tracks into signal / backrgound
        self.m_expert = supported_interfaces[general_options.m_method].getExpert()
        self.m_expert.load(weightfile)

        #: cached list of feature variables
        self.m_feature_variables = general_options.m_variables

        dummy = np.zeros((self.m_feature_variables.size(),))

        #: cached dataset, defining input to MVA expert for one track
        self.m_dataset = Belle2.MVA.SingleDataset(general_options, dummy, 0)

    def analyse(self, trackCand):
        """
        Calculates expert output for given candidate track

        :param trackCand: Belle2.RecoTrack to analyze with expert
        """

        if not self.m_expert:
            B2ERROR("MVA Expert is not loaded! I will return 0")
            return 0.0

        if trackCand.getNumberOfVTXHits() < 3:
            return 0.0
        elif trackCand.getNumberOfVTXHits() > self.max_vtx_hits:
            return 1.0
        else:
            data = extract_features(trackCand)

            for i in range(len(var_set)):
                self.m_dataset.m_input[i] = data[i]

            return self.m_expert.apply(self.m_dataset)[0]

    def event(self):
        """
        Called for each event.
        """

        trackCands = Belle2.PyStoreArray(self.trackCandidatesColumnName)

        for trackCand in trackCands:

            qiValue = self.analyse(trackCand)

            # Override QualityIndicator with qiValue for later use
            trackCand.setQualityIndicator(qiValue)
