#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from tqdm import tqdm
import numpy as np
import torch


class Arena():
    """
    An Arena class where two nets merge tracks and performance is compared.
    """

    def __init__(self, player1, player2, valid_data_loader):
        self.player1 = player1
        self.player2 = player2
        self.valid_data_loader = valid_data_loader

    def playGame(self):
        batch = next(self.valid_data_loader)
        vxd_hits, cdc_hits, vxd_tracks, cdc_tracks, true_vxd, true_cdc, true_links = batch

        true_tracks = torch.cat((true_vxd, true_cdc), axis=0)
        true_tracks = true_tracks.data.cpu().numpy().squeeze()
        true_links = true_links.data.cpu().numpy().squeeze()

        pred_links, pred_tracks = self.player1.predict(vxd_hits, cdc_hits, vxd_tracks, cdc_tracks)
        return self.getScore(pred_links, pred_tracks, true_links, true_tracks)

    def playGames(self, num):
        num = int(num / 2)
        scores_1 = []
        scores_2 = []

        for _ in tqdm(range(num), desc="Arena.playGames (1)"):
            gameResult = self.playGame()
            scores_1.append(gameResult)

        self.player1, self.player2 = self.player2, self.player1

        for _ in tqdm(range(num), desc="Arena.playGames (2)"):
            result = self.playGame()
            scores_2.append(result)

        return scores_1, scores_2

    def getScore(self, pred_links, pred_tracks, true_links, true_tracks):
        """
        Returns score of merger on event
        """
        sum_links = np.sum(true_links == pred_links.round())
        sum_tracks = np.sum(true_tracks == pred_tracks.round())

        return 0.5*sum_links/pred_links.shape[0] + 0.5*sum_tracks/pred_tracks.shape[0]
