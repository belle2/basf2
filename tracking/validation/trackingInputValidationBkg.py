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
<header>
  <contact>software-tracking@belle2.org</contact>
  <input>EvtGenSim.root</input>
  <output>TrackingInputValidationBkg.root</output>
  <description>
  This module checks the number of input hits for tracking.
  </description>
</header>
"""

from tracking.validation.plot import ValidationPlot
from ROOT import Belle2
import collections
import ROOT
import basf2
from svd import add_svd_reconstruction
from pxd import add_pxd_reconstruction
NAME = 'Tracking Input Validation'  # not used?
CONTACT = 'software-tracking@belle2.org'
INPUT_FILE = '../EvtGenSim.root'
OUTPUT_FILE = 'TrackingInputValidationBkg.root'
N_EVENTS = 1000

ACTIVE = True


basf2.set_random_seed(1509)


class TrackingInputValidation(basf2.Module):
    """
    Module to collect information about the number of
    * PXDDigits
    * PXDClusters
    * PXDSpacePoints
    * SVDShaperDigits
    * SVDClusters
    * SVDSpacePoints
    * CDCHits
    """

    def __init__(
        self,
        name='',
        contact='',
        output_file_name=None,
    ):
        """Constructor"""

        super().__init__()
        #: name of this validation output
        self.validation_name = NAME
        #: contact person
        self.contact = CONTACT
        #: #: name of the output ROOT file
        self.output_file_name = OUTPUT_FILE

    def initialize(self):
        """Receive signal at the start of event processing"""

        #: StoreArray containing the PXDDigits
        self.PXDDigits = Belle2.PyStoreArray('PXDDigits')
        #: StoreArray containing the PXDClusters
        self.PXDClusters = Belle2.PyStoreArray('PXDClusters')
        #: StoreArray containing the PXDSpacePoints
        self.PXDSpacePoints = Belle2.PyStoreArray('PXDSpacePoints')

        #: StoreArray containing the SVDShaperDigits
        self.SVDShaperDigits = Belle2.PyStoreArray('SVDShaperDigits')
        #: StoreArray containing the SVDClusters
        self.SVDClusters = Belle2.PyStoreArray('SVDClusters')
        #: StoreArray containing the SVDSpacePoints
        self.SVDSpacePoints = Belle2.PyStoreArray('SVDSpacePoints')

        #: StoreArray containing the CDCHits
        self.CDCHits = Belle2.PyStoreArray('CDCHits')

        #: list of the number of PXDDigits
        self.nPXDDigits = collections.deque()
        #: list of the number of PXDClusters
        self.nPXDClusters = collections.deque()
        #: list of the number of PXDSpacePoints
        self.nPXDSpacePoints = collections.deque()

        #: list of the number of SVDShaperDigits
        self.nSVDShaperDigits = collections.deque()
        #: list of the number of SVDClusters
        self.nSVDClusters = collections.deque()
        #: list of the number of SVDSpacePoints
        self.nSVDSpacePoints = collections.deque()

        #: list of the number of CDCHits
        self.nCDCHits = collections.deque()

    def event(self):
        '''Event function'''

        self.nPXDDigits.append(self.PXDDigits.getEntries())
        self.nPXDClusters.append(self.PXDClusters.getEntries())
        self.nPXDSpacePoints.append(self.PXDSpacePoints.getEntries())
        self.nSVDShaperDigits.append(self.SVDShaperDigits.getEntries())
        self.nSVDClusters.append(self.SVDClusters.getEntries())
        self.nSVDSpacePoints.append(self.SVDSpacePoints.getEntries())
        self.nCDCHits.append(self.CDCHits.getEntries())

    def terminate(self):
        """Receive signal at the end of event processing"""

        ''' Saving'''
        output_tfile = ROOT.TFile(self.output_file_name, 'recreate')

        h_nPXDDigits = ValidationPlot('h_nPXDDigits')
        h_nPXDDigits.hist(self.nPXDDigits, bins=100, lower_bound=0, upper_bound=1000)
        h_nPXDDigits.contact = self.contact
        h_nPXDDigits.check = 'Average of 250 +/- 100 is expected'
        h_nPXDDigits.description = 'Number of PXDDigits after ROI filtering'
        h_nPXDDigits.title = 'Number of selected PXDDigits per event'
        h_nPXDDigits.xlabel = 'Number of PXDDigits'
        h_nPXDDigits.ylabel = ''
        h_nPXDDigits.write(output_tfile)

        h_nPXDClusters = ValidationPlot('h_nPXDClusters')
        h_nPXDClusters.hist(self.nPXDClusters, bins=100, lower_bound=0, upper_bound=500)
        h_nPXDClusters.contact = self.contact
        h_nPXDClusters.check = 'Average of 75 +/- 20 is expected'
        h_nPXDClusters.description = 'Number of PXDClusters after ROI filtering'
        h_nPXDClusters.title = 'Number of PXDClusters per event'
        h_nPXDClusters.xlabel = 'Number of PXDClusters'
        h_nPXDClusters.ylabel = ''
        h_nPXDClusters.write(output_tfile)

        h_nPXDSpacePoints = ValidationPlot('h_nPXDSpacePoints')
        h_nPXDSpacePoints.hist(self.nPXDSpacePoints, bins=100, lower_bound=0, upper_bound=500)
        h_nPXDSpacePoints.contact = self.contact
        h_nPXDSpacePoints.check = 'Average of 75 +/- 20 is expected'
        h_nPXDSpacePoints.description = 'Number of PXDSpacePoints after ROI filtering. \
                                         Should be the same as the number of PXDClusters'
        h_nPXDSpacePoints.title = 'Number of PXDSpacePoints per event'
        h_nPXDSpacePoints.xlabel = 'Number of PXDSpacePoints'
        h_nPXDSpacePoints.ylabel = ''
        h_nPXDSpacePoints.write(output_tfile)

        h_nSVDShaperDigits = ValidationPlot('h_nSVDShaperDigits')
        h_nSVDShaperDigits.hist(self.nSVDShaperDigits, bins=100, lower_bound=0, upper_bound=10000)
        h_nSVDShaperDigits.contact = self.contact
        h_nSVDShaperDigits.check = 'First peak at about 3200 +/- 200 is expected, with a second peak round 5500'
        h_nSVDShaperDigits.description = 'Number of SVDDigits'
        h_nSVDShaperDigits.title = 'Number of SVDDigits per event'
        h_nSVDShaperDigits.xlabel = 'Number of SVDDigits'
        h_nSVDShaperDigits.ylabel = ''
        h_nSVDShaperDigits.write(output_tfile)

        h_nSVDClusters = ValidationPlot('h_nSVDClusters')
        h_nSVDClusters.hist(self.nSVDClusters, bins=100, lower_bound=0, upper_bound=2000)
        h_nSVDClusters.contact = self.contact
        h_nSVDClusters.check = 'Average of 500 +/- 100 is expected'
        h_nSVDClusters.description = 'Number of SVDClusters'
        h_nSVDClusters.title = 'Number of SVDClusters per event'
        h_nSVDClusters.xlabel = 'Number of SVDClusters'
        h_nSVDClusters.ylabel = ''
        h_nSVDClusters.write(output_tfile)

        h_nSVDSpacePoints = ValidationPlot('h_nSVDSpacePoints')
        h_nSVDSpacePoints.hist(self.nSVDSpacePoints, bins=100, lower_bound=0, upper_bound=2000)
        h_nSVDSpacePoints.contact = self.contact
        h_nSVDSpacePoints.check = 'Average of 200 +/- 50 is expected'
        h_nSVDSpacePoints.description = 'Number of SVDSpacePoints'
        h_nSVDSpacePoints.title = 'Number of SVDSpacePoints per event'
        h_nSVDSpacePoints.xlabel = 'Number of SVDSpacePoints'
        h_nSVDSpacePoints.ylabel = ''
        h_nSVDSpacePoints.write(output_tfile)

        h_nCDCHits = ValidationPlot('h_nCDCHits')
        h_nCDCHits.hist(self.nCDCHits, bins=100, lower_bound=0, upper_bound=5000)
        h_nCDCHits.contact = self.contact
        h_nCDCHits.check = 'Average of 3000 +/- 200 is expected'
        h_nCDCHits.description = 'Number of CDCHits'
        h_nCDCHits.title = 'Number of CDCHits per event'
        h_nCDCHits.xlabel = 'Number of CDCHits'
        h_nCDCHits.ylabel = ''
        h_nCDCHits.write(output_tfile)

        output_tfile.Close()


""" Steering """

path = basf2.create_path()

path.add_module('RootInput', inputFileName=INPUT_FILE)
path.add_module('Gearbox')
path.add_module('Geometry')
add_svd_reconstruction(path, isROIsimulation=False)
add_pxd_reconstruction(path)
path.add_module('PXDSpacePointCreator')

path.add_module(TrackingInputValidation())

path.add_module('Progress')


if ACTIVE:
    print(path)
    basf2.process(path)
