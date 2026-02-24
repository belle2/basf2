#!/usr/bin/env python3
"""
Training script for ModeSelector neural networks.

This script trains the two-stage ModeSelector:
1. Category network (B0 vs B+ vs continuum classification)
2. Main network (signal vs background classification using category output)

Usage:
    # Train category network
    python train.py --input modeSelector_training.npz --network category --output networks/

    # Train main network (requires trained category network)
    python train.py --input modeSelector_training.npz --network main --cat_model networks/net_cat.pt --output networks/

The script implements:
- FEI calibration sampling (decayModeID-based weights)
- Continuum downsampling (25% relative to BB)
- Fraction sampling (configurable, default 0.7)
- sigProb and Mbc preselection
"""

import argparse
import os

import numpy as np
import torch
import torch.nn as nn
import torch.optim as optim
from scipy import sparse
from torch.utils.data import DataLoader, TensorDataset
from tqdm import tqdm

# Import config
from . import config


def get_fei_calib(dm_id, pdg):
    """
    Get FEI calibration factor for a decay mode.

    Parameters
    ----------
    dm_id : int
        Decay mode ID
    pdg : int
        PDG code (521 for B+, 511 for B0)

    Returns
    -------
    float
        Calibration weight
    """
    if abs(pdg) == 521:
        return config.FEI_CALIB_BP.get(dm_id, config.FEI_CALIB_BP_REST)
    elif abs(pdg) == 511:
        return config.FEI_CALIB_B0.get(dm_id, config.FEI_CALIB_B0_REST)
    else:
        return 1.0  # Continuum


def compute_event_calib(bp_mc_truth, b0_mc_truth, mc_var_names, use_delta_p_good_tag=False,
                        delta_p_thresh=0.1):
    """
    Compute event calibration weights for FEI sampling (vectorized).

    Parameters
    ----------
    bp_mc_truth : ndarray, shape (n_events, n_vars)
        MC truth for best B+ candidate
    b0_mc_truth : ndarray, shape (n_events, n_vars)
        MC truth for best B0 candidate
    mc_var_names : list
        Names of MC truth variables
    use_delta_p_good_tag : bool
        If True, define good tags as mostcommonBTagDeltaP < delta_p_thresh.
        If False (default), use isSignalAcceptWrongFSPs | isSignalAcceptMissing.
    delta_p_thresh : float
        Threshold for mostcommonBTagDeltaP good-tag definition. Default: 0.1

    Returns
    -------
    event_calib_bp : ndarray
        Calibration weights for B+ candidates
    event_calib_b0 : ndarray
        Calibration weights for B0 candidates
    """
    n_events = len(bp_mc_truth)

    # Get column indices
    pdg_idx = list(mc_var_names).index('PDG')
    dm_idx = list(mc_var_names).index('extraInfo(decayModeID)')
    gen3_idx = list(mc_var_names).index('genParticle(3, varForMCGen(PDG))')
    gen4_idx = list(mc_var_names).index('genParticle(4, varForMCGen(PDG))')
    if use_delta_p_good_tag:
        delta_p_idx = list(mc_var_names).index('mostcommonBTagDeltaP')
    else:
        sig_acc_wrong_idx = list(mc_var_names).index('isSignalAcceptWrongFSPs')
        sig_acc_miss_idx = list(mc_var_names).index('isSignalAcceptMissing')

    # Initialize with default weight
    event_calib_bp = np.ones(n_events)
    event_calib_b0 = np.ones(n_events)

    # Create vectorized calibration lookup
    bp_calib_array = np.array([config.FEI_CALIB_BP.get(i, config.FEI_CALIB_BP_REST) for i in range(68)])
    b0_calib_array = np.array([config.FEI_CALIB_B0.get(i, config.FEI_CALIB_B0_REST) for i in range(68)])

    # Process B+ candidates
    has_bp = ~np.isnan(bp_mc_truth[:, pdg_idx])
    if has_bp.any():
        dm_ids = bp_mc_truth[has_bp, dm_idx].astype(int)

        if use_delta_p_good_tag:
            delta_p = bp_mc_truth[has_bp, delta_p_idx]
            tag_is_gen = ~np.isnan(delta_p) & (delta_p < delta_p_thresh)
        else:
            tag_is_gen = ((bp_mc_truth[has_bp, sig_acc_wrong_idx] == 1) |
                          (bp_mc_truth[has_bp, sig_acc_miss_idx] == 1))

        # Check if continuum (both gen particles are quarks)
        gen3 = np.where(np.isnan(bp_mc_truth[has_bp, gen3_idx]), 0, bp_mc_truth[has_bp, gen3_idx])
        gen4 = np.where(np.isnan(bp_mc_truth[has_bp, gen4_idx]), 0, bp_mc_truth[has_bp, gen4_idx])
        is_cont = (np.abs(gen3) < 10) & (np.abs(gen4) < 10)

        # Validate decayModeIDs
        if (dm_ids < 0).any() or (dm_ids > 67).any():
            raise ValueError(f"Invalid decayModeID found in B+ candidates: min={dm_ids.min()}, max={dm_ids.max()}")

        # Apply calibration: only for tag_is_gen and not continuum
        apply_calib = tag_is_gen & ~is_cont
        event_calib_bp[has_bp] = np.where(apply_calib, bp_calib_array[dm_ids], 1.0)

    # Process B0 candidates
    has_b0 = ~np.isnan(b0_mc_truth[:, pdg_idx])
    if has_b0.any():
        dm_ids = b0_mc_truth[has_b0, dm_idx].astype(int)

        if use_delta_p_good_tag:
            delta_p = b0_mc_truth[has_b0, delta_p_idx]
            tag_is_gen = ~np.isnan(delta_p) & (delta_p < delta_p_thresh)
        else:
            tag_is_gen = ((b0_mc_truth[has_b0, sig_acc_wrong_idx] == 1) |
                          (b0_mc_truth[has_b0, sig_acc_miss_idx] == 1))

        gen3 = np.where(np.isnan(b0_mc_truth[has_b0, gen3_idx]), 0, b0_mc_truth[has_b0, gen3_idx])
        gen4 = np.where(np.isnan(b0_mc_truth[has_b0, gen4_idx]), 0, b0_mc_truth[has_b0, gen4_idx])
        is_cont = (np.abs(gen3) < 10) & (np.abs(gen4) < 10)

        # Validate decayModeIDs
        if (dm_ids < 0).any() or (dm_ids > 67).any():
            raise ValueError(f"Invalid decayModeID found in B0 candidates: min={dm_ids.min()}, max={dm_ids.max()}")

        apply_calib = tag_is_gen & ~is_cont
        event_calib_b0[has_b0] = np.where(apply_calib, b0_calib_array[dm_ids], 1.0)

    return event_calib_bp, event_calib_b0


