#!/usr/bin/env python3

# torch imports
import torch
from torch.nn import CrossEntropyLoss
from torch.nn import Softmax
from torch.optim import Adam
from torch.utils.data import DataLoader
import torch.nn as nn
from torch.nn import functional as F

# custom taylor cross entropy function
from TCCE import TCCE

# other imports
import uproot as ur
import numpy as np
from sklearn.metrics import accuracy_score
import pandas as pd
import argparse
import re
import sys

# Model and DataLoader
from PriorDataLoaderAndModel import PriorModel
from PriorDataLoaderAndModel import PriorDataLoader

# model training


def mod_train(train_dl, model, test_dl, learn_rate, ep, terms):
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
                inputs = inputs.to('cuda')
                targets = targets.to('cuda')
            y_out = model(inputs)
            y = torch.hstack((y_out, targets.reshape(-1, 1)))
            tcce = TCCE(terms)
            loss = tcce(y, targets)
            lam1 = 0.005
            all_lin1_param = torch.cat([x.view(-1) for x in model.hidden1.parameters()])
            all_lin2_param = torch.cat([x.view(-1) for x in model.hidden3.parameters()])
            all_lin3_param = torch.cat([x.view(-1) for x in model.hidden5.parameters()])
            all_lin4_param = torch.cat([x.view(-1) for x in model.hidden7.parameters()])
            l1_regularization = lam1 * (torch.norm(all_lin1_param, 1) + torch.norm(all_lin2_param, 1) +
                                        torch.norm(all_lin3_param, 1) + torch.norm(all_lin4_param, 1))
            loss = loss + l1_regularization
            loss.backward()
            optimizer.step()
            if i != l_train - 1:
                print("The loss is: %0.3f" % loss.item(), end='\r')
            else:
                print("The loss is: %0.3f" % loss.item(), end='\n')
        model = model.eval()

        with torch.no_grad():
            acc = 0
            testloss = 0
            for i, (inputs, targets) in enumerate(test_dl):
                if torch.cuda.is_available():
                    inputs = inputs.to('cuda')
                y_out = model(inputs)
                y_max = [np.argmax(e) for e in y_out.to('cpu').detach().numpy()]
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


def prep_dat(path, key, particlelist, labels):
    data = PriorDataLoader(path, key, particlelist, labels)
    train, test = data.get_split()
    train_dl = DataLoader(train, batch_size=512, shuffle=True)
    test_dl = DataLoader(test, batch_size=1024, shuffle=False)
    return train_dl, test_dl

# for evaluation to write the scaling file


def evalmodel(model, datload, n):
    prob = []
    true = []
    model = model.eval()
    for i, (inputs, test) in enumerate(datload):
        y = model(inputs.to('cuda')).to('cpu').detach().numpy()
        true.append(test)
        prob.append(y)
    return np.concatenate(prob), np.concatenate(true)


def main():
    ap = argparse.ArgumentParser()
    required_args = ap.add_argument_group('required arguements')
    required_args.add_argument(
        '-i',
        '--input_path',
        help='Path to ntuples',
        required=True
    )
    ap.add_argument(
        '-r',
        '--retrain_model',
        help='Path to the model for retraining',
        default=""
    )
    ap.add_argument(
        '-v',
        '--variable_list',
        help='List of variable names in order cos(theta), momentum and pdg (default: cosTheta,p,mcPDG)',
        default="cosTheta,p,mcPDG"
    )
    required_args.add_argument(
        '-p',
        '--particle_list',
        help='List of particle mcPDG to be used (comma separated)',
        required=True
    )
    required_args.add_argument(
        '-o',
        '--output_path',
        help='Output model file name (with path)',
        required=True
    )
    required_args.add_argument(
        '-k',
        '--key',
        help='Key of the tree to be used for training in the root file',
        required=True
    )
    ap.add_argument(
        '-lr',
        '--learning_rate',
        help='Learning rate for training (default = 1e-5)',
        default=1e-5
    )
    ap.add_argument(
        '-e',
        '--epochs',
        help='Number of epochs to be trained for (default = 64)',
        default=64
    )
    ap.add_argument(
        '-t',
        '--taylor_terms',
        help='Number of terms for Taylor series of cross entropy loss (default = 0 (takes log loss instead of taylor series))',
        default=0
    )
    ap.add_argument(
        '-s',
        '--scaling_file',
        help='Path to the root file to write data for scaling',
        default=""
    )

    args = ap.parse_args()
    path = args.input_path
    labels = re.split(',', args.variable_list)
    particlelist = np.array(re.split(',', args.particle_list)).astype(int)
    output = args.output_path
    key = args.key
    old_model = args.retrain_model
    lr = float(args.learning_rate)
    tt = int(args.taylor_terms)
    epoch = int(args.epochs)
    scalefile = args.scaling_file

    # preparing the data and model
    train_dl, test_dl = prep_dat(path, key, particlelist, labels)
    model = PriorModel(len(particlelist))
    if torch.cuda.is_available():
        model.to('cuda')
    if old_model != "":
        model.load_state_dict(torch.load(old_model))

    # to allow the user to exit out of the training if earlystopping does not stop overfitting
    try:
        mod_train(train_dl, model, test_dl, lr, epoch, tt)
        torch.save(model.state_dict(), output)
        if scalefile != "":
            y_prob, y_true = evalmodel(model, train_dl, len(particlelist))
            bdict = {}
            for p in particlelist:
                bdict[str(p) + '_pdg'] = "float64"
                bdict["truth"] = "int32"
            with ur.recreate(scalefile) as f:
                f["scale"] = ur.newtree(bdict)
                datadict = {"truth": y_true}
                for p in range(len(particlelist)):
                    datadict[str(np.sort(particlelist)[p]) + '_pdg'] = y_prob[:, p]
                f["scale"].extend(datadict)

    except KeyboardInterrupt:
        torch.save(model.state_dict(), output)
        if scalefile != "":
            y_prob, y_true = evalmodel(model, train_dl, len(particlelist))
            bdict = {}
            for p in particlelist:
                bdict[str(p) + '_pdg'] = "float64"
            bdict["truth"] = "int32"
            with ur.recreate(scalefile) as f:
                f["scale"] = ur.newtree(bdict)
                datadict = {"truth": y_true}
                for p in range(len(particlelist)):
                    datadict[str(np.sort(particlelist)[p]) + '_pdg'] = y_prob[:, p]
                f["scale"].extend(datadict)
        sys.exit()


if __name__ == "__main__":
    main()
