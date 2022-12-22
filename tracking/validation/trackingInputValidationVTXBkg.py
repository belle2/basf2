#!/usr/bin/env python3

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
  <input>VTXEvtGenSim.root</input>
  <output>TrackingInputValidationVTXBkg.root</output>
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
from vtx import add_vtx_reconstruction
NAME = 'Tracking Input Validation'  # not used?
CONTACT = 'software-tracking@belle2.org'
INPUT_FILE = '../VTXEvtGenSim.root'
OUTPUT_FILE = 'TrackingInputValidationVTXBkg.root'
N_EVENTS = 1000

ACTIVE = True


basf2.set_random_seed(1509)


class TrackingInputValidationVTX(basf2.Module):
    """
    Module to collect information about the number of
    * VTXDigits
    * VTXClusters
    * VTXSpacePoints
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

        #: StoreArray containing the VTXDigits
        self.VTXDigits = Belle2.PyStoreArray('VTXDigits')
        #: StoreArray containing the VTXClusters
        self.VTXClusters = Belle2.PyStoreArray('VTXClusters')
        #: StoreArray containing the VTXSpacePoints
        self.VTXSpacePoints = Belle2.PyStoreArray('VTXSpacePoints')

        #: StoreArray containing the CDCHits
        self.CDCHits = Belle2.PyStoreArray('CDCHits')

        #: list of the number of VTXDigits
        self.nVTXDigits = collections.deque()
        #: list of the number of VTXClusters
        self.nVTXClusters = collections.deque()
        #: list of the number of VTXSpacePoints
        self.nVTXSpacePoints = collections.deque()

        #: list of the number of CDCHits
        self.nCDCHits = collections.deque()

    def event(self):
        '''Event function'''

        self.nVTXDigits.append(self.VTXDigits.getEntries())
        self.nVTXClusters.append(self.VTXClusters.getEntries())
        self.nVTXSpacePoints.append(self.VTXSpacePoints.getEntries())
        self.nCDCHits.append(self.CDCHits.getEntries())

    def terminate(self):
        """Receive signal at the end of event processing"""

        ''' Saving'''
        output_tfile = ROOT.TFile(self.output_file_name, 'recreate')

        h_nVTXDigits = ValidationPlot('h_nVTXDigits')
        h_nVTXDigits.hist(self.nVTXDigits, bins=100, lower_bound=0, upper_bound=2000)
        h_nVTXDigits.contact = self.contact
        h_nVTXDigits.check = 'Average of 800 +/- 100 is expected.'
        h_nVTXDigits.description = 'Number of VTXDigits'
        h_nVTXDigits.title = 'Number of VTXDigits per event'
        h_nVTXDigits.xlabel = 'Number of VTXDigits'
        h_nVTXDigits.ylabel = ''
        h_nVTXDigits.write(output_tfile)

        h_nVTXClusters = ValidationPlot('h_nVTXClusters')
        h_nVTXClusters.hist(self.nVTXClusters, bins=100, lower_bound=0, upper_bound=500)
        h_nVTXClusters.contact = self.contact
        h_nVTXClusters.check = 'Average of 250 +/- 50 with a std of 50 is expected.'
        h_nVTXClusters.description = 'Number of VTXClusters'
        h_nVTXClusters.title = 'Number of VTXClusters per event'
        h_nVTXClusters.xlabel = 'Number of VTXClusters'
        h_nVTXClusters.ylabel = ''
        h_nVTXClusters.write(output_tfile)

        h_nVTXSpacePoints = ValidationPlot('h_nVTXSpacePoints')
        h_nVTXSpacePoints.hist(self.nVTXSpacePoints, bins=100, lower_bound=0, upper_bound=500)
        h_nVTXSpacePoints.contact = self.contact
        h_nVTXSpacePoints.check = 'Average of 250 +/- 50 with a std of 50 is expected'
        h_nVTXSpacePoints.description = 'Number of VTXSpacePoints. Should be the same as the number of VTXClusters.'
        h_nVTXSpacePoints.title = 'Number of VTXSpacePoints per event'
        h_nVTXSpacePoints.xlabel = 'Number of VTXSpacePoints'
        h_nVTXSpacePoints.ylabel = ''
        h_nVTXSpacePoints.write(output_tfile)

        h_nCDCHits = ValidationPlot('h_nCDCHits')
        h_nCDCHits.hist(self.nCDCHits, bins=100, lower_bound=3000, upper_bound=6000)
        h_nCDCHits.contact = self.contact
        h_nCDCHits.check = 'Average of 4000 +/- 200 with a std of around 200 is expected.'
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
add_vtx_reconstruction(path)

path.add_module(TrackingInputValidationVTX())

path.add_module('Progress')


if ACTIVE:
    print(path)
    basf2.process(path)