def load_and_sample_data(input_file, fraction=0.7, cont_fraction=0.25,
                         sigprob_thresh=0.01, mbc_thresh=5.23, random_state=None,
                         use_delta_p_good_tag=False, delta_p_thresh=0.1):
    """
    Load training data and apply sampling.

    Parameters
    ----------
    input_file : str
        Path to modeSelector_training.npz
    fraction : float
        Fraction of BB events to sample (default 0.7)
    cont_fraction : float
        Additional fraction for continuum (default 0.25)
    sigprob_thresh : float
        Minimum signal probability threshold
    mbc_thresh : float
        Minimum Mbc threshold
    random_state : int
        Random seed

    Returns
    -------
    features : sparse matrix
        Sampled and filtered feature matrix (only has_inputs columns)
    bp_truth : ndarray
        MC truth for B+ candidates
    b0_truth : ndarray
        MC truth for B0 candidates
    mc_var_names : list
        Names of MC truth variables
    has_inputs : list of int
        Selected feature indices (non-zero features, excluding Mbc)
    """
    if random_state is not None:
        np.random.seed(random_state)

    # Load data
    print(f"Loading {input_file}...")
    data = np.load(input_file, allow_pickle=True)
    features = sparse.load_npz(input_file.replace('.npz', '_features.npz'))

    bp_truth = data['bp_truth']
    b0_truth = data['b0_truth']
    mc_var_names = list(data['mc_var_names'])

    n_events = len(bp_truth)
    print(f"  Loaded {n_events} events, {features.shape[1]} features")

    # Get indices for MC truth variables
    sigprob_idx = mc_var_names.index('extraInfo(SignalProbability)')
    mbc_idx = mc_var_names.index('Mbc')
    gen3_idx = mc_var_names.index('genParticle(3, varForMCGen(PDG))')
    gen4_idx = mc_var_names.index('genParticle(4, varForMCGen(PDG))')

    # Compute event calibration weights
    print("Computing FEI calibration weights...")
    event_calib_bp, event_calib_b0 = compute_event_calib(bp_truth, b0_truth, mc_var_names,
                                                         use_delta_p_good_tag=use_delta_p_good_tag,
                                                         delta_p_thresh=delta_p_thresh)

    # Determine best candidate (highest sigProb between B+ and B0)
    bp_sigprob = bp_truth[:, sigprob_idx]
    b0_sigprob = b0_truth[:, sigprob_idx]

    # Handle NaN (no candidate of that type)
    bp_sigprob = np.where(np.isnan(bp_sigprob), -1, bp_sigprob)
    b0_sigprob = np.where(np.isnan(b0_sigprob), -1, b0_sigprob)

    bp_is_best = bp_sigprob > b0_sigprob

    # Get best candidate's properties for each event
    best_sigprob = np.where(bp_is_best, bp_sigprob, b0_sigprob)
    best_mbc = np.where(bp_is_best, bp_truth[:, mbc_idx], b0_truth[:, mbc_idx])
    event_calib = np.where(bp_is_best, event_calib_bp, event_calib_b0)

    # Determine if continuum (both gen particles are quarks)
    gen3 = np.where(bp_is_best, bp_truth[:, gen3_idx], b0_truth[:, gen3_idx])
    gen4 = np.where(bp_is_best, bp_truth[:, gen4_idx], b0_truth[:, gen4_idx])
    is_cont = (np.abs(gen3) < 10) & (np.abs(gen4) < 10)

    # Apply preselection
    print(f"Applying preselection (sigProb > {sigprob_thresh}, Mbc > {mbc_thresh})...")
    presel = (best_sigprob > sigprob_thresh) & (best_mbc > mbc_thresh)
    print(f"  {presel.sum()} / {n_events} events pass ({presel.mean() * 100:.1f}%)")

    # Apply sampling with FEI calibration and fraction
    print(f"Applying sampling (fraction={fraction}, cont_fraction={cont_fraction})...")
    sample_prob = event_calib * fraction
    sample_prob[is_cont] *= cont_fraction

    # Cap probabilities at 1.0
    sample_prob = np.minimum(sample_prob, 1.0)

    random_vals = np.random.random(n_events)
    sampled = (random_vals < sample_prob) & presel

    print(f"  {sampled.sum()} / {n_events} events sampled ({sampled.mean() * 100:.1f}%)")
    print(f"    BB events: {(sampled & ~is_cont).sum()}")
    print(f"    Continuum: {(sampled & is_cont).sum()}")

    # Apply sampling
    features = features[sampled]
    bp_truth = bp_truth[sampled]
    b0_truth = b0_truth[sampled]

    # Compute remove_inputs dynamically and verify against config.REMOVE_INPUTS
    print("\nVerifying REMOVE_INPUTS from feature sparsity...")

    n_total = features.shape[1]  # 1643

    # Stage 1: All-zero columns
    nonzero_cols = set(np.unique(features.nonzero()[1]))
    all_zero_cols = set(range(n_total)) - nonzero_cols

    # Stage 2: Add Mbc block (block i=10, indices 1360-1495)
    n_input_ids = config.N_INPUT_IDS  # 136
    mbc_start = n_input_ids * 10
    mbc_end = n_input_ids * 11
    mbc_cols = set(range(mbc_start, mbc_end))

    computed_remove = sorted(all_zero_cols | mbc_cols)
    print(f"  All-zero columns:  {len(all_zero_cols)}")
    print(f"  Mbc block [{mbc_start}, {mbc_end}): {len(mbc_cols)} columns")
    print(f"  Total to remove:   {len(computed_remove)}")

    # Verify against hardcoded list in config
    if computed_remove != config.REMOVE_INPUTS:
        print("  WARNING: REMOVE_INPUTS mismatch!")
        print(f"    Computed from data: {len(computed_remove)} indices removed ({n_total - len(computed_remove)} kept)")
        print(f"    config.REMOVE_INPUTS: {len(config.REMOVE_INPUTS)} indices removed ({n_total - len(config.REMOVE_INPUTS)} kept)")
        print("    Using config.REMOVE_INPUTS (data may not cover all input_ids)")
    else:
        print(f"  REMOVE_INPUTS verified OK ({len(config.REMOVE_INPUTS)} removed, "
              f"{n_total - len(config.REMOVE_INPUTS)} kept)")
    has_inputs = sorted(set(range(n_total)) - set(config.REMOVE_INPUTS))

    # Apply feature selection
    features = features[:, has_inputs]

    return (features, bp_truth, b0_truth, mc_var_names, has_inputs)


