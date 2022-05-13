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
PyTorch specification for the merger dataset.
"""

import sys
import os
import torch
import pandas as pd
from torch.utils.data import Dataset, random_split

from collections import namedtuple

import vtx_cdc_merger.var_set as var_set


# MergerData is a namedtuple of matrices
MergerData = namedtuple('MergerData', ['vxd_hits', 'cdc_hits', 'vxd_tracks', 'cdc_tracks', 'true_vxd',
                                       'true_cdc', 'true_links'])


def load_data(filename, preprocess=True):
    """Read merger data for single event from file"""

    vxd_hits = pd.read_hdf(filename, 'vxd_hits')
    cdc_hits = pd.read_hdf(filename, 'cdc_hits')
    vxd_tracks = pd.read_hdf(filename, 'vxd_tracks')
    cdc_tracks = pd.read_hdf(filename, 'cdc_tracks')

    vxd_hits = var_set.get_array(vxd_hits, var_set.scales_vxd_hits, use_scales=preprocess)
    cdc_hits = var_set.get_array(cdc_hits, var_set.scales_cdc_hits, use_scales=preprocess)
    vxd_tracks = var_set.get_array(vxd_tracks, var_set.scales_vxd_tracks, use_scales=preprocess)
    cdc_tracks = var_set.get_array(cdc_tracks, var_set.scales_cdc_tracks, use_scales=preprocess)

    true_vxd = pd.read_hdf(filename, 'true_vxd').values
    true_cdc = pd.read_hdf(filename, 'true_cdc').values
    true_links = pd.read_hdf(filename, 'true_links').values

    return MergerData(vxd_hits, cdc_hits, vxd_tracks, cdc_tracks, true_vxd, true_cdc, true_links)


class MergerDataset(Dataset):
    """PyTorch dataset specification for merger data"""

    def __init__(self, input_dir, n_samples=None):
        input_dir = os.path.expandvars(input_dir)
        filenames = [os.path.join(input_dir, f) for f in os.listdir(input_dir)
                     if f.startswith('event_id') and not f.endswith('_ID.npz')]
        self.filenames = (
            filenames[:n_samples] if n_samples is not None else filenames)

    def __getitem__(self, index):
        return load_data(self.filenames[index])

    def __len__(self):
        return len(self.filenames)


def get_datasets(input_dir, n_train, n_valid):
    data = MergerDataset(input_dir, n_train + n_valid)
    # deterministic splitting ensures all workers split the same way
    torch.manual_seed(1)
    # Split into train and validation
    train_data, valid_data = random_split(data, [n_train, n_valid])
    return train_data, valid_data


def collate_fn(merger_data):
    """
    Collate function for building mini-batches from a list of merger data events.
    This function should be passed to the pytorch DataLoader.

    This implementation could probably be optimized further.
    """

    batch_size = len(merger_data)

    # Special handling of batch size 1
    if batch_size == 1:
        g = merger_data[0]
        return [torch.from_numpy(m).float() for m in g]

    else:
        print('Not supported ')
        sys.exit(1)
