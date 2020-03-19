# !/usr/bin/env python
# -*- coding: utf-8 -*-

##
# contributors: G. Casarosa, T. Lueck
##

######
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
######

from basf2 import *
from ROOT import Belle2
import numpy as np
from per_event_statistics import PerEventStatisticsGetterModule


class SVDExtraEventStatisticsModule(PerEventStatisticsGetterModule):

    def __init__(self, filename):
        super().__init__(filename)
        self.svdSPs = Belle2.PyStoreArray("SVDSpacePoints")
        self.svdclusters = Belle2.PyStoreArray("SVDClusters")
        self.svdstrips = Belle2.PyStoreArray("SVDShaperDigits")
        self.svdZS5strips = Belle2.PyStoreArray("SVDShaperDigitsZS5")

    def initialize(self):
        super().initialize()
        # an array needed to assign the actual values, can be extended with further information
        self.svd_sps = np.zeros(1, dtype=np.int32)
        self.svd_clusters = np.zeros(1, dtype=np.int32)
        self.svd_strips = np.zeros(1, dtype=np.int32)
        self.svd_ZS5strips = np.zeros(1, dtype=np.int32)

        # assign the branch address to the first field of the array
        self.statistics.Branch('svdSPs', self.svd_clusters[0:], "svdSPs/I")
        self.statistics.Branch('svdClusters', self.svd_clusters[0:], "svdClusters/I")
        self.statistics.Branch('svdStrips', self.svd_strips[0:], "svdStrips/I")
        self.statistics.Branch('svdZS5strips', self.svd_ZS5strips[0:], "svdZS5strips/I")

        # register the StoreArray for the SVD clusters
        self.svdSPs.isRequired()
        self.svdclusters.isRequired()
        self.svdstrips.isRequired()
        self.svdZS5strips.isRequired()

    def event(self):
        self.svd_sps[0] = self.svdSPs.getEntries()

        self.svd_clusters[0] = self.svdclusters.getEntries()
        self.svd_strips[0] = self.svdstrips.getEntries()
        self.svd_ZS5strips[0] = self.svdZS5strips.getEntries()
        print("number of SP = "+str(self.svd_sps[0])+" and clusters = "+str(self.svd_clusters[0]))
        super().event()
