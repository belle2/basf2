import ignite
from ignite.contrib.handlers.tensorboard_logger import TensorboardLogger, OutputHandler, WeightsHistHandler
import torch
from torch_geometric.data import Batch
import numpy as np
import collections.abc
from datetime import datetime
from pathlib import Path
import yaml
from .metrics import PerfectLCAGeometric, PerfectEvent, PerfectMasses, IsTrueB


class GraFEIIgniteTrainer:
    """A class to setup the ignite trainer and hold all the things associated"""

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
        include_efficiency=False,
    ):
        """These are all the inputs to ignite's create_supervised_trainer plus the yaml configs

        Args:
            model(Torch Model): The actual PyTorch model
            optimizer(Torch Optimizer): Optimizer used in training
            loss_fn(Torch Loss): Loss function
            device(Torch Device): Device to use
            configs(dict): Dictionary of run configs from loaded YAML config file
            tags(list): Various tags to sort train and validation evaluators by, e.g. "Training", "Validation Known"
            ignore_index(int): Label index to ignore when calculating metrics, e.g. padding
            include_efficiency(bool): Whether to include efficiency and purity metrics. Slows down computation significantly
        """

        self.model = model
        self.optimizer = optimizer
        self.configs = configs
        self.tags = tags
        self.ignore_index = ignore_index
        self.include_efficiency = include_efficiency
        self.device = device

        # Run timestamp to distinguish trainings
        self.timestamp = datetime.now().strftime("%Y.%m.%d_%H.%M")
        # And record monitor tag to know which data to watch
        self.monitor_tag = self.configs["dataset"]["config"]["monitor_tag"]

        # Output directory for checkpoints
        self.run_dir = None
        # Output directory for Tensorboard logging
        self.tb_dir = None
        if self.configs["output"] is not None:
            if ("path" in self.configs["output"].keys()) and (
                self.configs["output"]["path"] is not None
            ):
                self.run_dir = Path(
                    self.configs["output"]["path"],
                    self.configs["output"]["run_name"],
                )

            if ("tensorboard" in self.configs["output"].keys()) and (
                self.configs["output"]["tensorboard"] is not None
            ):
                # Need a timestamp to organise runs in tensorboard
                self.tb_dir = Path(
                    self.configs["output"]["tensorboard"],
                    self.configs["output"]["run_name"],
                    self.timestamp,
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
            model.train()  # This just sets the training mode, doesn't actually run the training which is done "manually" below

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

            if scheduler:
                scheduler.step()

            return loss.item()

        self.trainer = ignite.engine.Engine(_update_model)

        # Setup train and validation evaluators
        self.evaluators = {}

        for tag in self.tags:
            # Setup metrics
            metrics = {
                # ignite.metrics.Loss takes (y_pred, y, **kwargs) arguments.
                # MultiTrainLoss needs in total 6 arguments to be computed, so the additional ones are passed in a dictionary.
                "loss": ignite.metrics.Loss(
                    loss_fn,
                    output_transform=lambda x: [
                        x[0],
                        x[3],
                        {
                            "edge_input": x[1],
                            "edge_target": x[4],
                            "u_input": x[2],
                            "u_target": x[5],
                        },
                    ],
                    device=device,
                ),
                # "pad_accuracy": grafei.metrics.Pad_Accuracy(
                #     ignore_index=ignore_index,
                #     output_transform=lambda x: [x[1], x[4]],
                #     device=device,
                # ),
                # "perfectLCA": grafei.metrics.PerfectLCAGeometric(
                #     ignore_index=ignore_index,
                #     device=device,
                #     output_transform=lambda x: [
                #         x[1],
                #         x[4],
                #         x[6],
                #         x[5][:, 1],
                #         x[7],
                #         x[8],
                #     ],
                # ),
                "perfectLCA_noBkg": PerfectLCAGeometric(
                    ignore_index=ignore_index,
                    device=device,
                    output_transform=lambda x: [
                        x[1],
                        x[4],
                        x[6],
                        x[5],
                        x[7],
                        x[8],
                    ],
                    ignore_background=True,
                ),
                "perfectMasses_noBkg": PerfectMasses(
                    ignore_index=ignore_index,
                    device=device,
                    output_transform=lambda x: [x[0], x[3], x[5], x[7], x[8]],
                    ignore_background=True,
                ),
                "perfectEvent_noBkg": PerfectEvent(
                    ignore_index=ignore_index,
                    device=device,
                    output_transform=lambda x: [
                        x[0],
                        x[3],
                        x[1],
                        x[4],
                        x[6],
                        x[5],
                        x[7],
                        x[8],
                    ],
                    ignore_background=True,
                ),
            }
            if self.configs["geometric_model"]["global_layer"]:
                metrics.update(
                    {
                        "correct_class": IsTrueB(
                            ignore_index=ignore_index,
                            output_transform=lambda x: [x[2], x[5], x[8]],
                            device=device,
                        ),
                        # "momentum_loss": ignite.metrics.Loss(
                        #     grafei.losses.B_MomentumLoss(reduction="mean"),
                        #     output_transform=lambda x: [
                        #         x[2][:, :3],
                        #         x[5][:, :3],
                        #     ],
                        #     device=device,
                        # ),
                    }
                )

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

            # Add GPU memory info
            if self.configs["train"]["record_gpu_usage"] and device == torch.device(
                "cuda"
            ):
                ignite.contrib.metrics.GpuInfo().attach(
                    self.trainer, name="gpu"
                )  # metric names are 'gpu:X mem(%)', 'gpu:X util(%)'
                ignite.contrib.metrics.GpuInfo().attach(
                    self.evaluators[tag], name="gpu"
                )  # metric names are 'gpu:X mem(%)', 'gpu:X util(%)'

    def score_fn(self, engine):
        """Metric to use for early stoppging"""
        return engine.state.metrics["loss"]

    # def lca_score_fn(self, engine):
    #     """Metric to use for checkpoints"""
    #     return engine.state.metrics["perfectLCA_noBkg"]

    def perfect_score_fn(self, engine):
        """Metric to use for checkpoints"""
        return engine.state.metrics["perfectEvent_noBkg"]

    def _clean_config_dict(self, configs):
        """Clean configs to prepare them for writing to file

        This will convert any non-native types to Python natives.
        Currently just converts numpy arrays to lists.

        Args:
            configs (dict): Config dictionary

        Returns:
            dict: Cleaned config dict
        """
        # TODO: Add torch conversions as well
        for k, v in configs.items():
            if isinstance(v, collections.abc.Mapping):
                configs[k] = self._clean_config_dict(configs[k])
            elif isinstance(v, np.ndarray):
                configs[k] = v.tolist()
            else:
                configs[k] = v
        return configs

    def setup_handlers(self, cfg_filename="confignite.yaml"):
        """Create the various ignite handlers (callbacks)

        Args:
            cfg_filename(str, optional): Name of config yaml file to use when saving configs
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

        # Attach a progress bar
        if self.configs["train"]["progress_bar"]:
            progress_metrics = (
                ["gpu:0 mem(%)", "gpu:0 util(%)"]
                if (
                    self.configs["train"]["record_gpu_usage"]
                    and self.device == torch.device("cuda")
                )
                else None
            )
            pbar = ignite.contrib.handlers.ProgressBar(persist=True, bar_format="")
            # pbar.attach(self.trainer, output_transform=lambda x: {'loss': x})
            # pbar.attach(self.trainer, metric_names='all')
            pbar.attach(
                self.trainer,
                metric_names=progress_metrics,
                output_transform=lambda x: {"loss": x},
            )

        # Setup early stopping
        early_handler = ignite.handlers.EarlyStopping(
            patience=self.configs["train"]["early_stop_patience"],
            score_function=self.score_fn,
            trainer=self.trainer,
            min_delta=1e-3,
        )
        self.evaluators[self.monitor_tag].add_event_handler(
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
                score_function=self.perfect_score_fn,
                score_name="validation_perfectEvent",
                n_saved=1,
                global_step_transform=ignite.handlers.global_step_from_engine(
                    self.evaluators[self.monitor_tag]
                ),
            )
            self.evaluators[self.monitor_tag].add_event_handler(
                ignite.engine.Events.EPOCH_COMPLETED, best_model_handler
            )

        # Attach Tensorboard logging
        if self.tb_dir is not None:
            tb_logger = TensorboardLogger(
                log_dir=self.tb_dir, max_queue=1
            )

            # Attach the logger to the evaluator on the training and validation dataset and log NLL, Accuracy metrics after
            # each epoch. We setup `global_step_transform=global_step_from_engine(trainer)` to take the epoch of the
            # `trainer` instead of `evaluator`.
            for tag in self.tags:
                tb_logger.attach(
                    self.evaluators[tag],
                    event_name=ignite.engine.Events.EPOCH_COMPLETED,
                    log_handler=OutputHandler(
                        tag=tag,
                        metric_names="all",
                        global_step_transform=ignite.handlers.global_step_from_engine(
                            self.trainer
                        ),
                    ),
                )
            # Attach logger to trainer in order to save model weights as histograms
            tb_logger.attach(
                self.trainer,
                event_name=ignite.engine.Events.EPOCH_COMPLETED,
                log_handler=WeightsHistHandler(
                    self.model
                ),
            )

        return

    # Set up end of epoch validation procedure
    # Tell it to print epoch results
    def log_results(self, trainer, mode_tags):
        """Callback to run evaluation and report the results.

        We place this here since it needs access to the evaluator engines in order to run.
        No idea why ignite even bother to pass the trainer engine in the first place, their examples all call
        things created outside of the log_results function that aren't passed to it... bad programming practice in my opinion.

        Call this function via the add_event_handler() ignite function to tell it when to fire, e.g.:
            `GraFEIIgniteTrainer.trainer.add_event_handler(ignite.engine.Events.EPOCH_COMPLETED,
                                                           GraFEIIgniteTrainer.log_results,
                                                           mode_tags)`

        Args:
            trainer (ignite.Engine): trainer that gets passed by ignite to this method.
            mode_tags (dict): Dictionary of mode tags containing (mode, dataset, dataloader) tuples
        """

        for tag, values in mode_tags.items():
            evaluator = self.evaluators[tag]

            eval_steps = (
                self.configs["val"]["steps"] if "steps" in self.configs["val"] else None
            )
            # Need to wrap this in autocast since it caculates metrics (i.e. loss) without autocast switched on
            # This is mostly fine except it fails to correctly cast the class weights tensor passed to the loss
            if self.configs["train"]["mixed_precision"] and self.device == torch.device(
                "cuda"
            ):
                with torch.cuda.amp.autocast():
                    evaluator.run(values[2], epoch_length=eval_steps)
            else:
                evaluator.run(values[2], epoch_length=eval_steps)

            metrics = evaluator.state.metrics
            message = [f"{tag} Results - Epoch: {trainer.state.epoch}"]
            message.extend([f"Avg {m}: {metrics[m]:.4f}" for m in metrics])
            print(message)

            # Prints an example of a predicted LCA (for debugging only)
            # evaluator.state.output holds (y_pred, y)
            if (
                "print_sample" in self.configs["train"]
                and self.configs["train"]["print_sample"]
            ):
                y_pred, y = evaluator.state.output
                probs = torch.softmax(y_pred, dim=1)  # (N, C, d1, d2)
                winners = probs.argmax(dim=1)  # (N, d1, d2)
                mask = y == -1
                winners[mask] = -1
                print(values[1][0][0])  # Input features
                print(winners[0])
                print(y[0])
                # print(values[2][0])