class MultiClassNet(nn.Module):
    """Multi-class classification network with fully connected layers.

    Architecture matches the offline training network:
    - 256 -> 128 (dropout 0.2) -> 64 (dropout 0.15) -> 32 (dropout 0.1) -> 16 -> num_labels
    - ReLU activation
    - Xavier initialization (gain=0.5, bias=0.01)
    """

    def __init__(self, input_size, num_labels=3):
        super().__init__()

        self.activation = nn.ReLU()

        self.network = nn.Sequential(
            nn.Linear(input_size, 256),
            self.activation,
            nn.Linear(256, 128),
            nn.Dropout(0.2),
            self.activation,
            nn.Linear(128, 64),
            nn.Dropout(0.15),
            self.activation,
            nn.Linear(64, 32),
            nn.Dropout(0.1),
            self.activation,
            nn.Linear(32, 16),
            self.activation,
            nn.Linear(16, num_labels)
        )

        self._init_weights()

    def _init_weights(self):
        """Initialize weights with Xavier uniform (gain=0.5) and bias=0.01."""
        for m in self.modules():
            if isinstance(m, nn.Linear):
                nn.init.xavier_uniform_(m.weight, gain=0.5)
                nn.init.constant_(m.bias, 0.01)

    def forward(self, x):
        return self.network(x)


def train_epoch(model, train_loader, criterion, optimizer, device, disco_lambda=0.0):
    """
    Train for one epoch with optional distance correlation penalty.

    Parameters
    ----------
    model : nn.Module
        Model to train
    train_loader : DataLoader
        Training data loader. When disco_lambda > 0, batches must be 3-tuples
        (features, labels, mbc_values); otherwise 2-tuples (features, labels).
    criterion : nn.Module
        Loss function
    optimizer : torch.optim.Optimizer
        Optimizer
    device : torch.device
        Device to use
    disco_lambda : float
        Coefficient for distance correlation loss (0 = disabled)

    Returns
    -------
    train_loss : float
        Average classification loss
    disco_loss : float
        Average distance correlation loss (0.0 if disabled)
    """
    model.train()
    total_loss = 0
    total_disco = 0
    n_batches = 0

    for batch in train_loader:
        # Unpack batch - 3-tuple when DisCo enabled, 2-tuple otherwise
        if len(batch) == 3:
            data, target, batch_mbc = batch
            batch_mbc = batch_mbc.to(device)
        else:
            data, target = batch
            batch_mbc = None

        data, target = data.to(device), target.to(device)

        optimizer.zero_grad()
        output = model(data)

        # Classification loss
        cls_loss = criterion(output, target)

        # Distance correlation loss (if enabled)
        disco_loss = torch.tensor(0.0, device=device)
        if disco_lambda > 0 and batch_mbc is not None:
            probs = torch.softmax(output, dim=1)

            if probs.shape[1] == 6:  # main network
                combined_output = -(probs[:, 5] - probs[:, 1])

                # Continuum class (4)
                cont_mask = target == 4
                if cont_mask.sum() > 1:
                    disco_loss = disco_loss + disco_lambda * distance_corr(
                        combined_output[cont_mask], batch_mbc[cont_mask]
                    )

                # Crossfeed internal class (3)
                cross_mask = target == 3
                if cross_mask.sum() > 1:
                    disco_loss = disco_loss + 2 * disco_lambda * distance_corr(
                        combined_output[cross_mask], batch_mbc[cross_mask]
                    )

        loss = cls_loss + disco_loss
        loss.backward()

        # Gradient clipping for stability
        torch.nn.utils.clip_grad_norm_(model.parameters(), max_norm=1.0)

        optimizer.step()

        total_loss += cls_loss.item() * len(data)
        total_disco += disco_loss.item()
        n_batches += 1

    return (total_loss / len(train_loader.dataset),
            total_disco / n_batches if n_batches > 0 else 0.0)


