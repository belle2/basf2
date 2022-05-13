#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import os
import numpy as np
from tqdm import tqdm

from vtx_cdc_merger.utils import AverageMeter

import torch
import torch.optim as optim

from .MergerNNet import MergerNNet


class NNetWrapper():
    def __init__(self):
        self.nnet = MergerNNet(input_dim=64, hidden_dim=64,
                               hidden_activation='ReLU', layer_norm=True)

    def prepare_inputs(self, vxd_hits, vxd_tracks, cdc_hits, cdc_tracks, skip_unpack=True):

        if skip_unpack:
            # Unpack tuple
            vxd_trackids = vxd_hits[:, 0].long()
            vxd_hits = vxd_hits[:, 1:]
            vxd_tracks = vxd_tracks
            cdc_trackids = cdc_hits[:, 0].long()
            cdc_hits = cdc_hits[:, 1:]
            cdc_tracks = cdc_tracks
        else:
            vxd_trackids = vxd_hits[0][:, 0].long()
            vxd_hits = vxd_hits[0][:, 1:]
            vxd_tracks = vxd_tracks[0]
            cdc_trackids = cdc_hits[0][:, 0].long()
            cdc_hits = cdc_hits[0][:, 1:]
            cdc_tracks = cdc_tracks[0]

        # Get correct dtype
        vxd_hits = torch.FloatTensor(vxd_hits)
        vxd_trackids = torch.LongTensor(vxd_trackids)
        vxd_tracks = torch.FloatTensor(vxd_tracks)
        cdc_hits = torch.FloatTensor(cdc_hits)
        cdc_trackids = torch.LongTensor(cdc_trackids)
        cdc_tracks = torch.FloatTensor(cdc_tracks)

        return vxd_hits, vxd_trackids, vxd_tracks, cdc_hits, cdc_trackids, cdc_tracks

    def prepare_targets(self, true_vxd, true_cdc, true_links):

        # Unpack tuple
        true_vxd = true_vxd[0]
        true_cdc = true_cdc[0]
        true_links = true_links[0]

        # Get correct dtype
        true_vxd = torch.FloatTensor(true_vxd)
        true_cdc = torch.FloatTensor(true_cdc)
        true_links = torch.FloatTensor(true_links)

        return true_vxd, true_cdc, true_links

    def train(self, examples, lr, epochs, batch_size):
        """
        examples: list of examples, each example is of form (x, y)
        """
        optimizer = optim.Adam(self.nnet.parameters(), lr=lr)

        loss_fn = torch.nn.BCELoss()

        for epoch in range(epochs):
            print('EPOCH ::: ' + str(epoch + 1))
            self.nnet.train()
            trig_losses = AverageMeter()

            batch_count = int(len(examples) / batch_size)

            t = tqdm(range(batch_count), desc='Training Net')
            for _ in t:
                sample_ids = np.random.randint(len(examples), size=batch_size)
                vxd_hits, vxd_tracks, cdc_hits, cdc_tracks, true_vxd, true_cdc, true_links = list(
                    zip(*[examples[i] for i in sample_ids]))

                vxd_hits, vxd_trackids, vxd_tracks, cdc_hits, cdc_trackids, cdc_tracks = self.prepare_inputs(
                    vxd_hits, vxd_tracks, cdc_hits, cdc_tracks, skip_unpack=False
                )

                true_vxd, true_cdc, true_links = self.prepare_targets(true_vxd, true_cdc, true_links)

                # Compute output
                output_e, output_a = self.nnet(vxd_hits, vxd_trackids, vxd_tracks, cdc_hits, cdc_trackids, cdc_tracks)

                true_tracks = torch.cat((true_vxd, true_cdc), 0)

                # calculate loss
                loss = loss_fn(output_e, true_links.squeeze()) + loss_fn(output_a, true_tracks.squeeze())

                # record loss
                trig_losses.update(loss.item())
                t.set_postfix(Loss_trig=trig_losses)

                # compute gradient and do SGD step
                optimizer.zero_grad()
                loss.backward()
                optimizer.step()

    def predict(self, vxd_hits, cdc_hits, vxd_tracks, cdc_tracks):

        # preparing input
        vxd_hits, vxd_trackids, vxd_tracks, cdc_hits, cdc_trackids, cdc_tracks = self.prepare_inputs(
            vxd_hits, vxd_tracks, cdc_hits, cdc_tracks
        )

        self.nnet.eval()
        with torch.no_grad():
            output_e, output_a = self.nnet(vxd_hits, vxd_trackids, vxd_tracks, cdc_hits, cdc_trackids, cdc_tracks)

        return output_e.data.cpu().numpy(), output_a.cpu().numpy()

    def save_checkpoint(self, folder='checkpoint', filename='checkpoint.pth.tar'):
        folder = os.path.expandvars(folder)
        filepath = os.path.join(folder, filename)
        if not os.path.exists(folder):
            print("Checkpoint Directory does not exist! Making directory {}".format(folder))
            os.mkdir(folder)
        else:
            print("Checkpoint Directory exists! ")
        torch.save({
            'state_dict': self.nnet.state_dict(),
        }, filepath)

    def load_checkpoint(self, folder='checkpoint', filename='checkpoint.pth.tar'):
        filepath = os.path.join(folder, filename)
        filepath = os.path.expandvars(filepath)
        if not os.path.exists(filepath):
            raise ("No model in path {}".format(filepath))
        map_location = 'cpu'
        checkpoint = torch.load(filepath, map_location=map_location)
        self.nnet.load_state_dict(checkpoint['state_dict'])
