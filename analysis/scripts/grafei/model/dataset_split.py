import torch
import torch_geometric
from pathlib import Path
from .geometric_datasets import BelleRecoSetGeometricInMemory, BelleRecoSetGeometric


def create_dataloader_mode_tags(configs, tags):
    """
    Convenience function to create the dataset/dataloader for each mode tag (train/val) and return them.

    Args:
        configs (dict): Training configuration
        tags (list): Train/val mode tags containing dataset paths

    Returns:
        dict: Mode tag dictionary containing tuples of (mode, dataset, dataloader)
    """

    mode_tags = {}

    for tag, path, mode in tags:
        dataset_type = (
            BelleRecoSetGeometricInMemory
            if configs["dataset"]["inMemory"]
            else BelleRecoSetGeometric
        )

        dataset = dataset_type(
            root=Path(path, mode),
            run_name=configs["output"]["run_name"],
            overwrite=configs["geometric_model"]["overwrite"],
            **configs["dataset"]["config"],
        )

        print(
            f"{type(dataset).__name__} created for {mode} with {dataset.__len__()} samples"
        )

        dataloader_type = (
            torch_geometric.loader.DataListLoader
            if (torch.cuda.is_available() and torch.cuda.device_count() > 1)
            else torch_geometric.loader.DataLoader
        )

        dataloader = dataloader_type(
            dataset, batch_size=configs["train"]["batch_size"], shuffle=True
        )

        mode_tags[tag] = (mode, dataset, dataloader)

    return mode_tags
