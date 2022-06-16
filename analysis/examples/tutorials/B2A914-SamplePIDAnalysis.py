#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
# This tutorial shows how to kick off an analysis using a trained set of #
# PID calibration weights. Only the computation of overall accuracy and  #
# the pi and K efficiencies is shown. This is intended to be used after  #
# <prep_data.py> and training a set of weights.                          #
##########################################################################

import pidDataUtils as pdu
import numpy as np

std = pdu.read_npz('data/slim_dstar/test.npz')
wgt = std.copy(deep=True)
print(f'{len(std)} events')

weights = np.load('models/net_wgt.npy')
std = pdu.prepare_df(std)
wgt = pdu.prepare_df(wgt, weights=weights)
print(f'{len(std)} events after cuts')

print('\nValues of the weights')
print(weights)


def compute_accuracy(df, mask=None):
    if mask:
        _df = df.loc[mask]
    return (_df['pid'] == _df['labels']).values.sum() / len(_df)


print('\n          (std)  (wgt)')
std_acc = compute_accuracy(std)
wgt_acc = compute_accuracy(wgt)
print(f'Accuracy: {std_acc:.3f}  {wgt_acc:.3f}')
for label in [2, 3]:
    lbl = "pion" if label == 2 else "kaon"
    _std_eff = compute_accuracy(std, mask=std["labels"] == label)
    _wgt_eff = compute_accuracy(wgt, mask=wgt["labels"] == label)
    print(f'{lbl} eff: {_std_eff:.3f}  {_wgt_eff:.3f}')

# I have an external package, 'pidplots', that interfaces with these DataFrames
# and provides many methods for quickly making plots of the PID performance.