def evaluate(model, loader, criterion, device):
    """Evaluate model. Handles both 2-tuple and 3-tuple batches (3rd element ignored)."""
    model.eval()
    total_loss = 0

    with torch.no_grad():
        for batch in loader:
            data, target = batch[0].to(device), batch[1].to(device)
            output = model(data)
            loss = criterion(output, target)
            total_loss += loss.item() * len(data)

    return total_loss / len(loader.dataset)


def distance_corr(var_1, var_2, normedweight=None, power=1):
    """
    Compute distance correlation between var_1 and var_2.

    Based on /home/pf/IJS/feq/training/train.py lines 604-661.

    The distance correlation is a measure of dependence between two random variables.
    It is zero if and only if the variables are independent.

    Parameters
    ----------
    var_1 : torch.Tensor, shape (n,)
        First variable (e.g., Mbc)
    var_2 : torch.Tensor, shape (n,)
        Second variable (e.g., classifier output)
    normedweight : torch.Tensor, optional, shape (n,)
        Per-example weight (should sum to n). If None, uses uniform weights.
    power : int
        Exponent for distance correlation (default 1)

    Returns
    -------
    torch.Tensor (scalar)
        Distance correlation coefficient
    """
    n = len(var_1)

    # Handle normedweight - create uniform weights if not provided
    if normedweight is None:
        normedweight = torch.ones(n, device=var_1.device, dtype=var_1.dtype)
    elif isinstance(normedweight, (int, float)):
        normedweight = normedweight * torch.ones(n, device=var_1.device, dtype=var_1.dtype)

    # Compute pairwise distance matrices
    amat = torch.abs(var_1.unsqueeze(1) - var_1.unsqueeze(0))
    bmat = torch.abs(var_2.unsqueeze(1) - var_2.unsqueeze(0))

    # Compute row averages (weighted by normedweight across columns)
    amatavg = torch.mean(amat * normedweight.unsqueeze(0), dim=1)
    bmatavg = torch.mean(bmat * normedweight.unsqueeze(0), dim=1)

    # Double centering
    Amat = (amat
            - amatavg.unsqueeze(1)
            - amatavg.unsqueeze(0)
            + torch.mean(amatavg * normedweight))

    Bmat = (bmat
            - bmatavg.unsqueeze(1)
            - bmatavg.unsqueeze(0)
            + torch.mean(bmatavg * normedweight))

    # Compute final statistics
    ABavg = torch.mean(Amat * Bmat * normedweight.unsqueeze(0), dim=1)
    AAavg = torch.mean(Amat * Amat * normedweight.unsqueeze(0), dim=1)
    BBavg = torch.mean(Bmat * Bmat * normedweight.unsqueeze(0), dim=1)

    # Compute correlation based on power
    if power == 1:
        dCorr = (torch.mean(ABavg * normedweight) /
                 torch.sqrt(torch.mean(AAavg * normedweight) *
                            torch.mean(BBavg * normedweight)))
    elif power == 2:
        dCorr = (torch.mean(ABavg * normedweight)**2 /
                 (torch.mean(AAavg * normedweight) *
                  torch.mean(BBavg * normedweight)))
    else:
        dCorr = ((torch.mean(ABavg * normedweight) /
                  torch.sqrt(torch.mean(AAavg * normedweight) *
                             torch.mean(BBavg * normedweight)))**power)

    return dCorr


