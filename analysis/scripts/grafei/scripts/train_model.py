#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import logging
import sys
from pathlib import Path
import click
import torch
import ignite as ig
import numpy as np
import random
from grafei.model.config import load_config
from grafei.model.dataset_split import create_dataloader_mode_tags
from grafei.model.geometric_network import GraFEIModel
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
    "-n",
    "--n_samples",
    "n_samples",
    required=False,
    type=int,
    help="Number of samples to train on",
)
@click.option("--quiet", "log_level", flag_value=logging.WARNING, default=True)
@click.option("-v", "--verbose", "log_level", flag_value=logging.INFO)
@click.option("-vv", "--very-verbose", "log_level", flag_value=logging.DEBUG)
def main(
    cfg_path: Path,
    n_samples: int,
    log_level: int,
):
    """"""
    logging.basicConfig(
        stream=sys.stdout,
        level=log_level,
        datefmt="%Y-%m-%d %H:%M",
        format="%(asctime)s - %(name)s - %(levelname)s - %(message)s",
    )

    # First figure out which device all this is running on
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"Using {str(device).upper()} device\n")

    # Load configs
    configs, tags = load_config(
        Path(cfg_path).resolve(),
        samples=n_samples,
    )

    # Random seed
    if configs["train"]["seed"]:
        seed = configs["train"]["seed"]
        random.seed(seed)
        np.random.seed(seed)
        torch.manual_seed(seed)
        torch.cuda.manual_seed_all(seed)

    # Load datasets
    mode_tags = create_dataloader_mode_tags(configs, tags)

    # Find out if we are reconstructing B or Ups
    B_reco = mode_tags["Training"][1].B_reco

    configs["model"].update({"edge_classes": 6 if B_reco else 7, "B_reco": B_reco})

    # Now build the model
    # Extract the number of features, assuming last dim is features
    n_infeatures = mode_tags["Training"][1][0].x.shape[-1]
    e_infeatures = mode_tags["Training"][1][0].edge_attr.shape[-1]
    g_infeatures = mode_tags["Training"][1][0].u.shape[-1]

    base_model = GraFEIModel(
        nfeat_in_dim=n_infeatures,
        efeat_in_dim=e_infeatures,
        gfeat_in_dim=g_infeatures,
        **configs["model"],
    )

    print(f"Model: {base_model}\n")
    print(
        f"Number of model parameters: {sum(p.numel() for p in base_model.parameters() if p.requires_grad)}\n"
    )
    print(f"Using LCAS format, max depth of {5 if B_reco else 6} corresponding to {'B' if B_reco else 'Upsilon(4S)'}\n")

    # Compile the model (requires PyTorch >= 2.0.0)
    if float(torch.__version__[0]) >= 2 and configs["train"]["compile_model"]:
        print("Compiling the model!")
        model = torch.compile(base_model)
    else:
        model = base_model

    # Send the model to specific device
    model.to(device)

    # Set the loss
    loss_fn = MultiTrainLoss(
        ignore_index=-1,
        reduction=configs["model"]["loss_reduction"],
        alpha_mass=configs["model"]["alpha_mass"],
    )

    # Set the optimiser
    optimizer = torch.optim.Adam(
        model.parameters(),
        configs["train"]["learning_rate"],
        weight_decay=0,
        amsgrad=False,
        eps=0.001,
    )

    scheduler = torch.optim.lr_scheduler.CosineAnnealingLR(
        optimizer=optimizer,
        T_max=10,
    ) if configs["train"]["lr_scheduler"] else None

    grafei_ignite_trainer = GraFEIIgniteTrainer(
        model=model,
        optimizer=optimizer,
        loss_fn=loss_fn,
        device=device,
        configs=configs,
        tags=list(mode_tags.keys()),
        scheduler=scheduler,
        ignore_index=-1,
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
