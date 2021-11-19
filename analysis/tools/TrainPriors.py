#!/usr/bin/env python3

# torch imports
import torch
from torch.optim import Adam
from torch.utils.data import DataLoader

# custom taylor cross entropy function
from basf2_mva_extensions.torch_TCCE import TCCE

# other imports
import uproot3 as ur
import numpy as np
from sklearn.metrics import accuracy_score
import argparse
import sys

# Model and DataLoader

from PriorDataLoaderAndModel import PriorModel
from PriorDataLoaderAndModel import PriorDataLoader

# description

"""
This tool can be used to train a pytorch model to calculate prior probabilities for particle identification.

This tool uses a root file containing momentum and cos(theta) (theta is the angle made by momentum vector with the beam axis)
and trains a pytorch model (imported as PriorModel) by taking a second order combination of these variables along with the
transverse momentum (thus giving 9 total features) which is done by the PriorDataLoader.
The program then outputs a '.pth' model file along with a scaling file (if the path is provided and if not this step is skipped).
The required input is taken as flags when using the program and these inputs are listed below.

Parameters:
    input_path (str): The path to the data file containing the data to train the model with.
    output_path (str): The path to the output model file that is generated after training.
    key (str): The key to the tree containing data within the root file.
    particle_list (int): List of mcPDG values to train the model with.
    variable_list (optional) (str): List of variables in the order cos(theta), momentum and PDG.
    taylor_terms (optional) (int): Number of Taylor series terms of categorical cross entropy loss to be used for training.
    learn_rate (optional) (float): The learning rate to be used for training the model.
    retrain_model (optional) (str): The path to an already trained model to be used for further training.
    scale_file (optional) (str): The path where a root file for temperature scaling must be stored for calibration purposes.
    epochs (optional) (int): The number of epochs for training the model.

Note:
    The parameters are taken as flags through the arguments parser.

Example:
    TrainPriors.py -i /path/to/datafile.root -o /path/to/model.pth -p 11 13 211 321 -k tree -v cosTheta p mcPDG
    -t 20 -lr 1e-6 -r /path/to/alreadyTrainedModel.pth -s /path/to/scalefile.root -e 10
"""

# model training


def mod_train(
    train_dl: DataLoader,
    model: PriorModel,
    test_dl: DataLoader,
    learn_rate: float,
    ep: int,
    terms: int,
):
    """
    Trains pytorch model for calculating PID priors.

    Parameters:
        train_dl (DataLoader): The training data loader.
        model (PriorModel): The torch model that is trained on the training set.
        test_dl (DataLoader): Data loader for validation set.
        learn_rate (float): The learning rate for the model.
        ep (int): Number of epochs to train the model for.
        terms (int): Number of terms for the taylor cross entropy loss with 0 denoting log loss.

    Returns:
        None.
    """
    optimizer = Adam(model.parameters(), lr=learn_rate)
    l_train = len(train_dl)
    l_test = len(test_dl)
    t = 5
    chacc = 0
    backup_model = model

    for epoch in range(ep):
        print("Epoch :", epoch)
        model = model.train()
        for i, (inputs, targets) in enumerate(train_dl):
            optimizer.zero_grad()
            if torch.cuda.is_available():
                inputs = inputs.to("cuda")
                targets = targets.to("cuda")
            y_out = model(inputs)
            y = torch.hstack((y_out, targets.reshape(-1, 1)))
            tcce = TCCE(terms)
            loss = tcce(y, targets)
            lam1 = 0.005
            all_lin1_param = torch.cat([x.view(-1) for x in model.hidden1.parameters()])
            all_lin2_param = torch.cat([x.view(-1) for x in model.hidden3.parameters()])
            all_lin3_param = torch.cat([x.view(-1) for x in model.hidden5.parameters()])
            all_lin4_param = torch.cat([x.view(-1) for x in model.hidden7.parameters()])
            l1_regularization = lam1 * (
                torch.norm(all_lin1_param, 1)
                + torch.norm(all_lin2_param, 1)
                + torch.norm(all_lin3_param, 1)
                + torch.norm(all_lin4_param, 1)
            )
            loss = loss + l1_regularization
            loss.backward()
            optimizer.step()
            if i != l_train - 1:
                print("The loss is: %0.3f" % loss.item(), end="\r")
            else:
                print("The loss is: %0.3f" % loss.item(), end="\n")
        model = model.eval()

        with torch.no_grad():
            acc = 0
            testloss = 0
            for i, (inputs, targets) in enumerate(test_dl):
                if torch.cuda.is_available():
                    inputs = inputs.to("cuda")
                y_out = model(inputs)
                y_max = [np.argmax(e) for e in y_out.to("cpu").detach().numpy()]
                acc += accuracy_score(y_max, targets.detach().numpy())
                if i != l_test - 1:
                    testloss += tcce(y_out, targets) / (l_test - 1)
            print("Avg. test loss is %0.3f" % (testloss))
            print("Test accuracy is %0.3f%%" % ((acc / (l_test)) * 100))
        if acc <= chacc and t != 0:
            t -= 1

        elif acc <= chacc and t == 0:
            model = backup_model
            break
        elif acc > chacc:
            stp_loss = loss.item()
            t = 4
            backup_model = model


# preparing data for training


