##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import ignite
import torch
from torch_geometric.data import Batch
import numpy as np
import collections.abc
from datetime import datetime
from pathlib import Path
import yaml
from .metrics import PerfectLCA, PerfectEvent, PerfectMasses


class GraFEIIgniteTrainer:
    """
    Class to setup the ignite trainer and hold all the things associated.

    :param model: The actual PyTorch model.
    :type model: `Model <https://pytorch.org/tutorials/beginner/introyt/modelsyt_tutorial.html>`_
    :param optimizer: Optimizer used in training.
    :type optimizer: `Optimizer <https://pytorch.org/docs/stable/optim.html#torch.optim.Optimizer>`_
    :param loss_fn: Loss function.
    :type loss_fn: `Loss <https://pytorch.org/docs/stable/nn.html#loss-functions>`_
    :param device: Device to use.
    :type device: `Device <https://pytorch.org/docs/stable/tensor_attributes.html#torch.device>`_
    :param configs: Dictionary of run configs from loaded yaml config file.
    :type configs: dict
    :param tags: Various tags to sort train and validation evaluators by, e.g. "Training", "Validation".
    :type tags: list
    :param scheduler: Learning rate scheduler.
    :type scheduler: `Scheduler <https://pytorch.org/docs/stable/optim.html#how-to-adjust-learning-rate>`_
    :param ignore_index: Label index to ignore when calculating metrics, e.g. padding.
    :type ignore_index: int
    """

    def __init__(
        self,
        model,
        optimizer,
        loss_fn,
        device,
        configs,
        tags,
        scheduler=None,
        ignore_index=-1.0,
    ):
        """
        Initialization.
        """
        #: Model
        self.model = model
        #: Optimizer
        self.optimizer = optimizer
        #: Configs
        self.configs = configs
        #: Tags
        self.tags = tags
        #: Index to ignore
        self.ignore_index = ignore_index
        #: CPU or GPU
        self.device = device

        #: Run timestamp to distinguish trainings
        self.timestamp = datetime.now().strftime("%Y.%m.%d_%H.%M")

        #: Output directory for checkpoints
        self.run_dir = None
        if self.configs["output"] is not None:
            if ("path" in self.configs["output"].keys()) and (
                self.configs["output"]["path"] is not None
            ):
                self.run_dir = Path(
                    self.configs["output"]["path"],
                    self.configs["output"]["run_name"],
                )

        # Setup ignite trainer
        use_amp = configs["train"]["mixed_precision"] and self.device == torch.device(
            "cuda"
        )

        if use_amp:
            from torch.cuda.amp import autocast
            from torch.cuda.amp import GradScaler

            scaler = GradScaler(enabled=True)

        def _update_model(engine, batch):
            # This just sets the training mode
            model.train()

            optimizer.zero_grad()

            batch = (
                Batch.from_data_list(batch).to(device)
                if isinstance(batch, list)
                else batch.to(device)
            )

            x_y, edge_y, u_y = batch.x_y, batch.edge_y, batch.u_y

            if use_amp:
                with autocast(enabled=True):
                    x_pred, e_pred, u_pred = model(batch)
                    loss = loss_fn(x_pred, x_y, e_pred, edge_y, u_pred, u_y)
                scaler.scale(loss).backward()
                scaler.step(optimizer)
                scaler.update()
            else:
                x_pred, e_pred, u_pred = model(batch)
                loss = loss_fn(x_pred, x_y, e_pred, edge_y, u_pred, u_y)
                loss.backward()
                optimizer.step()

            return loss.item()

        #: Ignite trainer
        self.trainer = ignite.engine.Engine(_update_model)

        if scheduler:
            ig_scheduler = ignite.handlers.param_scheduler.LRScheduler(scheduler)
            self.trainer.add_event_handler(ignite.engine.Events.ITERATION_STARTED, ig_scheduler)

        #: Setup train and validation evaluators
        self.evaluators = {}

        for tag in self.tags:
            # Setup metrics
            metrics = {
                # ignite.metrics.Loss takes (y_pred, y, **kwargs) arguments.
                # MultiTrainLoss needs in total 6 arguments to be computed,
                # so the additional ones are passed in a dictionary.
                "loss": ignite.metrics.Loss(
                    loss_fn,
                    output_transform=lambda x: [
                        x[0], x[3],
                        {
                            "edge_input": x[1],
                            "edge_target": x[4],
                            "u_input": x[2],
                            "u_target": x[5],
                        },
                    ],
                    device=device,
                ),
                "perfectLCA": PerfectLCA(
                    ignore_index=ignore_index,
                    device=device,
                    output_transform=lambda x: [
                        x[1], x[4], x[6], x[5], x[7], x[8],
                    ],
                ),
                "perfectMasses": PerfectMasses(
                    ignore_index=ignore_index,
                    device=device,
                    output_transform=lambda x: [x[0], x[3], x[5], x[7], x[8]],
                ),
                "perfectEvent": PerfectEvent(
                    ignore_index=ignore_index,
                    device=device,
                    output_transform=lambda x: [
                        x[0], x[3], x[1], x[4], x[6], x[5], x[7], x[8],
                    ],
                ),
            }

            def _predict_on_batch(engine, batch):
                model.eval()  # It just enables evaluation mode

                batch = (
                    Batch.from_data_list(batch).to(device)
                    if isinstance(batch, list)
                    else batch.to(device)
                )

                x_y, edge_y, u_y, edge_index, torch_batch = (
                    batch.x_y,
                    batch.edge_y,
                    batch.u_y,
                    batch.edge_index,
                    batch.batch,
                )
                num_graph = batch.batch[torch_batch.shape[0] - 1] + 1

                with torch.no_grad():
                    if use_amp:
                        with autocast(enabled=True):
                            x_pred, e_pred, u_pred = model(batch)
                    else:
                        x_pred, e_pred, u_pred = model(batch)

                return (
                    x_pred,
                    e_pred,
                    u_pred,
                    x_y,
                    edge_y,
                    u_y,
                    edge_index,
                    torch_batch,
                    num_graph,
                )

            self.evaluators[tag] = ignite.engine.Engine(_predict_on_batch)

            for metric_name, metric in zip(metrics.keys(), metrics.values()):
                metric.attach(self.evaluators[tag], metric_name)

    def _score_fn(self, engine):
        """Metric to use for early stoppging"""
        return -engine.state.metrics["loss"]

    def _perfect_score_fn(self, engine):
        """Metric to use for checkpoints"""
        return engine.state.metrics["perfectEvent"]

    def _clean_config_dict(self, configs):
        """
        Clean configs to prepare them for writing to file.
        """
        for k, v in configs.items():
            if isinstance(v, collections.abc.Mapping):
                configs[k] = self._clean_config_dict(configs[k])
            elif isinstance(v, np.ndarray):
                configs[k] = v.tolist()
            else:
                configs[k] = v
        return configs

    def setup_handlers(self, cfg_filename="config.yaml"):
        """
        Creates the various ignite handlers (callbacks).

        Args:
            cfg_filename (str): Name of config yaml file to use when saving configs.
        """
        # Create the output directory
        if self.run_dir is not None:
            self.run_dir.mkdir(parents=True, exist_ok=True)
            # And save the configs, putting here to only save when setting up checkpointing
            with open(
                self.run_dir / f"{self.timestamp}_{cfg_filename}", "w"
            ) as outfile:
                cleaned_configs = self._clean_config_dict(self.configs)
                yaml.dump(cleaned_configs, outfile, default_flow_style=False)

        # Setup early stopping
        early_handler = ignite.handlers.EarlyStopping(
            patience=self.configs["train"]["early_stop_patience"],
            score_function=self._score_fn,
            trainer=self.trainer,
            min_delta=1e-3,
        )
        self.evaluators["Validation"].add_event_handler(
            ignite.engine.Events.EPOCH_COMPLETED, early_handler
        )

        # Configure saving the best performing model
        if self.run_dir is not None:
            to_save = {
                "model": self.model,
                "optimizer": self.optimizer,
                "trainer": self.trainer,
            }
            # Note that we judge early stopping above by the validation loss, but save the best model
            # according to validation perfectEvent score. This lets training continue for perfectEvent plateaus
            # so long as the model is still changing (and hopefully improving again after some time).
            best_model_handler = ignite.handlers.Checkpoint(
                to_save=to_save,
                save_handler=ignite.handlers.DiskSaver(
                    self.run_dir, create_dir=True, require_empty=False
                ),
                filename_prefix=self.timestamp,
                score_function=self._perfect_score_fn,
                score_name="validation_perfectEvent",
                n_saved=1,
                global_step_transform=ignite.handlers.global_step_from_engine(
                    self.evaluators["Validation"]
                ),
            )
            self.evaluators["Validation"].add_event_handler(
                ignite.engine.Events.EPOCH_COMPLETED, best_model_handler
            )

        return

    # Set up end of epoch validation procedure
    # Tell it to print epoch results
    def log_results(self, trainer, mode_tags):
        """
        Callback to run evaluation and report the results.

        :param trainer: Trainer passed by ignite to this method.
        :type trainer: `Engine <https://pytorch.org/ignite/generated/ignite.engine.engine.Engine.html#ignite.engine.engine.Engine>`_
        :param mode_tags: Dictionary of mode tags containing (mode, dataset, dataloader) tuples.
        :type mode_tags: dict
        """

        for tag, values in mode_tags.items():
            evaluator = self.evaluators[tag]

            # Need to wrap this in autocast since it caculates metrics (i.e. loss) without autocast switched on
            # This is mostly fine except it fails to correctly cast the class weights tensor passed to the loss
            if self.configs["train"]["mixed_precision"] and self.device == torch.device("cuda"):
                with torch.cuda.amp.autocast():
                    evaluator.run(values[2], epoch_length=None)
            else:
                evaluator.run(values[2], epoch_length=None)

            metrics = evaluator.state.metrics
            message = [f"{tag} Results - Epoch: {trainer.state.epoch}"]
            message.extend([f"Avg {m}: {metrics[m]:.4f}" for m in metrics])
            print(message)
