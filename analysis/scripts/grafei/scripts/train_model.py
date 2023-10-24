#!/usr/bin/env python
# -*- coding: utf-8 -*-
import logging
import sys
from pathlib import Path
import click
import torch
import torch_geometric
import ignite as ig
import numpy as np
import random
from grafei.model.config import load_config
from grafei.model.dataset_split import create_dataloader_mode_tags
from grafei.model.geometric_network import GeometricNetwork
from grafei.model.data_utils import calculate_class_weights
from grafei.model.multiTrain import MultiTrainLoss
from grafei.model.create_trainer import GraFEIIgniteTrainer


@click.command()
@click.option(
    "-c",
    "--config",
    "cfg_path",
    required=True,
    type=click.Path(exists=True),
    help="path to config file",
)
@click.option(
    "-s",
    "--samples",
    "samples",
    required=False,
    type=int,
    help="Number of samples to train on",
)
@click.option("--quiet", "log_level", flag_value=logging.WARNING, default=True)
@click.option("-v", "--verbose", "log_level", flag_value=logging.INFO)
@click.option("-vv", "--very-verbose", "log_level", flag_value=logging.DEBUG)
# @click.version_option(grafei.__version__)
def main(
    cfg_path: Path,
    samples: int,
    log_level: int,
):
    logging.basicConfig(
        stream=sys.stdout,
        level=log_level,
        datefmt="%Y-%m-%d %H:%M",
        format="%(asctime)s - %(name)s - %(levelname)s - %(message)s",
    )

    # First figure out which device all this is running on
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"Using {device} device")

    # Load configs
    configs, tags = load_config(
        Path(cfg_path).resolve(),
        samples=samples,
    )

    # assert configs["geometric_model"]["global_layer"] or (
    #     not configs["geometric_model"]["global_layer"]
    #     and configs["geometric_model"]["alpha_momentum"] == 0
    #     and configs["geometric_model"]["alpha_prob"] == 0
    # ), "You can not learn B probability and/or B momentum without global layer"

    # Random seed
    if configs["train"]["seed"]:
        seed = configs["train"]["seed"]
        random.seed(seed)
        np.random.seed(seed)
        torch.manual_seed(seed)
        torch.cuda.manual_seed_all(seed)

    # Load datasets
    mode_tags = create_dataloader_mode_tags(configs, tags)

    # Now build the model
    # Extract the number of features, assuming last dim is features
    n_infeatures = mode_tags["Training"][1][0].x.shape[-1]
    e_infeatures = mode_tags["Training"][1][0].edge_attr.shape[-1]
    g_infeatures = mode_tags["Training"][1][0].u.shape[-1]

    base_model = GeometricNetwork(
        nfeat_in_dim=n_infeatures,
        efeat_in_dim=e_infeatures,
        gfeat_in_dim=g_infeatures,
        edge_classes=configs["dataset"]["edge_classes"],
        x_classes=configs["dataset"]["x_classes"],
        **configs["geometric_model"],
    )

    # Parallelize on multiple GPUs
    if torch.cuda.device_count() > 1:
        print("We're going to use", torch.cuda.device_count(), "GPUs!")
        base_model = torch_geometric.nn.DataParallel(base_model)

    print(f"Model: {base_model}")
    print(
        f"Number of parameters: {sum(p.numel() for p in base_model.parameters() if p.requires_grad)}"
    )

    # Compile the model (requires Pytorch >= 2.0.0)
    if float(torch.__version__[0]) >= 2 and configs["train"]["compile_model"]:
        print("Compiling the model!")
        model = torch.compile(base_model)
    else:
        model = base_model

    # Send the model to specific device
    model.to(device)

    # Compute class weights
    edge_weights = None
    node_weights = None
    if configs["train"]["class_weights"]:
        edge_weights = calculate_class_weights(
            dataloader=mode_tags["Training"][2],
            num_classes=configs["dataset"]["edge_classes"],
            edges=True,
        )
        node_weights = calculate_class_weights(
            dataloader=mode_tags["Training"][2],
            num_classes=configs["dataset"]["x_classes"],
            edges=False,
        )
        edge_weights = edge_weights.to(device)
        node_weights = node_weights.to(device)

    # Set the loss
    loss_fn = MultiTrainLoss(
        ignore_index=-1,
        reduction=configs["geometric_model"]["loss_reduction"],
        # alpha_momentum=configs["geometric_model"]["alpha_momentum"],
        alpha_mass=configs["geometric_model"]["alpha_mass"],
        # alpha_prob=configs["geometric_model"]["alpha_prob"],
        # global_layer=configs["geometric_model"]["global_layer"],
        edge_weights=edge_weights,
        node_weights=node_weights,
    )

    # Set the optimiser
    optimizer = torch.optim.Adam(
        model.parameters(),
        configs["train"]["learning_rate"],
        weight_decay=configs["train"]["l2_loss"],
        amsgrad=False,
        eps=0.001,
    )

    scheduler = (
        torch.optim.lr_scheduler.MultiStepLR(
            optimizer=optimizer,
            milestones=[6, 11],
            gamma=0.5,
        )
        if configs["train"]["lr_scheduler"]
        else None
    )

    grafei_ignite_trainer = GraFEIIgniteTrainer(
        model=model,
        optimizer=optimizer,
        loss_fn=loss_fn,
        device=device,
        configs=configs,
        tags=list(mode_tags.keys()),
        scheduler=scheduler,
        ignore_index=-1,
        include_efficiency=configs["train"]["include_efficiency"],
    )

    # Set up the actual checkpoints and save the configs if requested
    grafei_ignite_trainer.setup_handlers(
        cfg_filename=Path(cfg_path).name,
    )

    # Add callback to run evaluation after each epoch
    grafei_ignite_trainer.trainer.add_event_handler(
        ig.engine.Events.EPOCH_COMPLETED,
        grafei_ignite_trainer.log_results,
        mode_tags,
    )

    # Actually run the training, mode_tags calls the train_loader
    train_steps = configs["train"]["steps"] if "steps" in configs["train"] else None
    grafei_ignite_trainer.trainer.run(
        mode_tags["Training"][2],
        max_epochs=configs["train"]["epochs"],
        epoch_length=train_steps,
    )


if __name__ == "__main__":
    main()
