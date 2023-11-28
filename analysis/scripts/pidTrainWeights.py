#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Methods and a script for training PID calibration weights.

Sample usage:
    $ python pidTrainWeights.py data/ models/net.pt -n 100

Use `python pidTrainWeights.py -h` to see all command-line options.
"""

import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim

import numpy as np

from os import makedirs
from os.path import join, dirname
from tqdm.auto import tqdm


def _make_const_lists():
    """Moving this code into a function to avoid a top-level ROOT import."""
    import ROOT.Belle2

    PARTICLES, PDG_CODES = [], []
    for i in range(len(ROOT.Belle2.Const.chargedStableSet)):
        particle = ROOT.Belle2.Const.chargedStableSet.at(i)
        name = (particle.__repr__()[7:-1]
                .replace("-", "")
                .replace("+", "")
                .replace("euteron", ""))
        PARTICLES.append(name)
        PDG_CODES.append(particle.getPDGCode())
    # PARTICLES = ["e", "mu", "pi", "K", "p", "d"]
    # PDG_CODES = [11, 13, 211, 321, 2212, 1000010020]

    DETECTORS = []
    for det in ROOT.Belle2.Const.PIDDetectors.set():
        DETECTORS.append(ROOT.Belle2.Const.parseDetectors(det))
    # DETECTORS = ["SVD", "CDC", "TOP", "ARICH", "ECL", "KLM"]

    return PARTICLES, PDG_CODES, DETECTORS


# PARTICLES, PDG_CODES, DETECTORS = _make_const_lists()
PARTICLES = ["e", "mu", "pi", "K", "p", "d"]
PDG_CODES = [11, 13, 211, 321, 2212, 1000010020]
DETECTORS = ["SVD", "CDC", "TOP", "ARICH", "ECL", "KLM"]


class WeightNet(nn.Module):
    """PyTorch architecture for training calibration weights."""

    def __init__(self, n_class=6, n_detector=6, const_init=1):
        """Initialize the network for training.

        Args:
            n_class (int, optional): Number of classification classes (particle
                types). Defaults to 6.
            n_detector (int, optional): Number of detectors. Defaults to 6.
            const_init (int, optional): Constant value to initialize all
                weights. If None, PyTorch's default random initialization is
                used instead. Defaults to 1.
        """
        super().__init__()

        #: number of particle types
        self.n_class = n_class

        #: number of detectors
        self.n_detector = n_detector

        #: linear layers for each particle type
        self.fcs = nn.ModuleList(
            [nn.Linear(self.n_detector, 1, bias=False) for _ in range(self.n_class)]
        )

        if const_init is not None:
            self.const_init(const_init)

    def forward(self, x):
        """Network's forward methods. Sums the detector log-likelihoods for each particle
        type, then computes the likelihood ratios. Uses the weights.

        Args:
            x (torch.Tensor): Input detector log-likelihood data. Should be of
                shape (N, n_detector * n_class), where N is the number of samples.

        Returns:
            torch.Tensor: Weighted likelihood ratios.
        """
        n = self.n_detector
        outs = [self.fcs[i](x[:, i * n: (i + 1) * n]) for i in range(self.n_class)]
        out = torch.cat(outs, dim=1)
        return F.softmax(out, dim=1)

    def get_weights(self, to_numpy=True):
        """Returns the weights as a six-by-six array or tensor.

        Args:
            to_numpy (bool, optional): Whether to return the weights as a numpy
                array (True) or torch tensor (False). Defaults to True.

        Returns:
            np.array or torch.Tensor: The six-by-six matrix containing the
                weights.
        """
        weights = torch.cat([fc.weight.detach() for fc in self.fcs])
        if to_numpy:
            return weights.cpu().numpy()
        else:
            return weights

    def const_init(self, const):
        """Fill all the weights with the given value.

        Args:
            const (float): Constant value to fill all weights with.
        """
        with torch.no_grad():
            for fc in self.fcs:
                fc.weight.fill_(const)

    def random_init(self, mean=1.0, std=0.5):
        """Fill all the weights with values sampled from a Normal distribution
        with given mean and standard deviation.

        Args:
            mean (float, optional): The mean of the Normal distribution.
                Defaults to 1.0.
            std (float, optional): The standard deviation of the Normal
                distribution. Defaults to 0.5.
        """
        with torch.no_grad():
            for fc in self.fcs:
                fc.weight.fill_(0)
                fc.weight.add_(torch.normal(mean=mean, std=std, size=fc.weight.size()))

    def kill_unused(self, only):
        """Kills weights corresponding to unused particle types.

        Args:
            only (list(str) or None): List of allowed particle types. The
                weights corresponding to any particle types that are _not_ in
                this list will be filled with zero and be frozen (e.g. gradients
                will not be computed/updated).
        """
        if only is not None:
            # particle types that are not being trained...
            # set to zero and freeze
            for i, pdg in enumerate(PDG_CODES):
                if pdg in only:
                    continue
                self.fcs[i].weight.requires_grad = False
                self.fcs[i].weight.fill_(1)


def load_training_data(directory, p_lims=None, theta_lims=None, device=None):
    """Loads training and validation data within the given momentum and theta
    limits (if given).

    Args:
        directory (str): Directory containing the train and validation sets.
        p_lims (tuple(float), optional): Minimum and maximum momentum. Defaults
            to None.
        theta_lims (tuple(float), optional): Minimum and maximum theta in
            degrees. Defaults to None.
        device (torch.device, optional): Device to move the data onto. Defaults
            to None.

    Returns:
        torch.Tensor: Training log-likelihood data.
        torch.Tensor: Training labels.
        torch.Tensor: Validation log-likelihood data.
        torch.Tensor: Validation labels.
    """
    p_lo, p_hi = p_lims if p_lims is not None else (-np.inf, +np.inf)
    t_lo, t_hi = theta_lims if theta_lims is not None else (-np.inf, +np.inf)
    t_lo, t_hi = np.radians(t_lo), np.radians(t_hi)

    def _load(filename):
        data = np.load(filename)
        X, y, p, t = data["X"], data["y"], data["p"], data["theta"]
        mask = np.logical_and.reduce([p >= p_lo, p <= p_hi, t >= t_lo, t <= t_hi])
        X = torch.tensor(X[mask]).to(device=device, dtype=torch.float)
        y = torch.tensor(y[mask]).to(device=device, dtype=torch.long)
        return X, y

    X_tr, y_tr = _load(join(directory, "train.npz"))
    X_va, y_va = _load(join(directory, "val.npz"))
    return X_tr, y_tr, X_va, y_va


def load_checkpoint(filename, device=None, only=None):
    """Loads training from a checkpoint.

    Args:
        filename (str): Checkpoint filename.
        device (torch.device, optional): Device to move the data onto. Defaults
            to None.
        only (list(str), optional): List of allowed particle types. Defaults to
            None.

    Returns:
        WeightNet: The network.
        optim.Optimizer: The optimizer.
        int: Epoch number.
        dict(str -> list(float)): Training losses (diag, pion, sum) from each
            epoch.
        dict(str -> list(float)): Validation losses (diag, pion, sum) from every
            tenth epoch.
        dict(str -> list(float)): Training accuracies (net, pion) from each
            epoch.
        dict(str -> list(float)): Validation accuracies (net, pion) from every
            tenth epoch.
    """
    checkpoint = torch.load(filename, map_location=torch.device("cpu"))
    net = WeightNet()
    net.load_state_dict(checkpoint["model_state_dict"])
    net.kill_unused(only)
    net.to(device=device)

    opt = optim.Adam(filter(lambda p: p.requires_grad, net.parameters()), lr=5e-4)
    opt.load_state_dict(checkpoint["optimizer_state_dict"])

    return (
        net,
        opt,
        checkpoint["epoch"],
        checkpoint["loss_t"],
        checkpoint["loss_v"],
        checkpoint["accu_t"],
        checkpoint["accu_v"],
    )


def save_checkpoint(filename, net, opt, epoch, loss_t, loss_v, accu_t, accu_v):
    """Saves training to a checkpoint.

    Args:
        filename (str): Checkpoint filename.
        net (WeightNet): The network.
        opt (optim.Optimizer): The optimizer.
        epoch (int): The current epoch number.
        loss_t (dict(str -> list(float))): Training losses (diag, pion, sum)
            from each epoch.
        loss_v (dict(str -> list(float))): Validation losses (diag, pion, sum)
            from every tenth epoch.
        accu_t (dict(str -> list(float))): Training accuracies (net, pion) from
            each epoch.
        accu_v (dict(str -> list(float))): Validation accuracies (net, pion)
            from every tenth epoch.
    """
    net.cpu()
    makedirs(dirname(filename), exist_ok=True)
    torch.save(
        {
            "model_state_dict": net.state_dict(),
            "optimizer_state_dict": opt.state_dict(),
            "epoch": epoch,
            "loss_t": loss_t,
            "loss_v": loss_v,
            "accu_t": accu_t,
            "accu_v": accu_v,
        },
        filename,
    )


def initialize(args, device=None):
    """Initializes training from the parsed command-line arguments.

    Args:
        args (argparse.Namespace): Parsed command-line arguments.
        device (torch.device, optional): Device to move the data onto. Defaults
            to None.

    Returns:
        WeightNet: The network.
        optim.Optimizer: The optimizer.
        int: Epoch number.
        dict(str -> list(float)): Training losses (diag, pion, sum) from each
            epoch.
        dict(str -> list(float)): Validation losses (diag, pion, sum) from every
            tenth epoch.
        dict(str -> list(float)): Training accuracies (net, pion) from each
            epoch.
        dict(str -> list(float)): Validation accuracies (net, pion) from every
            tenth epoch.
    """
    if args.resume is not None:
        net, opt, epochs_0, l_t, l_v, a_t, a_v = load_checkpoint(args.resume, device=device, only=args.only)

    else:
        net = WeightNet()
        if args.random:
            net.random_init()
        net.kill_unused(args.only)
        net.to(device=device)
        opt = optim.Adam(filter(lambda p: p.requires_grad, net.parameters()), lr=5e-4)
        epochs_0 = 0
        l_t = {"diag": [], "pion": [], "sum": []}
        l_v = {"diag": [], "pion": [], "sum": []}
        a_t = {"net": [], "pion": []}
        a_v = {"net": [], "pion": []}

    return net, opt, epochs_0, l_t, l_v, a_t, a_v


def train_model(args, use_tqdm=True):
    """Trains and saves a model according to the command-line arguments.

    Args:
        args (argparse.Namespace): Parsed command-line arguments.
        use_tqdm (bool, optional): Use TQDM to track progress. Defaults to True.
    """
    print("Reading data.")
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print("...and moving to", device)
    X_tr, y_tr, X_va, y_va = load_training_data(
        args.input, device=device, p_lims=args.p_lims, theta_lims=args.theta_lims
    )

    if len(y_tr) < 10 or len(y_va) < 10:
        print("Not enough data. Aborting...")
        return

    print(f"{len(y_tr)} train events, {len(y_va)} val events")

    print("Initializing network.")
    net, opt, epochs_0, loss_t, loss_v, accu_t, accu_v = initialize(args, device=device)

    diag_lfn = nn.CrossEntropyLoss()
    pion_lfn = nn.BCELoss()
    pion_wgt = args.beta

    def compute_accuracies(out, y):
        output = out.detach().cpu().numpy()
        target = y.detach().cpu().numpy()
        pred = np.squeeze(np.argmax(output, axis=1))
        accu = np.count_nonzero(pred == target) / len(pred)
        pi_out = output[(target == 2), 2]
        pi_pred = (pi_out > 0.5).astype(float)
        pi_accu = pi_pred.sum() / len(pi_pred)
        return accu, pi_accu

    def lfn(output, target):
        diag = diag_lfn(output, target)
        pi_mask = target == 2
        pi_out = output[pi_mask, 2]
        pi_y = (target[pi_mask] == 2).float()
        pion = pion_lfn(pi_out, pi_y)
        return diag + pion_wgt * pion, diag, pion

    print(f"Training network for {args.n_epochs} epochs.")

    iterator = range(args.n_epochs)
    if use_tqdm:
        iterator = tqdm(iterator)

    for epoch in iterator:
        # train step
        net.train()
        opt.zero_grad()
        out = net(X_tr)
        loss, diag, pion = lfn(out, y_tr)
        loss.backward()
        opt.step()

        # record training data
        loss_t["diag"].append(diag.item())
        loss_t["pion"].append(pion.item())
        loss_t["sum"].append(loss.item())

        if epoch % 10 == 0:
            accu, pi_accu = compute_accuracies(out, y_tr)
            accu_t["net"].append(accu)
            accu_t["pion"].append(pi_accu)

        # val step
        net.eval()
        with torch.no_grad():
            out = net(X_va)
            loss, diag, pion = lfn(out, y_va)

            loss_v["diag"].append(diag.item())
            loss_v["pion"].append(pion.item())
            loss_v["sum"].append(loss.item())

            if epoch % 10 == 0:
                accu, pi_accu = compute_accuracies(out, y_va)
                accu_v["net"].append(accu)
                accu_v["pion"].append(pi_accu)

    print("Training complete.")

    net.cpu()
    save_checkpoint(
        args.output, net, opt, epochs_0 + args.n_epochs, loss_t, loss_v, accu_t, accu_v,
    )

    wgt = net.get_weights(to_numpy=True)
    np.save(args.output.replace(".pt", "_wgt.npy"), wgt)

    print(f"Model saved to {args.output}.")


def get_parser():
    """Handles the command-line argument parsing.

    Returns:
        argparse.Namespace: The parsed arguments.
    """
    from argparse import ArgumentParser

    ap = ArgumentParser(description="", epilog="")
    ap.add_argument(
        "input", type=str, help="Path to folder with training files (in .npz format).",
    )
    ap.add_argument(
        "output",
        type=str,
        help="Output filename where model will be saved (should end in .pt).",
    )
    ap.add_argument(
        "-n",
        "--n_epochs",
        type=int,
        default=500,
        help="Number of epochs to train the network. Defaults to 500.",
    )
    ap.add_argument(
        "--p_lims",
        nargs=2,
        default=[-float('inf'), +float('inf')],
        help=(
            "Lower and upper limits for momentum in GeV. Lower limit "
            "should be given first. Default values are -inf, +inf."
        ),
    )
    ap.add_argument(
        "--theta_lims",
        nargs=2,
        default=[-float('inf'), +float('inf')],
        help=(
            "Lower and upper limits for theta in degrees. Lower limit "
            "should be given first. Default values are -inf, +inf."
        ),
    )
    ap.add_argument(
        "-R",
        "--resume",
        nargs="?",
        type=str,
        const="use_output",
        default=None,
        help=(
            "Load a pre-existing model and resume training instead of "
            "starting a new one. If '--resume' is used and no file is "
            "specified, the output filepath will be loaded, and the "
            "training will overwrite that file. Alternatively, if a "
            "filepath is given, training will begin from the state "
            "saved in that file and will be saved to the output filepath."
        ),
    )
    ap.add_argument(
        "--only",
        type=int,
        nargs="*",
        help=(
            "Use only log-likelihood data from a subset of particle "
            "types specified by PDG code. If left unspecified, all "
            "particle types will be used."
        ),
    )
    ap.add_argument(
        "--random",
        action="store_true",
        help=(
            "Initialize network weights to random values, normally "
            "distributed with mean of 1 and width of 0.5. Has no effect "
            "if 'resume' is used."
        ),
    )
    ap.add_argument(
        "--beta",
        type=float,
        default=0.1,
        help=(
            "Scaling factor for the pion binary cross entropy term in "
            "the loss function. Defaults to 0.1."
        ),
    )
    return ap


def validate_args(args):
    # validate some values
    assert args.n_epochs > 0, "Number of epochs must be larger than 0."
    assert args.p_lims[0] < args.p_lims[1], "p_lims: lower limit must be < upper limit."
    assert (
        args.theta_lims[0] < args.theta_lims[1]
    ), "theta_lims: lower limit must be < upper limit."
    if args.only is not None:
        for pdg in args.only:
            assert pdg in PDG_CODES, f"Given PDG code {pdg} not understood."

    if args.resume == "use_output":
        args.resume = args.output

    return args


def main():
    """Main network training logic."""

    args = get_parser().parse_args()
    args = validate_args(args)

    print("Welcome to the network trainer.")
    print(f"Data will be read from {args.input}.")
    if args.resume is not None:
        print(f"Model training will be continued from the saved state at {args.resume}")
    print(f"The trained model will be saved to {args.output}.")

    if not (args.p_lims[0] == -np.inf and args.p_lims[0] == +np.inf):
        print(
            f"Model will be trained on data with momentum in "
            f"[{args.p_lims[0]}, {args.p_lims[1]}] GeV."
        )

    if not (args.theta_lims[0] == -np.inf and args.theta_lims[0] == +np.inf):
        print(
            f"Model will be trained on data with theta in "
            f"[{args.theta_lims[0]}, {args.theta_lims[1]}] degrees."
        )

    print(f"The model will be trained for {args.n_epochs} epochs.")
    print(f"Training will use a pion scaling factor of beta = {args.beta}.")
    print("---")

    train_model(args)
    print("\nFinished!")


if __name__ == "__main__":
    main()
