#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import math
import numpy as np
import torch
import torch.nn.functional as F
from torch.utils.data import DataLoader

import basf2 as b2
from ROOT import Belle2
import modularAnalysis as ma

from conv_net import ConvNet
from cluster_image import ClusterImage

__author__ = 'Abtin Narimani Charan'
__copyright__ = 'Copyright 2021 - Belle II Collaboration'
__maintainer__ = 'Abtin Narimani Charan'
__email__ = 'abtin.narimani.charan@desy.de'
__updated__ = '29.06.2021'

variableManager = Belle2.Variable.Manager.Instance()


class CNN_PID_ECL(b2.Module):
    ''' Extract CNN value for an image '''

    def __init__(
        self,
        particleList,
        varibale
    ):
        super().__init__()
        self.particleList = particleList
        self.varibale = varibale

        torch.manual_seed(1234)
        self.device = torch.device('cpu')
        self.kwargs = {}

    def initialize(self):
        ''' initialization '''

        #: ECLCalDigits datastore
        eclCalDigits = Belle2.PyStoreArray(Belle2.ECLCalDigit.Class(), 'ECLCalDigits')
        eclCalDigits.registerInDataStore()

        eventExtraInfo = Belle2.PyStoreObj('EventExtraInfo')
        eventExtraInfo.registerInDataStore()

    def event(self):
        ''' event processing '''

        # Get the particle list of the event
        pList = Belle2.PyStoreObj(self.particleList)

        eventExtraInfo = Belle2.PyStoreObj('EventExtraInfo')
        if not eventExtraInfo.isValid():
            eventExtraInfo.create()

        mapping = Belle2.PyStoreObj('ECLCellIdMapping')
        Particles = Belle2.PyStoreArray('Particles')
        eclCalDigits = Belle2.PyStoreArray('ECLCalDigits')

        for i, particle in enumerate(pList.obj()):
            track = particle.getTrack()
            maxCellId = self.getExtCell(track)[0]

            self.pt = self.getExtCell(track)[1]
            self.pt = np.array(self.pt)
            self.charge = self.getExtCell(track)[2]

            self.extThetaId = mapping.getCellIdToThetaId(maxCellId)
            self.extThetaId = np.array(self.extThetaId)
            self.extPhiId = mapping.getCellIdToPhiId(maxCellId)
            self.extPhiId = np.array(self.extPhiId)

            if maxCellId < 0:
                print('maxCellId is less 0.')
            else:
                if (self.extThetaId > 13 and
                    self.extThetaId < 58 and
                    self.pt >= 0.2 and
                        self.pt <= 1.0):

                    energy_list = []
                    neighbours = mapping.getCellIdToNeighbour7(maxCellId)

                    for posid in range(49):
                        if posid < neighbours.size():
                            neighbourid = neighbours[posid]

                            storearraypos = mapping.getCellIdToStoreArray(neighbourid)
                            energy = 0.0
                            if storearraypos >= 0:
                                energy = eclCalDigits[storearraypos].getEnergy()
                            energy_list.append(energy)

                    self.energy_array = np.array(energy_list).reshape(7, 7)

                    prob_CNN_pion, prob_CNN_muon = self.extract_cnn_value()
                    value = variableManager.evaluate(self.variable, particle)
                    eventExtraInfo.addExtraInfo(cur_var, value)
                else:
                    return(np.nan, np.nan)
                    print('[INFO] Track is either outside ECL Barrel or Pt outside [0.2, 1.0] GeV/c. No CNN value.')

    def getExtCell(self, track):

        myDetID = Belle2.Const.EDetector.ECL
        hypothesis = Belle2.Const.pion
        pdgCode = abs(hypothesis.getPDGCode())

        extHits = track.getRelationsTo('ExtHits')
        for extHit in extHits:
            if abs(extHit.getPdgCode()) != pdgCode:
                continue
            if abs(extHit.getDetectorID()) != myDetID:
                continue
            if abs(extHit.getStatus()) != Belle2.ExtHitStatus.EXT_ENTER:
                continue
            copyid = extHit.getCopyID()
            if copyid == -1:
                continue
            cellid = copyid + 1

            px = extHit.getMomentum().X()
            py = extHit.getMomentum().Y()
            pt = math.sqrt(px**2 + py**2)

            charge = '0'
            if (extHit.getPdgCode() == -211 or extHit.getPdgCode() == 13):
                charge = 'minus'
            elif (extHit.getPdgCode() == 211 or extHit.getPdgCode() == -13):
                charge = 'plus'

            return(cellid, pt, charge)

    def extract_cnn_value(self):

        test_loader = self.prepare_images()
        model = self.read_model()
        model.eval()
        prob_CNN_list = []

        with torch.no_grad():
            for energy, theta, phi, extra in test_loader:

                energy = energy.to(self.device)
                theta = theta.to(self.device)
                phi = phi.to(self.device)
                extra = extra.to(self.device)

                output = model(energy, theta, phi, extra)

                output = F.softmax(output, dim=1)
                prob_CNN_list.append(output)

        total_prob_CNN = torch.cat(prob_CNN_list, dim=0)
        np_total_prob_CNN = total_prob_CNN.cpu().numpy()
        prob_CNN_pion = np_total_prob_CNN[0]
        prob_CNN_muon = np_total_prob_CNN[1]

        return(prob_CNN_pion, prob_CNN_muon)

    def prepare_images(self):

        params_image = {
            'image_length': 7,
            'energy_image': self.energy_array,
            'thetaId': self.extThetaId,
            'phiId': self.extPhiId,
            'pt': self.pt,
            'charge': self.charge,
            'threshold': 0.001
        }

        dataset = ClusterImage(params_image)

        infer_loader = DataLoader(
            dataset,
            shuffle=False,
            **self.kwargs
        )

        return(infer_loader)

    def model_cnn_name(self):

        model_name = f'model_charge_{self.charge}'

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
            'energy': True,
        }

        return(model_name, params_model)

    def read_model(self):

        model_name, params_model = self.model_cnn_name()
        model = ConvNet(params_model)
        model = model.to(self.device)
        model.load_state_dict(
            torch.load(
                f'./{model_name}.pt', map_location=lambda storage, loc: storage
            )
        )

        return(model)
