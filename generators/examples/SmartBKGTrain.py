##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import torch
import awkward as ak
import numpy as np
from pathlib import Path
from torch.utils.data import DataLoader

from smartBKG.models.gatgap import GATGAPModel
from smartBKG.utils.dataset import ArrayDataset
from smartBKG import MODEL_CONFIG
from smartBKG.utils.metrics import speedup

file_path = Path("./")
device = torch.device("cpu")


def load_events(
    filenames,
    balenced=True,
    max_events=None,
):
    """
    Load events from Parquet files.

    Args:
        filenames (list): List of file paths.
        balanced (bool): Whether to balance the number of pass and fail events.
        Should be set to `True` for training set and `False` for test set.
        max_events (int): Maximum number of events to load.

    Returns:
        tuple: Tuple containing arrays and meta information.
    """
    arrays = []
    info = {
        "total_udst": 0,
        "total_udst_fail": 0,
        "total_loaded_pass": 0,
        "total_loaded_fail": 0
    }
    for filename in filenames:
        ar = ak.from_parquet(filename)
        n_udst_pass = ak.sum(ar.label)
        n_udst_fail = ak.sum(~ar.label)
        n_total = n_udst_pass + n_udst_fail
        info["total_udst"] += n_total
        info["total_udst_fail"] += n_udst_fail

        if balenced:
            ar = ak.packed(
                ak.concatenate(
                    [
                        ar[ar.label],
                        ar[~ar.label][:n_udst_pass]
                    ]
                )
            )
        info["total_loaded_pass"] += n_udst_pass
        info["total_loaded_fail"] += len(ar) - n_udst_pass

        arrays.append(ar)
        if max_events:
            if len(ar) >= max_events:
                break
    return ak.with_field(ak.partitioned(arrays), True, "is_udst"), info


def get_loss(pred, label, retention_rate):
    """
    Compute the loss with retention rate applied. Can be extended for other losses.

    Args:
        pred (list): List of file paths.
        label (bool): True labels (ground truth).
        retention_rate (float): The rate at which events are retained by the filter.

    Returns:
        float: The speedup loss achieved by the filtering method.
    """
    return speedup(
        filter_prob=pred,
        y_true=label,
        retention_rate=retention_rate
    )


def fit(model, name, ds_train, ds_val, retention_rate, device=device,
        min_epochs=1, patience=12, lr_start=1e-3, lr_end=1e-4, epochs=1000):
    """
    Train the model with dynamic learning rate.

    Args:
        model (torch.nn.Module): The model used for the training.
        name (str): File name to save the model.
        ds_train (list): Dataset created with ArrayDataset for training.
        ds_val (list): Dataset created with ArrayDataset for validation.
        retention_rate (float): The rate at which events are retained by the filter.
        device (torch.device): The place to save dataset and model (`cpu` or `cuda`) during processing.
        min_epochs (int): The minimal number of epochs for the training.
        patience (int): The maximal number of continious epochs allowed to have no update.
        lr_start (float): The learning rate at the beginning of the training.
        lr_end (float): The minimal learning rate for the training. The strategy of dynamic learning rate
        is hard-coded with ReduceLROnPlateau scheduler and factor 1/2. This can be manually changed according
        to the result of hyper-parameter fine-tuning.
        epochs (int): The maximal number of epochs for the training.

    Returns:
        dict: The hitories of loss and accuracy for training and validation during the training.
    """
    history = {"loss": [], "val_loss": [], "acc": [], "val_acc": []}
    lr = lr_start
    opt = torch.optim.Adam(model.parameters(), lr=lr)
    scheduler = torch.optim.lr_scheduler.ReduceLROnPlateau(opt, factor=1/2, patience=patience, min_lr=lr_end)
    nan_count = 0
    for epoch in range(epochs):
        updated = False
        current_lr = [par["lr"] for par in opt.param_groups][0]
        # training
        dl_train = DataLoader(ds_train,  collate_fn=lambda x: x[0], num_workers=0)
        train_losses = []
        train_acces = []
        for x, y in dl_train:
            logits = model(x.to(device))
            labels = y.to(device)
            loss = get_loss(logits, labels, retention_rate)
            acc = ((logits > 0) == labels).float().mean()
            opt.zero_grad()
            loss.backward()
            opt.step()
            train_losses.append(float(loss.detach().cpu().numpy()))
            train_acces.append(float(acc.detach().cpu().numpy()))
        # change the seed such that we get newly shuffled batches the next iteration
        ds_train.seed += 1

        # validation
        with torch.no_grad():
            dl_val = DataLoader(ds_val,  collate_fn=lambda x: x[0], num_workers=0)
            val_losses = []
            val_acces = []
            for x, y in dl_val:
                logits = model(x.to(device))
                labels = y.to(device)
                loss = get_loss(logits, labels, retention_rate)
                acc = ((logits > 0) == labels).float().mean()
                val_losses.append(float(loss.detach().cpu().numpy()))
                val_acces.append(float(acc.detach().cpu().numpy()))

            history["val_loss"].append(np.mean(val_losses))
            history["loss"].append(np.mean(train_losses))
            history["val_acc"].append(np.mean(val_acces))
            history["acc"].append(np.mean(train_acces))

            min_idx = np.argmin(history["val_loss"])
            if torch.isnan(loss.detach()):
                model.load_state_dict(torch.load(name)['model_state_dict'])
                nan_count += 1
                if nan_count == int(patience/2):
                    break
                else:
                    scheduler.step(history["val_loss"][-1])
                    print(f'Nan detected, retry {nan_count}/{int(patience/2)}, lr {current_lr}')
                    continue
            if min_idx == (len(history["val_loss"]) - 1):
                updated = True
                torch.save(
                    {'model_state_dict': model.state_dict()},
                    name
                )
            scheduler.step(history["val_loss"][-1])
            if [par["lr"] for par in opt.param_groups][0] < current_lr:
                model.load_state_dict(torch.load(name)['model_state_dict'])

            print(f'epoch {epoch}, updated: {updated}, patience: {epoch-min_idx}/{patience}, lr: {current_lr}')
            print(f'train loss {history["loss"][-1]}, acc {history["acc"][-1]}')
            print(f'val loss {history["val_loss"][-1]}, acc {history["val_acc"][-1]}')

            if epoch < min_epochs:
                continue

            if ((patience <= epoch - min_idx) & (current_lr/2 < lr_end)) or (3*patience <= epoch - min_idx):
                model.load_state_dict(torch.load(name)['model_state_dict'])
                break
    return history


