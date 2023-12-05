##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import torch


def speedup(y_true, filter_prob, retention_rate, balance_weights=None, from_logits=True,
            t_gen=0.08, t_NN=0.63, t_SR=97.04):
    """
    For sampling method,
    calculated by equalising statistical uncertainty with/without nn filter, processing times included.
    Use from_logits=True by default if receiving filter_prob directly from nn outputs.
    """
    if from_logits:
        filter_prob = torch.sigmoid(filter_prob)
    if balance_weights is None:
        # Set balance_weights to 1 by default if the dataset is balanced.
        balance_weights = torch.ones_like(torch.tensor(filter_prob))

    N_TP = (filter_prob * balance_weights)[y_true == 1].sum() * retention_rate            # Number of true positives
    N_FP = (filter_prob * balance_weights)[y_true == 0].sum() * (1 - retention_rate)      # Number of false positives
    N_TN = ((1 - filter_prob) * balance_weights)[y_true == 0].sum() * (1 - retention_rate)  # Number of true negatives
    N_FN = ((1 - filter_prob) * balance_weights)[y_true == 1].sum() * retention_rate      # Number of false negatives

    # Add up all types of events and multiply them by their processing times
    t_simulated_filter = (
        (N_TP + N_FP) * (t_gen + t_NN + t_SR)
        + (N_TN + N_FN) * (t_gen + t_NN)
    )

    sumw = (y_true * balance_weights).sum()
    sumw2 = (1 / filter_prob * balance_weights)[y_true == 1].sum()
    N_simulated_nofilter = (sumw ** 2) / sumw2

    t_simulated_nofilter = N_simulated_nofilter * (t_gen + t_SR)

    return t_simulated_filter / t_simulated_nofilter
