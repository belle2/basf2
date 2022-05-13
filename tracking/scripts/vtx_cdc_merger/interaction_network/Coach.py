#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import logging
import os
import sys
import time
from collections import deque
from pickle import Pickler, Unpickler
from random import shuffle
from itertools import cycle

import numpy as np
import pandas as pd
from tqdm import tqdm

from vtx_cdc_merger.interaction_network.Arena import Arena

lr = 0.001
epochs = 1

arena_games = 50
maxlenOfQueue = 200000
numEps = 100
numItersForTrainExamplesHistory = 2


class Coach():
    """
    This class executes the learning. It uses the functions defined
    in Game and NeuralNet. args are specified in main.py.
    """

    def __init__(self, nnet, train_data_loader, valid_data_loader, checkpoint, numIters):
        self.nnet = nnet
        self.pnet = self.nnet.__class__()  # the competitor network
        self.trainExamplesHistory = []  # history of examples from args.numItersForTrainExamplesHistory latest iterations
        self.skipFirstSelfPlay = False  # can be overriden in loadTrainExamples()

        self.checkpoint = os.path.expandvars(checkpoint)
        self.summaries = None
        self.summary_file = None
        self.numIters = numIters
        self.train_data_loader = cycle(train_data_loader)
        self.valid_data_loader = cycle(valid_data_loader)

    def executeEpisode(self):
        """
        This function plays one episode, computes trigger for on one event (==100ns of data with VTX hits).
        As the game is played, each turn is added as a training example to
        trainExamples. The game is played till the game ends. After the game
        ends, the outcome of the game is used to assign values to each example
        in trainExamples.

        It uses a temp=1 if episodeStep < tempThreshold, and thereafter
        uses temp=0.

        Returns:
            trainExamples: a list of examples of the form (x1,.., xn, y)
                           x is batch of hit embeddings and y is trigger value (truth)
        """
        trainExamples = []

        batch = next(self.train_data_loader)

        vxd_hits, cdc_hits, vxd_tracks, cdc_tracks, true_vxd, true_cdc, true_links = batch

        trainExamples.append((vxd_hits, vxd_tracks, cdc_hits, cdc_tracks, true_vxd, true_cdc, true_links))
        return trainExamples

    def learn(self):
        """
        Performs numIters iterations with numEps episodes of self-play in each
        iteration. After every iteration, it retrains neural network with
        examples in trainExamples (which has a maximum length of maxlenofQueue)
        """

        for i in range(1, self.numIters + 1):
            # bookkeeping
            logging.info(f'Starting Iter #{i} ...')

            # prepare summary information
            summary = dict()
            sum_examples = 0

            # start with self play to create examples
            start_time = time.time()
            if not self.skipFirstSelfPlay or i > 1:
                iterationTrainExamples = deque([], maxlen=maxlenOfQueue)

                for _ in tqdm(range(numEps), desc="Self Play"):
                    episodeExamples = self.executeEpisode()
                    sum_examples += len(episodeExamples)
                    iterationTrainExamples += episodeExamples

                # save the iteration examples to the history
                self.trainExamplesHistory.append(iterationTrainExamples)

            if len(self.trainExamplesHistory) > numItersForTrainExamplesHistory:
                logging.warning(
                    f"Removing the oldest entry in trainExamples. len(trainExamplesHistory) = {len(self.trainExamplesHistory)}")
                self.trainExamplesHistory.pop(0)

            summary['play_time'] = time.time() - start_time

            # shuffle examples before training
            trainExamples = []
            for e in self.trainExamplesHistory:
                trainExamples.extend(e)
            shuffle(trainExamples)

            # save current state of new network
            self.nnet.save_checkpoint(folder=self.checkpoint, filename='temp.pth.tar')
            # set the prev network to state of new network
            self.pnet.load_checkpoint(folder=self.checkpoint, filename='temp.pth.tar')

            # training of the network
            start_time = time.time()
            self.nnet.train(trainExamples,
                            lr=lr,
                            epochs=epochs,
                            batch_size=1)
            summary['train_time'] = time.time() - start_time

            logging.info('PITTING AGAINST PREVIOUS VERSION')
            start_time = time.time()

            arena = Arena(self.nnet, self.nnet, self.valid_data_loader)
            scores_p, scores_n = arena.playGames(arena_games)
            p_score = np.mean(scores_p)
            n_score = np.mean(scores_n)

            logging.info('NEW/PREV SCORE : %f / %f' % (n_score, p_score))
            logging.info('ACCEPTING NEW MODEL')

            # summarize the iteration
            summary['pit_time'] = time.time() - start_time
            summary['iter'] = i
            summary['train_sampled_examples'] = sum_examples
            summary['pit_nnet_score'] = n_score
            summary['pit_pnet_score'] = p_score

            # save summary
            best_iter, best_score = self.save_summary(summary)

            # save checkpoint
            self.nnet.save_checkpoint(folder=self.checkpoint, filename=self.getCheckpointFile(i))

            # save checkpoint for current best network (not necessarily the last)
            logging.info(f'Checkpoint from Iter {best_iter} is best, score is {best_score}')
            self.pnet.load_checkpoint(folder=self.checkpoint, filename=self.getCheckpointFile(best_iter))
            self.pnet.save_checkpoint(folder=self.checkpoint, filename='best.pth.tar')

    def getCheckpointFile(self, iteration):
        return 'checkpoint_' + str(iteration) + '.pth.tar'

    def saveTrainExamples(self, iteration):
        folder = self.checkpoint
        if not os.path.exists(folder):
            os.makedirs(folder)
        filename = os.path.join(folder, self.getCheckpointFile(iteration) + ".examples")
        with open(filename, "wb+") as f:
            Pickler(f).dump(self.trainExamplesHistory)
        f.closed

    def loadTrainExamples(self):
        modelFile = os.path.join(self.args.training.load_folder_file[0], self.args.training.load_folder_file[1])
        examplesFile = modelFile + ".examples"
        if not os.path.isfile(examplesFile):
            logging.warning(f'File "{examplesFile}" with trainExamples not found!')
            r = input("Continue? [y|n]")
            if r != "y":
                sys.exit()
        else:
            logging.info("File with trainExamples found. Loading it...")
            with open(examplesFile, "rb") as f:
                self.trainExamplesHistory = Unpickler(f).load()
            logging.info('Loading done!')

            # examples based on the model were already collected (loaded)
            self.skipFirstSelfPlay = True

    def save_summary(self, summary):
        """Save summary information.
        This implementation is currently inefficient for simplicity:
            - we build a new DataFrame each time
            - we write the whole summary file each time

        Returns:
            best_iter: iteration with best new network score in summary
            best_score: best new network score in summary
        """
        if self.summaries is None:
            self.summaries = pd.DataFrame([summary])
        else:
            self.summaries = self.summaries.append([summary], ignore_index=True)
        if self.checkpoint is not None:
            summary_file = os.path.join(self.checkpoint, 'summaries.csv')
            self.summaries.to_csv(summary_file, index=False)

        # get iter and score of best iteration
        best_idx = self.summaries.pit_nnet_score.idxmax()
        best_score = self.summaries.loc[[best_idx]].pit_nnet_score.item()
        best_iter = int(self.summaries.loc[[best_idx]].iter.item())

        return best_iter, best_score
