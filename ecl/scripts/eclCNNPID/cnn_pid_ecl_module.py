#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" This script includes the core of 'CNN_PID_ECL' module

"""

import math
import numpy as np
import torch
import torch.nn.functional as F
from torch.utils.data import DataLoader

import basf2 as b2
from ROOT import Belle2
import modularAnalysis as ma

from cnn_pid_conv_net import ConvNet
from cnn_pid_cluster_image import ClusterImage

__author__ = 'Abtin Narimani Charan'
__copyright__ = 'Copyright 2021 - Belle II Collaboration'
__maintainer__ = 'Abtin Narimani Charan'
__email__ = 'abtin.narimani.charan@desy.de'


class CNN_PID_ECL(b2.Module):
    """ Extracts CNN value for an image

    A model based on convolutional neural network was trained
    on pions and muons' pixel images together with 3 more information
    which were added after convolutional layer. This module uses
    a pretrained model which is used as inference on pions and muons
    in the particle list.

    This module only works under the following conditions:
    1. The extrapolated tracks are inside ECL Barrel based
    on ThetaId of central pixel.
    2. Transverse momentum (pt) of extrapolated tracks are
    in [0.2, 1.0] range.

    The goal is better muon and pion separation in the ECL barrel for
    low pt tracks.
    """

    def __init__(
        self,
        particleList
    ):
        super().__init__()
        self.particleList = particleList

        torch.manual_seed(1234)
        self.device = torch.device('cpu')

    def initialize(self):
        """ Initialize necessary arrays and/or objects """

        self.eclCalDigits = Belle2.PyStoreArray(Belle2.ECLCalDigit.Class(), 'ECLCalDigits')
        self.eclCalDigits.registerInDataStore()

        self.mapping = Belle2.PyStoreObj('ECLCellIdMapping')
        self.mapping.registerInDataStore()

    def event(self):
        """ Event processing

        This function goes through the particleList and find eclcaldigit of
        an extrapolated track. Then it extracts the energy for a 7x7 pixel image
        around the entering position of the track. In addition it extracts
        ThetaId and PhiId of central pixel in a 7x7 image together with Pt of
        the track. Then, it calculates the CNN values for an extrapolated being
        muon- or pion-like.
        """

        pList = Belle2.PyStoreObj(self.particleList)

        for i, particle in enumerate(pList.obj()):

            track = particle.getTrack()

            if track:
                maxCellId = self.getExtCell(track)[0]

                self.pt = self.getExtCell(track)[1]
                self.pt = np.array([self.pt])
                self.charge = particle.getCharge()

                if self.charge == -1.0:
                    self.charge = 'minus'
                elif self.charge == 1.0:
                    self.charge = 'plus'

                self.extThetaId = self.mapping.getCellIdToThetaId(maxCellId)
                self.extThetaId = np.array([self.extThetaId])
                self.extPhiId = self.mapping.getCellIdToPhiId(maxCellId)
                self.extPhiId = np.array([self.extPhiId])

                if maxCellId < 0:
                    b2.B2WARNING('maxCellId is less 0.')
                    return(np.nan, np.nan)
                else:
                    if (self.extThetaId > 13 and
                        self.extThetaId < 58 and
                        self.pt >= 0.2 and
                            self.pt <= 1.0):

                        energy_list = []
                        neighbours = self.mapping.getCellIdToNeighbour7(maxCellId)

                        for posid in range(49):
                            if posid < neighbours.size():
                                neighbourid = neighbours[posid]

                                storearraypos = self.mapping.getCellIdToStoreArray(neighbourid)
                                energy = 0.0
                                if storearraypos >= 0:
                                    energy = self.eclCalDigits[storearraypos].getEnergy()
                                energy_list.append(energy)

                        self.energy_array = np.array(energy_list)

                        prob_CNN_pion, prob_CNN_muon = self.extract_cnn_value()
                        variable_pion = 'cnn_pid_ecl_pion'
                        variable_muon = 'cnn_pid_ecl_muon'

                        particle.addExtraInfo(variable_pion, prob_CNN_pion)
                        particle.addExtraInfo(variable_muon, prob_CNN_muon)
                        b2.B2INFO(f'{variable_pion}: {prob_CNN_pion}')
                        b2.B2INFO(f'{variable_muon}: {prob_CNN_muon}')
                    else:
                        b2.B2WARNING('Track is either outside ECL Barrel or Pt outside [0.2, 1.0] GeV/c. No CNN value.')
                        return(np.nan, np.nan)

    def getExtCell(self, track):
        """ Extract cellId of an extrapolated track """

        myDetID = Belle2.Const.EDetector.ECL
        hypothesis = Belle2.Const.pion
        pdgCode = abs(hypothesis.getPDGCode())

        extHits = track.getRelationsTo('ExtHits')

        for extHit in extHits:
            if abs(extHit.getPdgCode()) != pdgCode:
                continue
            if abs(extHit.getDetectorID()) != myDetID:
                continue
            if abs(extHit.getStatus()) != Belle2.ExtHitStatus.EXT_EXIT:
                continue
            copyid = extHit.getCopyID()
            if copyid == -1:
                continue
            cellid = copyid + 1

            px = extHit.getMomentum().X()
            py = extHit.getMomentum().Y()
            pt = math.sqrt(px**2 + py**2)

            return(cellid, pt)

    def extract_cnn_value(self):
        """ Extract CNN values for an extrapolated track

        The output of this function includes two probabilities:
        1. Probability of an extrapolated track being pion
        2. Probability of an extrapolated track being muon

        The output is a float.
        """

        test_loader = self.prepare_images()
        model = self.read_model()
        model.eval()

        with torch.no_grad():
            for energy, theta, phi, pt in test_loader:

                energy = energy.to(self.device)
                theta = theta.to(self.device)
                phi = phi.to(self.device)
                pt = pt.to(self.device)

                output = model(energy, theta, phi, pt)
                output = F.softmax(output, dim=1)

        prob_CNN_pion = output[0][0].item()
        prob_CNN_muon = output[0][1].item()

        return(prob_CNN_pion, prob_CNN_muon)

    def prepare_images(self):
        """ Prepare images

        A dictionary is passed to a function (ClusterImage)
        in order to prepare proper format for CNN inputs.
        """

        params_image = {
            'image_length': 7,
            'energy_image': self.energy_array,
            'thetaId': self.extThetaId,
            'phiId': self.extPhiId,
            'pt': self.pt,
            'threshold': 0.001
        }

        dataset = ClusterImage(params_image)
        infer_loader = DataLoader(dataset, shuffle=False)

        return(infer_loader)

    def model_cnn_name(self):
        """ Create model name

        The outputs of this function are:
        1. CNN model's name
        2. Models parameters which is important for
        initializing ConvNet.

        Models parameters should be exactly the same as the CNN model
        which was trained.
        """

        model_name = f'CNNModelPidECL_charge_{self.charge}'

        params_model = {
            'input_shape': (1, 7, 7),
            'initial_filters': 64,
            'num_emb_theta': 44,
            'dim_emb_theta': 22,
            'num_emb_phi': 144,
            'dim_emb_phi': 18,
            'num_ext_input': 1,
            'num_fc1': 128,
            'dropout_rate': 0.1,
            'num_classes': 2,
            'epochs': 100,
            'lr': 0.001,
            'patience': 20,
            'energy': True
        }

        return(model_name, params_model)

    def read_model(self):
        """ Load the model

        This function receives model's name and
        CNN parameters, then reads .pt file which
        includes weights and biases for CNN.
        """
        model_name, params_model = self.model_cnn_name()
        model = ConvNet(params_model)
        model = model.to(self.device)

        # The following 3 lines requires to have the payload
        # payload = model_name
        # accessor = Belle2.DBAccessorBase(Belle2.DBStoreEntry.c_RawFile, payload, True)
        # checkpoint = accessor.getFilename()

        model.load_state_dict(torch.load(
            f'/afs/desy.de/user/n/narimani/dust/{model_name}.pt'))

        return(model)
