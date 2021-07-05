# !/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#
# utils to measure the per-event execution time of all modules
# appended to the path, SVD observables are also stored in the tree
#
# in order to use this function:
# import it:
#     from svd.executionTime_utils import SVDExtraEventStatisticsModule
# and then use it to measure execution time of your path
#
# examples are in svd/examples/executionTime.py
#
#

from ROOT import Belle2
import numpy as np
from per_event_statistics import PerEventStatisticsGetterModule


class SVDExtraEventStatisticsModule(PerEventStatisticsGetterModule):
    """
    a basf2 python module to export all module time statistics (PerEventStatisticsGetterModule) +
    number of SVDSpacePoints, SVDClusters, SVDShaperDigits and SVDSHaperDigitsZS5
    into a ROOT TTree written to a file.
    """

    def __init__(self, filename):
        """
        creates the module
        @param filename: name of the rootfile where the TTree is written.
        """
        super().__init__(filename)
        #: StoreArray of SVDSpacePoints
        self.svdSPs = Belle2.PyStoreArray("SVDSpacePoints")
        #: StoreArray of SVDClusters
        self.svdclusters = Belle2.PyStoreArray("SVDClusters")
        #: StoreArray of SVDShaperDigits
        self.svdstrips = Belle2.PyStoreArray("SVDShaperDigits")
        #: StoreArray of ZS5  SVDShaperDigits
        self.svdZS5strips = Belle2.PyStoreArray("SVDShaperDigitsZS5")

    def initialize(self):
        """
        Create the needed store object pointer in the DataStore and the TFile with the TTree.
        """

        super().initialize()
        #: array storing the number of SVDSpacePoints
        self.svd_sps = np.zeros(1, dtype=np.int32)
        #: array storing the number of SVDClusters
        self.svd_clusters = np.zeros(1, dtype=np.int32)
        #: array storing the number of SVDShaperDigits
        self.svd_strips = np.zeros(1, dtype=np.int32)
        #: array storing the number of ZS5  SVDShaperDigits
        self.svd_ZS5strips = np.zeros(1, dtype=np.int32)

        #: branch address assignment for SVDSpacePoints
        self.statistics.Branch('svdSPs', self.svd_sps[0:], "svdSPs/I")
        #: branch address assignment for SVDClusters
        self.statistics.Branch('svdClusters', self.svd_clusters[0:], "svdClusters/I")
        #: branch address assignment for SVDShaperDigits
        self.statistics.Branch('svdStrips', self.svd_strips[0:], "svdStrips/I")
        #: branch address assignment for ZS5 SVDShaperDigits
        self.statistics.Branch('svdZS5strips', self.svd_ZS5strips[0:], "svdZS5strips/I")

        # register the StoreArray for the SVD clusters
        self.svdSPs.isRequired()
        self.svdclusters.isRequired()
        self.svdstrips.isRequired()
        self.svdZS5strips.isRequired()

    def event(self):
        """ event """

        self.svd_sps[0] = self.svdSPs.getEntries()

        self.svd_clusters[0] = self.svdclusters.getEntries()
        self.svd_strips[0] = self.svdstrips.getEntries()
        self.svd_ZS5strips[0] = self.svdZS5strips.getEntries()
        super().event()
