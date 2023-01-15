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
Performs training of merger neural net using prepared training samples.

Usage:
python3 train.py  --inputdir=/path/to/folder  --checkpoint=/path/to/checkpoint
"""

import argparse
import logging
import torch
import numpy as np
import random

from vtx_cdc_merger.interaction_network.Coach import Coach
from vtx_cdc_merger.interaction_network.NNet import NNetWrapper as nn
from vtx_cdc_merger.datasets import get_data_loaders

import torch.multiprocessing
torch.multiprocessing.set_sharing_strategy('file_system')

seed = 1343214
n_train = 4000
n_valid = 1000
numIters = 500


def parse_args():
    """Parse command line arguments."""
    parser = argparse.ArgumentParser('collect_data.py')
    add_arg = parser.add_argument
    add_arg('--inputdir', type=str, default="vtx_cdc_merger_data")
    add_arg('--checkpoint', type=str, default="vtx_cdc_merger_checkpoint")
    return parser.parse_args()


def main():
    """Main function"""

    log_format = '%(asctime)s %(levelname)s %(message)s'
    log_level = logging.INFO
    logging.basicConfig(level=log_level, format=log_format)

    # Parse the command line
    args = parse_args()

    # Reproducible training [NOTE, doesn't full work on GPU]
    torch.manual_seed(seed)
    torch.backends.cudnn.deterministic = True
    torch.backends.cudnn.benchmark = False
    np.random.seed(seed)
    random.seed(seed)

    # Data loaders for training and validation
    train_data_loader, valid_data_loader = get_data_loaders(input_dir=args.inputdir, batch_size=1, n_train=n_train, n_valid=n_valid)
    logging.info('Loaded %g training samples', len(train_data_loader.dataset))
    if valid_data_loader is not None:
        logging.info('Loaded %g validation samples', len(valid_data_loader.dataset))

    nnet = nn()
    logging.info('Loaded %s...', nn.__name__)

    logging.info('Loading the Coach...')
    c = Coach(
        nnet,
        train_data_loader=train_data_loader,
        valid_data_loader=valid_data_loader,
        checkpoint=args.checkpoint,
        numIters=numIters)

    logging.info('Starting the learning process 🎉')
    c.learn()


if __name__ == "__main__":
    main()