def build_main_labels(bp_truth, b0_truth, mc_var_names, use_delta_p_good_tag=False,
                      delta_p_thresh=0.1):
    """
    Build main network labels (6 classes) from MC truth.

    Classes (based on /home/pf/IJS/feq/training/training.py lines 386-420):
    - 0: bad_tag (background)
    - 1: is_target_neutral (signal B0)
    - 2: cross_deltaC1 (BB crossfeed, ΔC=1 - different B type)
    - 3: cross_internal (BB crossfeed, internal - same B type)
    - 4: continuum
    - 5: is_target_charged (signal B+)

    Logic:
    - Use category network logic to determine best candidate (B0 vs B+)
    - For B0 candidates: assign labels 0-4 directly
    - For B+ candidates: assign labels 0-4, but map is_target (1) -> 5

    Parameters
    ----------
    bp_truth : ndarray
        MC truth for B+ candidates
    b0_truth : ndarray
        MC truth for B0 candidates
    mc_var_names : list
        Names of MC truth variables
    use_delta_p_good_tag : bool
        If True, define good tags as mostcommonBTagDeltaP < delta_p_thresh.
        If False (default), use isSignalAcceptWrongFSPs | isSignalAcceptMissing.
    delta_p_thresh : float
        Threshold for mostcommonBTagDeltaP good-tag definition. Default: 0.1

    Returns
    -------
    labels : ndarray
        Main network labels (0-5)
    """

    # Get column indices
    pdg_idx = mc_var_names.index('PDG')
    sigprob_idx = mc_var_names.index('extraInfo(SignalProbability)')
    gen3_idx = mc_var_names.index('genParticle(3, varForMCGen(PDG))')
    gen4_idx = mc_var_names.index('genParticle(4, varForMCGen(PDG))')
    if use_delta_p_good_tag:
        delta_p_idx = mc_var_names.index('mostcommonBTagDeltaP')
    else:
        sig_acc_wrong_idx = mc_var_names.index('isSignalAcceptWrongFSPs')
        sig_acc_miss_idx = mc_var_names.index('isSignalAcceptMissing')

    # Compute labels for B0 and B+ candidates separately
    def compute_output_label(truth_array):
        """Compute output label (0-4) for a single candidate type."""
        n = len(truth_array)
        labels = np.zeros(n, dtype=np.int64)

        # Extract truth info (handle NaN for missing candidates)
        pdg = truth_array[:, pdg_idx]
        gen3 = np.where(np.isnan(truth_array[:, gen3_idx]), 0, truth_array[:, gen3_idx])
        gen4 = np.where(np.isnan(truth_array[:, gen4_idx]), 0, truth_array[:, gen4_idx])

        # Determine event type
        if use_delta_p_good_tag:
            delta_p = truth_array[:, delta_p_idx]
            is_target = ~np.isnan(delta_p) & (delta_p < delta_p_thresh)
        else:
            is_signal_wrong = np.where(np.isnan(truth_array[:, sig_acc_wrong_idx]), 0, truth_array[:, sig_acc_wrong_idx])
            is_signal_miss = np.where(np.isnan(truth_array[:, sig_acc_miss_idx]), 0, truth_array[:, sig_acc_miss_idx])
            is_target = (is_signal_wrong == 1) | (is_signal_miss == 1)
        is_cont = (np.abs(gen3) < 10) & (np.abs(gen4) < 10)

        # For BB events (not continuum), check crossfeed
        # NOTE: For crossfeed, we'd need mostcommonBTagPDG to determine deltaC=1 vs internal
        # Since we don't have that in the simplified structure, we use isBBCrossfeed
        # and assume that different B types = deltaC1, same B type = internal
        has_candidate = ~np.isnan(pdg)
        if 'isBBCrossfeed' in mc_var_names:
            crossfeed_idx = mc_var_names.index('isBBCrossfeed')
            is_crossfeed = np.where(np.isnan(truth_array[:, crossfeed_idx]), 0, truth_array[:, crossfeed_idx])
            is_crossfeed = is_crossfeed == 1
        else:
            # Fallback: not target and not continuum = crossfeed
            is_crossfeed = ~is_target & ~is_cont & has_candidate

        # For proper deltaC1 vs internal, we need mostcommonBTagPDG
        # Simplified: assume crossfeed within same B type is internal
        # For now, put all crossfeed in cross_internal (class 3)
        # This can be refined if we add mostcommonBTagPDG to the truth

        # Assign labels (default 0 = bad_tag)
        labels[is_target] = 1  # is_target
        labels[is_crossfeed & ~is_target] = 3  # cross_internal (simplified)
        labels[is_cont] = 4  # continuum

        return labels

    # Compute labels for both candidate types
    b0_labels = compute_output_label(b0_truth)
    bp_labels = compute_output_label(bp_truth)

    # Determine best candidate (highest sigProb)
    bp_sigprob = np.where(np.isnan(bp_truth[:, sigprob_idx]), -1, bp_truth[:, sigprob_idx])
    b0_sigprob = np.where(np.isnan(b0_truth[:, sigprob_idx]), -1, b0_truth[:, sigprob_idx])
    bp_is_best = bp_sigprob > b0_sigprob

    # Main network label logic:
    # - For neutral predicted (B0 best): use B0 labels (0-4)
    # - For charged predicted (B+ best): use B+ labels, but is_target (1) -> 5
    labels = np.where(bp_is_best, bp_labels, b0_labels)

    # Map is_target for charged candidates to class 5
    is_target_charged = bp_is_best & (bp_labels == 1)
    labels[is_target_charged] = 5

    return labels


class SparseDataset(torch.utils.data.Dataset):
    """Dataset for sparse feature matrices.

    Converts each row to dense on-the-fly during batching.
    Slower than dense loading but significantly more memory-efficient.
    """

    def __init__(self, features_sparse, labels, mbc_values=None):
        """
        Parameters
        ----------
        features_sparse : scipy.sparse matrix, shape (n_samples, n_features)
            Sparse feature matrix (CSR format recommended)
        labels : np.ndarray, shape (n_samples,)
            Target labels
        mbc_values : np.ndarray, optional, shape (n_samples,)
            Mbc values per event (for DisCo loss)
        """
        self.features = features_sparse.tocsr()
        self.labels = labels
        self.mbc_values = mbc_values

    def __len__(self):
        return self.features.shape[0]

    def __getitem__(self, idx):
        # Convert sparse row to dense 1D tensor
        feature_row = torch.from_numpy(
            self.features[idx].toarray().astype(np.float32).squeeze()
        )
        label = torch.tensor(self.labels[idx], dtype=torch.long)

        if self.mbc_values is not None:
            mbc = torch.tensor(self.mbc_values[idx], dtype=torch.float32)
            return feature_row, label, mbc

        return feature_row, label


def sparse_collate_fn(batch):
    """Collate function for SparseDataset with optional Mbc values."""
    if len(batch[0]) == 3:
        features, labels, mbc = zip(*batch)
        return torch.stack(features), torch.stack(labels), torch.stack(mbc)
    else:
        features, labels = zip(*batch)
        return torch.stack(features), torch.stack(labels)


