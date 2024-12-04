##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import torch_geometric
from pathlib import Path
from .geometric_datasets import GraphDataSet


def create_dataloader_mode_tags(configs, tags):
    """
    Convenience function to create the dataset/dataloader for each mode tag (train/val) and return them.

    Args:
        configs (dict): Training configuration.
        tags (list): Mode tags train/val containing dataset paths.

    Returns:
        dict: Mode tag dictionary containing tuples of (mode, dataset, dataloader).
    """

    mode_tags = {}

    for tag, path, mode in tags:
        dataset = GraphDataSet(
            root=Path(path, mode),
            run_name=configs["output"]["run_name"],
            **configs["dataset"]["config"],
        )

        print(
            f"{type(dataset).__name__} created for {mode} with {dataset.__len__()} samples\n"
        )

        dataloader = torch_geometric.loader.DataLoader(
            dataset, batch_size=configs["train"]["batch_size"],
            shuffle=True,
            drop_last=True,
        )

        mode_tags[tag] = (mode, dataset, dataloader)

    return mode_tags
