#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>giulia.casarosa@desy.de</contact>
  <output>ROIFindingValidation.root</output>
  <description>
  This module validates the ROI Finding module.
  </description>
</header>
"""
#   <input>EvtGenSimNoBkg.root</input>

NAME = 'ROIFinding'  # not used?
CONTACT = 'giulia.casarosa@desy.de'
# INPUT_FILE = '../EvtGenSimNoBkg.root' #can't use it because PXDDataReduction in simulated
# INPUT_FILE = 'simRootOutput.root'  # for debugging purposes
OUTPUT_FILE = 'ROIFindingTrackingValidation.root'
N_EVENTS = 1000

ACTIVE = True

from simulation import add_simulation
from svd import add_svd_reconstruction

import basf2
import ROOT
import collections
import math
from ROOT import Belle2

from tracking.validation.plot import ValidationPlot


basf2.set_random_seed(1509)

from tracking import add_tracking_for_PXDDataReduction_simulation


class ROIFindingTrackingValidationPlots(basf2.Module):

    """Module to collect  information about the ROIs (PXDIntercepts & ROIs)  and to
    generate validation plots on the performance of the ROIFinding module."""

    def __init__(
        self,
        name='',
        contact='',
        output_file_name=None,
        nROIs=0
    ):

        super().__init__()
        self.validation_name = NAME
        self.contact = CONTACT
        self.output_file_name = OUTPUT_FILE

        self.nROIs = 0
        self.nPXDDigits = 0
        self.nPXDDigitsIN = 0

        # default binning used for resolution plots over pt
        #        self.resolution_pt_binning = [0.05, 0.1, 0.25, 0.4, 0.6, 1., 1.5, 2., 3., 4.]

    def initialize(self):

        self.nROIs_L1 = collections.deque()
        self.nROIs_L2 = collections.deque()

        self.drf = collections.deque()

        self.statU_L1 = collections.deque()
        self.statV_L1 = collections.deque()
        self.statU_L2 = collections.deque()
        self.statV_L2 = collections.deque()

        self.theta_phi_L1 = collections.deque()

    def event(self):
        ''' ROIs quantities'''
        rois = Belle2.PyStoreArray('ROIs')

        nL1_ROI = 0
        nL2_ROI = 0
        for roi in rois:
            sensor = roi.getSensorID()
            if(sensor.getLayerNumber() == 1):
                nL1_ROI = nL1_ROI + 1
            else:
                nL2_ROI = nL2_ROI + 1

        self.nROIs_L1.append(nL1_ROI)
        self.nROIs_L2.append(nL2_ROI)

        self.nROIs = self.nROIs + rois.getEntries()

        '''Data Reduction Factor '''
        tot_pxd = Belle2.PyStoreArray('PXDDigits')
        in_pxd = Belle2.PyStoreArray('filteredPXDDigits')

        if len(tot_pxd) > 0:
            self.drf.append(len(in_pxd) / len(tot_pxd))

        ''' PXDIntercepts Statistical Error '''
        inters = Belle2.PyStoreArray('PXDIntercepts')
        for inter in inters:
            sensor = Belle2.VxdID(inter.getSensorID())
            if(sensor.getLayerNumber() == 1):
                self.statU_L1.append(inter.getSigmaU())
                self.statV_L1.append(inter.getSigmaV())
            else:
                self.statU_L2.append(inter.getSigmaU())
                self.statV_L2.append(inter.getSigmaV())

    def terminate(self):

        name = self.validation_name
        contact = self.contact
        basf2.B2RESULT("total nROIs = {}".format(self.nROIs))

        ''' Saving'''

        output_tfile = ROOT.TFile(self.output_file_name, 'recreate')

        h_nROIs_L1 = ValidationPlot('h_nROIs_L1')
        h_nROIs_L1.hist(self.nROIs_L1, bins=100, lower_bound=0, upper_bound=100)
        h_nROIs_L1.contact = contact
        h_nROIs_L1.check = 'average of 25 +/- 10 is expected'
        h_nROIs_L1.description = 'ROIs on L1'
        h_nROIs_L1.title = 'ROIs on L1'
        h_nROIs_L1.xlabel = 'number of ROIs'
        h_nROIs_L1.ylabel = ''
        h_nROIs_L1.write(output_tfile)

        h_nROIs_L2 = ValidationPlot('h_nROIs_L2')
        h_nROIs_L2.hist(self.nROIs_L2, bins=100, lower_bound=0, upper_bound=100)
        h_nROIs_L2.contact = contact
        h_nROIs_L2.check = 'average of 25 +/- 10 is expected'
        h_nROIs_L2.description = 'ROIs on L2'
        h_nROIs_L2.title = 'ROIs on L2'
        h_nROIs_L2.xlabel = 'number of ROIs'
        h_nROIs_L2.ylabel = ''
        h_nROIs_L2.write(output_tfile)

        h_drf = ValidationPlot('h_drf')
        h_drf.hist(self.drf, bins=100, lower_bound=0, upper_bound=1)
        h_drf.contact = contact
        h_drf.check = 'with no bkg, the average should be around 70%'
        h_drf.description = 'Fraction of PXDDDigits inside ROIs'
        h_drf.title = 'Fraction of PXDDDigits inside ROIs'
        h_drf.xlabel = 'DRF'
        h_drf.ylabel = ''
        h_drf.write(output_tfile)

        h_statU_L1 = ValidationPlot('h_statU_L1')
        h_statU_L1.hist(self.statU_L1, bins=100, lower_bound=0, upper_bound=0.2)
        h_statU_L1.contact = contact
        h_statU_L1.check = 'average should be around 0.021 cm'
        h_statU_L1.description = 'Statistical Error of the Intercept on L1 planes, along U'
        h_statU_L1.title = 'L1 Intercept Statistical Uncertainity along U'
        h_statU_L1.xlabel = 'U stat uncertainity'
        h_statU_L1.ylabel = ''
        h_statU_L1.write(output_tfile)

        h_statV_L1 = ValidationPlot('h_statV_L1')
        h_statV_L1.hist(self.statV_L1, bins=100, lower_bound=0, upper_bound=0.2)
        h_statV_L1.contact = contact
        h_statV_L1.check = 'average should be around 0.021 cm'
        h_statV_L1.description = 'Statistical Error of the Intercept on L1 planes, along V'
        h_statV_L1.title = 'L1 Intercept Statistical Uncertainity along V'
        h_statV_L1.xlabel = 'V stat uncertainity (cm)'
        h_statV_L1.ylabel = ''
        h_statV_L1.write(output_tfile)

        h_statU_L2 = ValidationPlot('h_statU_L2')
        h_statU_L2.hist(self.statU_L2, bins=100, lower_bound=0, upper_bound=0.2)
        h_statU_L2.contact = contact
        h_statU_L2.check = 'average should be around  0.015 cm'
        h_statU_L2.description = 'Statistical Error of the Intercept on L2 planes, along U'
        h_statU_L2.title = 'L2 Intercept Statistical Uncertainity along U'
        h_statU_L2.xlabel = 'U stat uncertainity (cm)'
        h_statU_L2.ylabel = ''
        h_statU_L2.write(output_tfile)

        h_statV_L2 = ValidationPlot('h_statV_L2')
        h_statV_L2.hist(self.statV_L2, bins=100, lower_bound=0, upper_bound=0.2)
        h_statV_L2.contact = contact
        h_statV_L2.check = 'average should be around 0.016 cm'
        h_statV_L2.description = 'Statistical Error of the Intercept on L2 planes, along V'
        h_statV_L2.title = 'L2 Intercept Statistical Uncertainity along V'
        h_statV_L2.xlabel = 'V stat uncertainity (cm)'
        h_statV_L2.ylabel = ''
        h_statV_L2.write(output_tfile)

        output_tfile.Close()


""" Steering """

path = basf2.create_path()

path.add_module('EventInfoSetter', evtNumList=N_EVENTS)
path.add_module('EvtGenInput')
add_simulation(path, usePXDDataReduction=False)
add_svd_reconstruction(path, isROIsimulation=True)

#    path.add_module('RootInput', inputFileName=INPUT_FILE, entrySequences=["0:{}".format(N_EVENTS-1)])
# path.add_module('Gearbox')
# path.add_module('Geometry')

pxd_unfiltered_digits = 'PXDDigits'
pxd_filtered_digits = 'filteredPXDDigits'

# SVD tracking
svd_reco_tracks = '__ROIsvdRecoTracks'
add_tracking_for_PXDDataReduction_simulation(path, ['SVD', 'CDC'], True, '__ROIsvdClusters')  # CDC is not used at the moment!

# ROI Finding
pxdDataRed = basf2.register_module('PXDROIFinder')
param_pxdDataRed = {
    'recoTrackListName': svd_reco_tracks,
    'PXDInterceptListName': 'PXDIntercepts',
    'ROIListName': 'ROIs',
    'tolerancePhi': 0.15,
    'toleranceZ': 0.5,
    'sigmaSystU': 0.02,
    'sigmaSystV': 0.02,
    'numSigmaTotU': 10,
    'numSigmaTotV': 10,
    'maxWidthU': 0.5,
    'maxWidthV': 0.5,
}
pxdDataRed.param(param_pxdDataRed)
path.add_module(pxdDataRed)

# Filtering of PXDDigits
pxd_digifilter = basf2.register_module('PXDdigiFilter')
pxd_digifilter.param('ROIidsName', 'ROIs')
pxd_digifilter.param('PXDDigitsName', pxd_unfiltered_digits)
pxd_digifilter.param('PXDDigitsInsideROIName', pxd_filtered_digits)
pxd_digifilter.param('PXDDigitsOutsideROIName', 'PXDDigitsOutside')
path.add_module(pxd_digifilter)

ROIValidationPlots = ROIFindingTrackingValidationPlots()
path.add_module(ROIValidationPlots)

path.add_module('Progress')

if ACTIVE:
    basf2.process(path)

    print(path)