def build_category_labels(bp_truth, b0_truth, mc_var_names):
    """
    Build category labels (B0=0, B+=1, continuum=2) from MC truth.

    Parameters
    ----------
    bp_truth : ndarray
        MC truth for B+ candidates
    b0_truth : ndarray
        MC truth for B0 candidates
    mc_var_names : list
        Names of MC truth variables

    Returns
    -------
    labels : ndarray
        Category labels (0=B0, 1=B+, 2=continuum)
    """
    n_events = len(bp_truth)

    # Get column indices
    pdg_idx = mc_var_names.index('PDG')
    sigprob_idx = mc_var_names.index('extraInfo(SignalProbability)')
    gen3_idx = mc_var_names.index('genParticle(3, varForMCGen(PDG))')
    gen4_idx = mc_var_names.index('genParticle(4, varForMCGen(PDG))')

    # Determine best candidate (highest sigProb)
    bp_sigprob = np.where(np.isnan(bp_truth[:, sigprob_idx]), -1, bp_truth[:, sigprob_idx])
    b0_sigprob = np.where(np.isnan(b0_truth[:, sigprob_idx]), -1, b0_truth[:, sigprob_idx])
    bp_is_best = bp_sigprob > b0_sigprob

    # Determine if continuum (both gen particles are quarks)
    gen3 = np.where(bp_is_best, bp_truth[:, gen3_idx], b0_truth[:, gen3_idx])
    gen4 = np.where(bp_is_best, bp_truth[:, gen4_idx], b0_truth[:, gen4_idx])
    gen3 = np.where(np.isnan(gen3), 0, gen3)
    gen4 = np.where(np.isnan(gen4), 0, gen4)
    is_cont = (np.abs(gen3) < 10) & (np.abs(gen4) < 10)

    # Build labels
    labels = np.zeros(n_events, dtype=np.int64)
    labels[is_cont] = 2  # continuum

    # For BB events, determine B0 vs B+
    bb_mask = ~is_cont
    best_pdg = np.where(bp_is_best, bp_truth[:, pdg_idx], b0_truth[:, pdg_idx])
    is_charged = np.abs(best_pdg) == 521

    labels[bb_mask & is_charged] = 1  # B+
    labels[bb_mask & ~is_charged] = 0  # B0

    return labels


