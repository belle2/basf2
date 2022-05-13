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
PyTorch dataset specifications.
"""

from torch.utils.data import DataLoader
from torch.utils.data.dataloader import default_collate


def get_data_loaders(batch_size, input_dir, n_train, n_valid):
    """This may replace the datasets function above"""
    collate_fn = default_collate

    from . import merger_data
    train_dataset, valid_dataset = merger_data.get_datasets(input_dir, n_train, n_valid)
    collate_fn = merger_data.collate_fn

    # Construct the data loaders
    loader_args = dict(batch_size=batch_size, collate_fn=collate_fn,
                       num_workers=1)
    train_sampler, valid_sampler = None, None

    train_data_loader = DataLoader(train_dataset, sampler=train_sampler,
                                   shuffle=(train_sampler is None), **loader_args)
    valid_data_loader = (DataLoader(valid_dataset, sampler=valid_sampler, **loader_args)
                         if valid_dataset is not None else None)
    return train_data_loader, valid_data_loader