def prep_dat(path: int, key: int, particlelist: list, labels: list) -> DataLoader:
    """
    Prepares dataloaders for training and validation set.

    Parameters:
        path (str): The path to the data file to train the model with.
        key (str): The key of the tree in the root file with the data required for training.
        particlelist (list(int)): List of particles to be considered for training purposes.
        labels (listi(str)): The list of labels for variables in the order cos(theta), momentum, and mcPDG.

    Returns:
        The data loaders for training and validation set to be used for training.
    """
    data = PriorDataLoader(path, key, particlelist, labels)
    train, test = data.get_split()
    train_dl = DataLoader(train, batch_size=512, shuffle=True)
    test_dl = DataLoader(test, batch_size=1024, shuffle=False)
    return train_dl, test_dl


# for evaluation to write the scaling file


def eval_model(model: PriorModel, datload: DataLoader) -> np.array:
    """
    Evaluates the model output for the model with the given dataset.

    Parameters:
        model (PriorModel): The trained torch model.
        datload (DataLoader): Dataloader containing labels as well as features of the data set.

    Returns:
        Numpy arrays containing probabilities as well as truth values to be stored in a scaling file
        for calibration purposes when calculating priors.
    """

    prob = []
    true = []
    model = model.eval()
    for i, (inputs, test) in enumerate(datload):
        if torch.cuda.is_available():
            y = model(inputs.to("cuda")).to("cpu").detach().numpy()
        else:
            y = model(inputs).detach().numpy()
        true.append(test)
        prob.append(y)
    return np.concatenate(prob), np.concatenate(true)


def get_parser() -> argparse.ArgumentParser:
    """
    Argument parser.

    Parameters:
        None.

    Returns:
        Argument parser.
    """
    ap = argparse.ArgumentParser()
    required_args = ap.add_argument_group("Required Arguments")
    required_args.add_argument(
        "-i", "--input_path", help="Path to ntuples conataining data for training.", required=True
    )
    required_args.add_argument(
        "-p",
        "--particle_list",
        help="List of particle mcPDG for which priors are to be calculated.",
        type=int,
        required=True,
        nargs="+",
    )
    required_args.add_argument(
        "-o", "--output_path", help="Output model file name (with path).", required=True
    )
    required_args.add_argument(
        "-k",
        "--key",
        help="Key of the tree to be used for training in the root file.",
        required=True,
    )

    ap.add_argument("-r", "--retrain_model", help="Path to the model for retraining.")
    ap.add_argument(
        "-v",
        "--variable_list",
        help="List of variable names in order cos(theta), momentum and pdg (Default: cosTheta p mcPDG).",
        default=["cosTheta", "p", "mcPDG"],
        nargs=3,
    )
    ap.add_argument(
        "-lr",
        "--learning_rate",
        help="Learning rate for training (Default: 1e-5).",
        default=1e-5,
        type=float,
    )
    ap.add_argument(
        "-e",
        "--epochs",
        help="Number of epochs to be trained for (Default: 64).",
        default=64,
        type=int,
    )
    ap.add_argument(
        "-t",
        "--taylor_terms",
        help="Number of terms for Taylor series of cross entropy loss (Default: 0 (takes log loss instead of taylor series)).",
        default=0,
        type=int,
    )
    ap.add_argument(
        "-s", "--scaling_file", help="Path to the root file to write data for scaling."
    )
    return ap


def main():
    description = __file__.__doc__
    args = get_parser().parse_args()
    path = args.input_path
    labels = args.variable_list
    particlelist = np.array(args.particle_list)
    output = args.output_path
    key = args.key
    old_model = args.retrain_model
    lr = args.learning_rate
    tt = args.taylor_terms
    epoch = args.epochs
    scalefile = args.scaling_file

    # preparing the data and model
    train_dl, test_dl = prep_dat(path, key, particlelist, labels)
    model = PriorModel(len(particlelist))
    if torch.cuda.is_available():
        model.to("cuda")
    if old_model is not None:
        model.load_state_dict(torch.load(old_model))

    # to allow the user to exit out of the training if earlystopping does not stop overfitting
    try:
        mod_train(train_dl, model, test_dl, lr, epoch, tt)
        torch.save(model.state_dict(), output)
        if scalefile is not None:
            y_prob, y_true = eval_model(model, train_dl)
            bdict = {}
            for p in particlelist:
                bdict[f"{p}_pdg"] = "float64"
                bdict["truth"] = "int32"
            with ur.recreate(scalefile) as f:
                f["scale"] = ur.newtree(bdict)
                datadict = {"truth": y_true}
                for p in range(len(particlelist)):
                    datadict[str(np.sort(particlelist)[p]) + "_pdg"] = y_prob[:, p]
                f["scale"].extend(datadict)

    except KeyboardInterrupt:
        torch.save(model.state_dict(), output)
        if scalefile is not None:
            y_prob, y_true = eval_model(model, train_dl)
            bdict = {}
            for p in particlelist:
                bdict[f"{p}_pdg"] = "float64"
            bdict["truth"] = "int32"
            with ur.recreate(scalefile) as f:
                f["scale"] = ur.newtree(bdict)
                datadict = {"truth": y_true}
                for p in range(len(particlelist)):
                    datadict[str(np.sort(particlelist)[p]) + "_pdg"] = y_prob[:, p]
                f["scale"].extend(datadict)
        sys.exit()


if __name__ == "__main__":
    main()