def main():
    parser = argparse.ArgumentParser(description='Train ModeSelector networks')
    parser.add_argument('--input', required=True, help='Input npz file from training mode')
    parser.add_argument('--network', choices=['category', 'main'], required=True,
                        help='Which network to train')
    parser.add_argument('--cat_model', help='Trained category model (required for main network)')
    parser.add_argument('--output', default='networks/', help='Output directory for trained models')
    parser.add_argument('--fraction', type=float, default=0.7, help='Sampling fraction for BB events')
    parser.add_argument('--cont_fraction', type=float, default=0.25, help='Additional fraction for continuum')
    parser.add_argument('--batch_size', type=int, default=8192, help='Batch size')
    parser.add_argument('--epochs', type=int, default=40, help='Number of epochs')
    parser.add_argument('--lr', type=float, default=0.0001, help='Learning rate')
    parser.add_argument('--weight_decay', type=float, default=1e-4, help='Weight decay for AdamW')
    parser.add_argument('--val_split', type=float, default=0.1, help='Validation split')
    parser.add_argument('--seed', type=int, default=42, help='Random seed')
    parser.add_argument('--disco_lambda', type=float, default=0.0,
                        help='Distance correlation penalty coefficient (0=disabled)')
    parser.add_argument('--use_sparse', action='store_true',
                        help='Use sparse data loading (memory-efficient but slower)')
    parser.add_argument('--use_delta_p_good_tag', action='store_true',
                        help='Define good tags as mostcommonBTagDeltaP < 0.1 instead of '
                             'isSignalAcceptWrongFSPs | isSignalAcceptMissing')
    parser.add_argument('--delta_p_thresh', type=float, default=0.1,
                        help='Threshold for mostcommonBTagDeltaP good-tag definition (default: 0.1)')

    args = parser.parse_args()

    # Set random seeds
    np.random.seed(args.seed)
    torch.manual_seed(args.seed)

    # Device
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    print(f"Using device: {device}")

    # Create output directory
    os.makedirs(args.output, exist_ok=True)

    # Load and sample data
    print("\n" + "=" * 60)
    print("Loading and sampling data")
    print("=" * 60)
    features, bp_truth, b0_truth, mc_var_names, has_inputs = load_and_sample_data(
        args.input,
        fraction=args.fraction,
        cont_fraction=args.cont_fraction,
        random_state=args.seed,
        use_delta_p_good_tag=args.use_delta_p_good_tag,
        delta_p_thresh=args.delta_p_thresh,
    )

    print(f"\nFeature matrix shape: {features.shape}")
    print(f"  Sparse matrix memory: {features.data.nbytes / 1024**2:.1f} MB")
    print(f"  Selected features (has_inputs): {len(has_inputs)}")

    # Build labels
    print("\nBuilding labels...")
    if args.network == 'category':
        labels = build_category_labels(bp_truth, b0_truth, mc_var_names)
        num_labels = 3
        print("  Category distribution:")
        print(f"    B0:        {(labels == 0).sum()} ({(labels == 0).mean() * 100:.1f}%)")
        print(f"    B+:        {(labels == 1).sum()} ({(labels == 1).mean() * 100:.1f}%)")
        print(f"    Continuum: {(labels == 2).sum()} ({(labels == 2).mean() * 100:.1f}%)")

        input_size = features.shape[1]

        if not args.use_sparse:
            # Convert to dense for training
            print("\nConverting to dense arrays...")
            features_dense = features.toarray().astype(np.float32)
        else:
            print("\nUsing sparse data loading (memory-efficient)...")
            features_dense = None  # Will use SparseDataset below

    else:  # main network
        if not args.cat_model:
            raise ValueError("--cat_model required for main network training")

        # Load trained category model
        print(f"\nLoading category model from {args.cat_model}...")
        cat_checkpoint = torch.load(args.cat_model)
        cat_model = MultiClassNet(
            input_size=features.shape[1],
            num_labels=3
        )
        cat_model.load_state_dict(cat_checkpoint['model_state_dict'])
        cat_model = cat_model.to(device)
        cat_model.eval()
        print(f"  Category model loaded (epoch {cat_checkpoint['epoch']+1})")

        # Convert to dense for category prediction
        print("\nConverting to dense arrays...")
        features_dense = features.toarray().astype(np.float32)

        # Generate category outputs for all samples
        print("\nGenerating category outputs...")
        cat_outputs = []
        with torch.no_grad():
            for i in tqdm(range(0, len(features_dense), args.batch_size),
                          desc="  Processing batches"):
                batch = torch.from_numpy(
                    features_dense[i:i + args.batch_size]
                ).to(device)
                cat_out = torch.softmax(cat_model(batch), dim=1)
                cat_outputs.append(cat_out.cpu().numpy())
        cat_outputs = np.vstack(cat_outputs)

        print(f"  Category outputs shape: {cat_outputs.shape}")
        print("  Category predictions:")
        cat_preds = np.argmax(cat_outputs, axis=1)
        print(f"    B0:        {(cat_preds == 0).sum()} ({(cat_preds == 0).mean() * 100:.1f}%)")
        print(f"    B+:        {(cat_preds == 1).sum()} ({(cat_preds == 1).mean() * 100:.1f}%)")
        print(f"    Continuum: {(cat_preds == 2).sum()} ({(cat_preds == 2).mean() * 100:.1f}%)")

        # Compute charged_cat flag: 1 if B+ is predicted over B0 (matches ModeSelectorModule inference)
        charged_cat = (cat_outputs[:, 1] > cat_outputs[:, 0]).astype(np.float32)

        # Concatenate category outputs + charged_cat flag to features
        print("\nConcatenating category outputs to features...")
        features_with_cat = np.hstack([features_dense, cat_outputs, charged_cat.reshape(-1, 1)])
        features_dense = features_with_cat
        input_size = features_dense.shape[1]
        print(f"  New feature shape: {features_dense.shape}")

        # Build main network labels
        labels = build_main_labels(bp_truth, b0_truth, mc_var_names,
                                   use_delta_p_good_tag=args.use_delta_p_good_tag,
                                   delta_p_thresh=args.delta_p_thresh)
        num_labels = 6
        print("  Main network label distribution:")
        print(f"    bad_tag:           {(labels == 0).sum()} ({(labels == 0).mean() * 100:.1f}%)")
        print(f"    is_target_neutral: {(labels == 1).sum()} ({(labels == 1).mean() * 100:.1f}%)")
        print(f"    cross_deltaC1:     {(labels == 2).sum()} ({(labels == 2).mean() * 100:.1f}%)")
        print(f"    cross_internal:    {(labels == 3).sum()} ({(labels == 3).mean() * 100:.1f}%)")
        print(f"    continuum:         {(labels == 4).sum()} ({(labels == 4).mean() * 100:.1f}%)")
        print(f"    is_target_charged: {(labels == 5).sum()} ({(labels == 5).mean() * 100:.1f}%)")

    # Extract Mbc values for DisCo loss (if enabled)
    mbc_values = None
    if args.disco_lambda > 0:
        print("\nExtracting Mbc values for distance correlation...")
        mbc_idx = mc_var_names.index('Mbc')
        # Get Mbc from best candidate (same logic as labels)
        bp_sigprob = np.where(np.isnan(bp_truth[:, mc_var_names.index('extraInfo(SignalProbability)')]), -1,
                              bp_truth[:, mc_var_names.index('extraInfo(SignalProbability)')])
        b0_sigprob = np.where(np.isnan(b0_truth[:, mc_var_names.index('extraInfo(SignalProbability)')]), -1,
                              b0_truth[:, mc_var_names.index('extraInfo(SignalProbability)')])
        bp_is_best = bp_sigprob > b0_sigprob
        mbc_array = np.where(bp_is_best, bp_truth[:, mbc_idx], b0_truth[:, mbc_idx])
        mbc_values = torch.from_numpy(mbc_array.astype(np.float32))
        print(f"  Mbc values extracted: {len(mbc_values)}")

    # Train/val split
    print(f"\nSplitting train/val (val_split={args.val_split})...")
    n_events = features.shape[0] if features_dense is None else len(features_dense)
    n_val = int(n_events * args.val_split)
    n_train = n_events - n_val

    indices = np.random.permutation(n_events)
    train_idx = indices[:n_train]
    val_idx = indices[n_train:]

    # Split Mbc values
    if mbc_values is not None:
        mbc_train = mbc_values[train_idx]
        mbc_val = mbc_values[val_idx]
    else:
        mbc_train = None
        mbc_val = None

    print(f"  Train: {n_train} events")
    print(f"  Val:   {n_val} events")

    # Create dataloaders (sparse or dense)
    if features_dense is None:
        # Sparse loading: category network only
        mbc_train_np = mbc_train.numpy() if mbc_train is not None else None
        mbc_val_np = mbc_val.numpy() if mbc_val is not None else None
        train_dataset = SparseDataset(features[train_idx], labels[train_idx], mbc_train_np)
        val_dataset = SparseDataset(features[val_idx], labels[val_idx], mbc_val_np)
        train_loader = DataLoader(train_dataset, batch_size=args.batch_size, shuffle=True,
                                  collate_fn=sparse_collate_fn, num_workers=4)
        val_loader = DataLoader(val_dataset, batch_size=args.batch_size, shuffle=False,
                                collate_fn=sparse_collate_fn, num_workers=4)
    else:
        # Dense loading (default)
        X_train = torch.from_numpy(features_dense[train_idx])
        y_train = torch.from_numpy(labels[train_idx])
        X_val = torch.from_numpy(features_dense[val_idx])
        y_val = torch.from_numpy(labels[val_idx])

        if mbc_train is not None:
            # Include mbc in dataset so shuffling aligns correctly with DisCo
            train_dataset = TensorDataset(X_train, y_train, mbc_train)
            val_dataset = TensorDataset(X_val, y_val, mbc_val)
        else:
            train_dataset = TensorDataset(X_train, y_train)
            val_dataset = TensorDataset(X_val, y_val)

        train_loader = DataLoader(train_dataset, batch_size=args.batch_size, shuffle=True)
        val_loader = DataLoader(val_dataset, batch_size=args.batch_size, shuffle=False)

    # Create model
    print("\n" + "=" * 60)
    print("Creating model")
    print("=" * 60)
    model = MultiClassNet(input_size=input_size, num_labels=num_labels)
    model = model.to(device)

    print(f"  Input size: {input_size}")
    print(f"  Num labels: {num_labels}")
    print(f"  Parameters: {sum(p.numel() for p in model.parameters()):,}")

    # Loss and optimizer (matching offline training)
    criterion = nn.CrossEntropyLoss()
    optimizer = optim.AdamW(model.parameters(), lr=args.lr, weight_decay=args.weight_decay)
    scheduler = optim.lr_scheduler.ReduceLROnPlateau(
        optimizer, mode='min', factor=0.5, patience=3, verbose=True
    )

    # Training loop
    print("\n" + "=" * 60)
    print("Training")
    print("=" * 60)
    if args.disco_lambda > 0:
        print(f"Using distance correlation with lambda={args.disco_lambda}")

    best_val_loss = float('inf')
    best_epoch = 0

    for epoch in range(args.epochs):
        # Train
        train_loss, disco_loss = train_epoch(
            model, train_loader, criterion, optimizer, device,
            disco_lambda=args.disco_lambda
        )

        # Validate
        val_loss = evaluate(model, val_loader, criterion, device)

        # Scheduler step
        scheduler.step(val_loss)

        # Print progress
        if args.disco_lambda > 0:
            print(f"Epoch {epoch+1:3d}/{args.epochs}: "
                  f"train_loss={train_loss:.6f}, disco_loss={disco_loss:.6f}, "
                  f"val_loss={val_loss:.6f}")
        else:
            print(f"Epoch {epoch+1:3d}/{args.epochs}: "
                  f"train_loss={train_loss:.6f}, val_loss={val_loss:.6f}")

        # Save best model
        if val_loss < best_val_loss:
            best_val_loss = val_loss
            best_epoch = epoch
            model_path = os.path.join(args.output, f'net_{args.network}.pt')
            torch.save({
                'epoch': epoch,
                'model_state_dict': model.state_dict(),
                'optimizer_state_dict': optimizer.state_dict(),
                'val_loss': val_loss,
                'train_loss': train_loss,
                'has_inputs': has_inputs,
                'config': {
                    'input_size': input_size,
                    'num_labels': num_labels,
                    'fraction': args.fraction,
                    'cont_fraction': args.cont_fraction,
                    'network_type': args.network,
                }
            }, model_path)
            print(f"  -> Saved best model (val_loss={val_loss:.6f})")

    print("\n" + "=" * 60)
    print("Training complete")
    print("=" * 60)
    print(f"Best epoch: {best_epoch+1}")
    print(f"Best val loss: {best_val_loss:.6f}")
    print(f"Model saved to: {os.path.join(args.output, f'net_{args.network}.pt')}")

    # Evaluation on validation set
    print("\n" + "=" * 60)
    print("Evaluation on validation set")
    print("=" * 60)

    # Load best model for evaluation
    best_checkpoint = torch.load(os.path.join(args.output, f'net_{args.network}.pt'))
    model.load_state_dict(best_checkpoint['model_state_dict'])
    model.eval()

    val_probs_list = []
    val_labels_list = []

    with torch.no_grad():
        for batch in val_loader:
            data = batch[0].to(device)
            target = batch[1]
            output = model(data)
            probs = torch.softmax(output, dim=1)
            val_probs_list.append(probs.cpu().numpy())
            val_labels_list.append(target.numpy())

    val_probs = np.vstack(val_probs_list)
    val_labels = np.concatenate(val_labels_list)

    # Per-class metrics
    if args.network == 'category':
        class_names = ['B0', 'B+', 'continuum']
    else:
        class_names = ['bad_tag', 'is_target_neutral', 'cross_deltaC1',
                       'cross_internal', 'continuum', 'is_target_charged']

    print(f"\n  {'Class':<22} {'N':>8} {'Accuracy':>10} {'Mean P':>10} {'Median P':>10}")
    print(f"  {'-'*62}")
    for cls_idx, cls_name in enumerate(class_names):
        cls_mask = val_labels == cls_idx
        n_cls = cls_mask.sum()
        if n_cls > 0:
            cls_probs = val_probs[cls_mask, cls_idx]
            pred_cls = np.argmax(val_probs[cls_mask], axis=1)
            accuracy = (pred_cls == cls_idx).mean()
            print(f"  {cls_name:<22} {n_cls:>8,} {accuracy:>10.4f} "
                  f"{cls_probs.mean():>10.4f} {np.median(cls_probs):>10.4f}")

    # Overall accuracy
    overall_acc = (np.argmax(val_probs, axis=1) == val_labels).mean()
    print(f"\n  Overall accuracy: {overall_acc:.4f}")


if __name__ == '__main__':
    main()