def test(model, ds_test, device=device):
    """
    Test the trained model.

    Args:
        model (torch.nn.Module): The model used for the training.
        ds_test (list): Dataset created with ArrayDataset for test.
        device (torch.device): The place to save dataset and model (`cpu` or `cuda`) during processing.

    Returns:
        dict: The test performance in terms of accuracy and the list of labels as well as predictions
        for all the events in the dataset.
    """
    record = {"acc": [], "label": [], "pred": []}
    with torch.no_grad():
        dl_test = DataLoader(ds_test,  collate_fn=lambda x: x[0], num_workers=0)
        acces = []
        truth = []
        preds = []
        for x, y in dl_test:
            logits = model(x.to(device))
            labels = y.to(device)
            acc = ((logits > 0) == labels).float().mean()
            acces.append(float(acc.detach().cpu().numpy()))
            truth.extend(labels.cpu().numpy())
            preds.extend(torch.sigmoid(logits).detach().cpu().numpy())
        record["acc"] = np.mean(acces)
        record["label"] = np.concatenate(truth)
        record["pred"] = np.concatenate(preds)
    return record


if __name__ == "__main__":
    # training process
    train_udst, info = load_events(
        filenames=file_path.glob('preprocessed*.parquet'),
        balenced=True
    )
    retention_rate = info["total_loaded_pass"] / info["total_udst"]
    max_events = info["total_loaded_pass"]+info["total_loaded_fail"]
    train_val_ratio = 0.8

    # the train_val_ratio and batch_size should be adjusted in case of small dataset
    # to make sure that there is no batch containing only pass/fail events
    ds_train = ArrayDataset(train_udst[:int(train_val_ratio*max_events)], batch_size=128, shuffle=True)
    ds_val = ArrayDataset(train_udst[int(train_val_ratio*max_events):], batch_size=1024, shuffle=True)

    model_name = "GATGAP.pth"
    model = GATGAPModel(**MODEL_CONFIG).to(device)
    history = fit(model, model_name, ds_train, ds_val, retention_rate=retention_rate)

    # test process
    test_udst, info = load_events(
        filenames=file_path.glob('preprocessed*.parquet'),
        balenced=False
    )
    ds_test = ArrayDataset(test_udst, batch_size=1024, shuffle=False)
    record = test(model, ds_test)
