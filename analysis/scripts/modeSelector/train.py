#!/usr/bin/env python3
"""
Training script for ModeSelector neural networks.

This script trains the two-stage ModeSelector:
1. Category network (B0 vs B+ vs continuum classification)
2. Main network (signal vs background classification using category output)

Usage:
    # Train category network (single file)
    python train.py --input modeSelector_training.npz --network category --output networks/

    # Train category network (multiple files)
    python train.py --input modeSelector_training_*.npz --network category --output networks/

    # Train main network (requires trained category network)
    python train.py --input modeSelector_training.npz --network main --cat_model networks/net_cat.pt --output networks/

The script implements:
- FEI calibration sampling (decayModeID-based weights)
- Continuum downsampling (applied at production by produceTrainingInputs.py; --cont_fraction defaults to 1.0)
- Fraction sampling (configurable, default 1.0)
- sigProb preselection (applied to all events including continuum)
"""

import argparse
import glob
import os
import time
from concurrent.futures import ThreadPoolExecutor

import numpy as np
import torch
import torch.nn as nn
import torch.optim as optim
from modeSelector import config
from scipy import sparse
from torch.utils.data import DataLoader, TensorDataset
from tqdm import tqdm

MAIN_BG_BAD_TAG = config.N_INPUT_IDS
MAIN_BG_CROSS_DC1 = config.N_INPUT_IDS + 1
MAIN_BG_CONT = config.N_INPUT_IDS + 2
MAIN_NUM_LABELS = config.N_INPUT_IDS + 3


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


def compute_event_calib(best_bp_sigprob_iid, best_b0_sigprob_iid,
                        bp_tag_is_gen, b0_tag_is_gen, is_cont):
    """
    Compute event calibration weights for FEI sampling (vectorized).

    Calibration factors are applied to events where the best-sigProb
    candidate's mostcommonBTagPDG == PDG and the event is not continuum.

    Parameters
    ----------
    best_bp_sigprob_iid : ndarray of int16
        input_id of best-sigProb B+ candidate; -1 if absent
    best_b0_sigprob_iid : ndarray of int16
        input_id of best-sigProb B0 candidate; -1 if absent
    bp_tag_is_gen : ndarray of int8
        1 if best-sigProb B+ has mostcommonBTagPDG == PDG, else 0
    b0_tag_is_gen : ndarray of int8
        1 if best-sigProb B0 has mostcommonBTagPDG == PDG, else 0
    is_cont : ndarray of int8
        1 if event is continuum, else 0

    Returns
    -------
    event_calib_bp : ndarray
        Calibration weights for B+ candidates
    event_calib_b0 : ndarray
        Calibration weights for B0 candidates
    """
    n_events = len(best_bp_sigprob_iid)
    bp_threshold = config.N_BP_MODES * 2

    bp_calib_arr = np.array([config.FEI_CALIB_BP.get(i, config.FEI_CALIB_BP_REST)
                             for i in range(config.N_BP_MODES)])
    b0_calib_arr = np.array([config.FEI_CALIB_B0.get(i, config.FEI_CALIB_B0_REST)
                             for i in range(config.N_B0_MODES)])

    event_calib_bp = np.ones(n_events)
    apply_bp = (best_bp_sigprob_iid >= 0) & (bp_tag_is_gen == 1) & (is_cont == 0)
    if apply_bp.any():
        dm_ids = (best_bp_sigprob_iid[apply_bp].astype(np.int32) % bp_threshold) // 2
        event_calib_bp[apply_bp] = bp_calib_arr[dm_ids]

    event_calib_b0 = np.ones(n_events)
    apply_b0 = (best_b0_sigprob_iid >= 0) & (b0_tag_is_gen == 1) & (is_cont == 0)
    if apply_b0.any():
        dm_ids = (best_b0_sigprob_iid[apply_b0].astype(np.int32) - bp_threshold) // 2
        event_calib_b0[apply_b0] = b0_calib_arr[dm_ids]

    return event_calib_bp, event_calib_b0


def load_and_sample_data(input_files, fraction=1.0, cont_fraction=1.0,
                         sigprob_thresh=0.01, random_state=None):
    """
    Load training data and apply sampling.

    Parameters
    ----------
    input_files : str or list of str
        Path(s) to modeSelector_training.npz file(s)
    fraction : float
        Base sampling fraction for all events (default 1.0); continuum is further scaled by cont_fraction
    cont_fraction : float
        Additional downscale for continuum events (default 1.0, i.e. no extra downsampling;
        continuum is already downsampled at production time by produceTrainingInputs.py)
    sigprob_thresh : float
        Minimum signal probability threshold
    random_state : int
        Random seed

    Returns
    -------
    features : sparse matrix
        Sampled and filtered feature matrix (only has_inputs columns)
    event_scalars : tuple (is_cont, gen_pdg, bp_is_best, best_sigprob,
                           best_bp_sigprob_iid, best_b0_sigprob_iid)
        Per-event compact MC truth scalars; int8/int16/float32 arrays of shape (n_events,).
    has_inputs : list of int
        Selected feature indices (non-zero features, excluding Mbc)
    mc_truth_cand : tuple (best_bp_iid, best_bp_dp, best_b0_iid, best_b0_dp)
        Per-event arrays of shape (n_events,). best_bp_iid/b0_iid are int16 with
        sentinel -1 when no qualifying candidate exists; best_bp_dp/b0_dp are float32
        with sentinel inf. Pre-filtered: gen_pdg == pdg and is_cont != 1.
    """
    if isinstance(input_files, str):
        input_files = [input_files]

    # Expand any glob patterns and exclude _features.npz files
    expanded = []
    for f in input_files:
        matches = glob.glob(f)
        expanded.extend(matches if matches else [f])
    input_files = [f for f in expanded if not f.endswith('_features.npz')]

    if not input_files:
        raise ValueError("No input files found.")

    if fraction > 1.0:
        raise ValueError(f"fraction must be <= 1.0, got {fraction}.")

    if cont_fraction > 1.0:
        raise ValueError(f"cont_fraction must be <= 1.0, got {cont_fraction}.")

    rng = np.random.default_rng(random_state)
    # Pre-generate per-file seeds so parallel workers are independent
    seeds = rng.integers(0, 2**31, size=len(input_files))

    def _process_one_file(args):
        input_file, seed = args
        data = np.load(input_file, allow_pickle=True)
        feats = sparse.load_npz(input_file.replace('.npz', '_features.npz'))

        is_cont = data['is_cont']
        gen_pdg = data['gen_pdg']
        bp_is_best = data['bp_is_best']
        best_sigprob = data['best_sigprob']
        best_bp_sigprob_iid = data['best_bp_sigprob_iid']
        best_b0_sigprob_iid = data['best_b0_sigprob_iid']
        bp_tag_is_gen = data['bp_tag_is_gen']
        b0_tag_is_gen = data['b0_tag_is_gen']

        best_bp_iid = data['best_bp_iid']
        best_bp_dp = data['best_bp_dp']
        best_b0_iid = data['best_b0_iid']
        best_b0_dp = data['best_b0_dp']

        event_calib_bp, event_calib_b0 = compute_event_calib(
            best_bp_sigprob_iid, best_b0_sigprob_iid,
            bp_tag_is_gen, b0_tag_is_gen, is_cont,
        )
        event_calib = np.where(bp_is_best == 1, event_calib_bp, event_calib_b0)

        # Keep events where the best candidate passes the sigProb threshold.
        # Applies to all events including continuum.
        presel = best_sigprob > sigprob_thresh

        sample_prob = event_calib / config.CALIB_WEIGHT_CAP * fraction
        sample_prob[is_cont == 1] *= cont_fraction
        n_clipped = int((sample_prob > 1.0).sum())
        sample_prob = np.minimum(sample_prob, 1.0)

        file_rng = np.random.default_rng(seed)
        sampled = (file_rng.random(len(best_sigprob)) < sample_prob) & presel

        return (feats[sampled],
                is_cont[sampled], gen_pdg[sampled], bp_is_best[sampled], best_sigprob[sampled],
                best_bp_sigprob_iid[sampled], best_b0_sigprob_iid[sampled],
                n_clipped,
                best_bp_iid[sampled], best_bp_dp[sampled],
                best_b0_iid[sampled], best_b0_dp[sampled])

    features_list = []
    is_cont_list, gen_pdg_list, bp_is_best_list, best_sigprob_list = [], [], [], []
    best_bp_sigprob_iid_list, best_b0_sigprob_iid_list = [], []
    best_bp_iid_list, best_bp_dp_list = [], []
    best_b0_iid_list, best_b0_dp_list = [], []
    total_clipped = 0
    total_events = 0

    n_workers = min(32, len(input_files))
    with ThreadPoolExecutor(max_workers=n_workers) as executor:
        future_list = [executor.submit(_process_one_file, (f, s))
                       for f, s in zip(input_files, seeds)]
        results = [f.result() for f in tqdm(future_list, desc="Loading files")]

    for result in results:
        (feats,
         is_cont_r, gen_pdg_r, bp_is_best_r, best_sigprob_r,
         best_bp_sigprob_iid_r, best_b0_sigprob_iid_r,
         n_clipped,
         bp_iid, bp_dp, b0_iid, b0_dp) = result
        features_list.append(feats)
        is_cont_list.append(is_cont_r)
        gen_pdg_list.append(gen_pdg_r)
        bp_is_best_list.append(bp_is_best_r)
        best_sigprob_list.append(best_sigprob_r)
        best_bp_sigprob_iid_list.append(best_bp_sigprob_iid_r)
        best_b0_sigprob_iid_list.append(best_b0_sigprob_iid_r)
        best_bp_iid_list.append(bp_iid)
        best_bp_dp_list.append(bp_dp)
        best_b0_iid_list.append(b0_iid)
        best_b0_dp_list.append(b0_dp)
        total_clipped += n_clipped
        total_events += len(is_cont_r)

    if total_clipped > 0:
        print(f"WARNING: calibration weight exceeded CALIB_WEIGHT_CAP ({config.CALIB_WEIGHT_CAP:.4f}) "
              f"for {total_clipped} events ({total_clipped / total_events * 100:.1f}% of sampled events). "
              f"These events were capped and their sampling weight reduced.")

    # Concatenate across all files
    features = sparse.vstack(features_list, format='csr')
    is_cont = np.concatenate(is_cont_list)
    gen_pdg = np.concatenate(gen_pdg_list)
    bp_is_best = np.concatenate(bp_is_best_list)
    best_sigprob = np.concatenate(best_sigprob_list)
    best_bp_sigprob_iid = np.concatenate(best_bp_sigprob_iid_list)
    best_b0_sigprob_iid = np.concatenate(best_b0_sigprob_iid_list)

    print(f"\nTotal after concatenation: {features.shape[0]} events")

    # Compute has_inputs dynamically and verify against config.HAS_INPUTS
    print("\nVerifying HAS_INPUTS from feature sparsity...")

    n_total = features.shape[1]  # 1643

    # Stage 1: All-zero columns
    nonzero_cols = set(np.unique(features.nonzero()[1]))
    all_zero_cols = set(range(n_total)) - nonzero_cols

    # Stage 2: Add Mbc block (block i=10, indices 1360-1495)
    n_input_ids = config.N_INPUT_IDS  # 136
    mbc_start = n_input_ids * 10
    mbc_end = n_input_ids * 11
    mbc_cols = set(range(mbc_start, mbc_end))

    computed_has_inputs = sorted(set(range(n_total)) - (all_zero_cols | mbc_cols))
    n_computed_remove = n_total - len(computed_has_inputs)
    print(f"  All-zero columns:  {len(all_zero_cols)}")
    print(f"  Mbc block [{mbc_start}, {mbc_end}): {len(mbc_cols)} columns")
    print(f"  Total to remove:   {n_computed_remove}")

    # Verify against hardcoded list in config
    if computed_has_inputs != config.HAS_INPUTS:
        print("  WARNING: HAS_INPUTS mismatch!")
        print(f"    Computed from data: {len(computed_has_inputs)} kept ({n_computed_remove} removed)")
        print(f"    config.HAS_INPUTS: {len(config.HAS_INPUTS)} kept ({n_total - len(config.HAS_INPUTS)} removed)")
        print("    Data may not cover all input_ids.")
        out_path = "has_inputs_recomputed.txt"
        n_kept = len(computed_has_inputs)
        n_removed = n_total - n_kept
        with open(out_path, "w") as f:
            f.write("HAS_INPUTS = [\n")
            row = []
            for idx in computed_has_inputs:
                row.append(idx)
                if len(row) == 17:
                    f.write("    " + ", ".join(str(x) for x in row) + ",\n")
                    row = []
            if row:
                f.write("    " + ", ".join(str(x) for x in row) + ",\n")
            f.write(f"]  # {n_kept} indices kept ({n_total} - {n_removed} removed)\n")
        print(f"    Recomputed list written to {out_path}")
    else:
        print(f"  HAS_INPUTS verified OK ({len(config.HAS_INPUTS)} kept, "
              f"{n_total - len(config.HAS_INPUTS)} removed)")

    if max(config.HAS_INPUTS) >= n_total:
        print("  WARNING: config.HAS_INPUTS contains out-of-range indices for current feature layout.")
        print("           Using recomputed HAS_INPUTS for this run.")
        has_inputs = computed_has_inputs
    else:
        has_inputs = list(config.HAS_INPUTS)

    # Apply feature selection
    features = features[:, has_inputs]

    mc_truth_cand = (
        np.concatenate(best_bp_iid_list),
        np.concatenate(best_bp_dp_list),
        np.concatenate(best_b0_iid_list),
        np.concatenate(best_b0_dp_list),
    )
    event_scalars = (is_cont, gen_pdg, bp_is_best, best_sigprob,
                     best_bp_sigprob_iid, best_b0_sigprob_iid)
    return (features, event_scalars, has_inputs, mc_truth_cand)


class MultiClassNet(nn.Module):
    """Multi-class classification network with fully connected layers.

    Two architectures selected automatically by num_labels:
    - Deep (num_labels <= 10, e.g. category network with 3 classes):
      256 -> 128 (dropout 0.2) -> 64 (dropout 0.15) -> 32 (dropout 0.1) -> 16 -> num_labels
    - Shallow (num_labels > 10, e.g. main network with 139 classes):
      256 -> 128 (dropout 0.1) -> 128 (dropout 0.1) -> 64 (dropout 0.05) -> num_labels
    - ReLU activation, Xavier initialization (gain=0.5, bias=0.01)
    """

    def __init__(self, input_size, num_labels=3):
        super().__init__()

        self.activation = nn.ReLU()

        if num_labels <= 10:
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
        else:
            self.network = nn.Sequential(
                nn.Linear(input_size, 256),
                self.activation,
                nn.Linear(256, 128),
                nn.Dropout(0.1),
                self.activation,
                nn.Linear(128, 128),
                nn.Dropout(0.1),
                self.activation,
                nn.Linear(128, 64),
                nn.Dropout(0.05),
                self.activation,
                nn.Linear(64, num_labels)
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

            if probs.shape[1] > config.NUM_CAT_LABELS:  # main network
                # Signal proxy: max softmax prob over predicted sector's signal modes.
                # charged_cat is appended as the last feature in the batch.
                bp_threshold = config.N_BP_MODES * 2
                charged_cat_batch = data[:, -1] > 0.5
                bp_max = probs[:, :bp_threshold].max(dim=1).values
                b0_max = probs[:, bp_threshold:config.N_INPUT_IDS].max(dim=1).values
                signal_prob = torch.where(charged_cat_batch, bp_max, b0_max)

                cont_class = MAIN_BG_CONT

                cont_mask = target == cont_class
                if cont_mask.sum() > 1:
                    disco_loss = disco_loss + disco_lambda * distance_corr(
                        signal_prob[cont_mask], batch_mbc[cont_mask]
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

    if len(loader.dataset) == 0:
        return float('nan')
    return total_loss / len(loader.dataset)


def distance_corr(var_1, var_2, normedweight=None, power=1):
    """
    Computes distance correlation between var_1 and var_2.

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


def build_mode_labels(mc_truth_cand, charged_cat, is_cont, gen_pdg,
                      best_bp_sigprob_iid, best_b0_sigprob_iid,
                      delta_p_thresh=0.15):
    """
    Build mode-prediction labels (0 to N_INPUT_IDS+2) from MC truth.

    Classes:
    - 0 to N_INPUT_IDS-1: signal mode; class index equals the candidate's input_id
    - N_INPUT_IDS+0 (136): bad_tag
    - N_INPUT_IDS+1 (137): cross_deltaC1 (|mostcommonBTagPDG| != |PDG sector|)
    - N_INPUT_IDS+2 (138): continuum

    Signal label assignment per event:
    1. Use charged_cat to select the predicted B sector (B+ or B0).
    2. Among all candidates in that sector, find the one with smallest
       mostcommonBTagDeltaP that satisfies mostcommonBTagPDG == PDG and is not
       continuum. If found and DeltaP < delta_p_thresh: label = that input_id.
    3. Otherwise assign background class from compact event scalars.

    Parameters
    ----------
    mc_truth_cand : tuple (best_bp_iid, best_bp_dp, best_b0_iid, best_b0_dp)
        Per-event arrays from load_and_sample_data. best_bp_iid/b0_iid are int16
        with sentinel -1; best_bp_dp/b0_dp are float32 with sentinel inf.
    charged_cat : ndarray of bool
        True if category network predicts B+, False for B0
    is_cont : ndarray of int8
        1 if continuum event (from best overall candidate), else 0
    gen_pdg : ndarray of int16
        mostcommonBTagPDG of best overall candidate (0 for continuum)
    best_bp_sigprob_iid : ndarray of int16
        input_id of best-sigProb B+ candidate; -1 if absent
    best_b0_sigprob_iid : ndarray of int16
        input_id of best-sigProb B0 candidate; -1 if absent
    delta_p_thresh : float
        Threshold for mostcommonBTagDeltaP. Default: 0.15

    Returns
    -------
    labels : ndarray of int64
        Mode labels (0 to N_INPUT_IDS+2)
    train_selection : ndarray of bool
        True for events where the category-selected sector has a valid candidate
    """
    n_events = len(is_cont)

    # --- Per-sector best candidate lookup ---
    best_bp_iid, best_bp_dp, best_b0_iid, best_b0_dp = mc_truth_cand
    best_iid = np.where(charged_cat, best_bp_iid, best_b0_iid).astype(np.int64)
    best_dp = np.where(charged_cat, best_bp_dp, best_b0_dp)

    has_target = (best_iid >= 0) & (best_dp < delta_p_thresh)
    labels = np.full(n_events, MAIN_BG_BAD_TAG, dtype=np.int64)
    labels[has_target] = best_iid[has_target]

    # --- Background labels for events without an is_target candidate ---
    no_target = ~has_target
    if no_target.any():
        sector_abs_pdg = np.where(charged_cat, 521, 511)

        nt = no_target
        labels[nt & (is_cont == 1)] = MAIN_BG_CONT
        cross_dc1 = nt & (is_cont == 0) & (np.abs(gen_pdg.astype(np.int32)) != sector_abs_pdg)
        labels[cross_dc1] = MAIN_BG_CROSS_DC1
        # remaining no_target events keep bad_tag (set as default above)

    # train_selection: keep events where the predicted sector has a valid candidate
    train_selection = np.where(charged_cat,
                               best_bp_sigprob_iid >= 0,
                               best_b0_sigprob_iid >= 0)

    assert labels.min() >= 0 and labels.max() <= MAIN_BG_CONT, (
        f"Labels out of range [0, {MAIN_BG_CONT}]: "
        f"min={labels.min()}, max={labels.max()}"
    )
    return labels, train_selection


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


def build_category_labels(is_cont, gen_pdg):
    """
    Build category labels (B0=0, B+=1, continuum=2) from compact MC truth scalars.

    Parameters
    ----------
    is_cont : ndarray of int8
        1 if continuum event, 0 otherwise
    gen_pdg : ndarray of int16
        mostcommonBTagPDG of the best-sigProb candidate (0 for continuum)

    Returns
    -------
    labels : ndarray of int64
        Category labels (0=B0, 1=B+, 2=continuum)
    """
    return np.where(
        is_cont,
        2,
        np.where(np.abs(gen_pdg.astype(np.int32)) == 521, 1, 0)
    ).astype(np.int64)


def main():
    parser = argparse.ArgumentParser(description='Train ModeSelector networks')
    parser.add_argument('--input', required=True, nargs='+',
                        help='One or more modeSelector_training.npz paths (shell glob or space-separated list)')
    parser.add_argument('--network', choices=['category', 'main'], required=True,
                        help='Which network to train')
    parser.add_argument('--cat_model', help='Trained category model (required for main network)')
    parser.add_argument('--output', default='networks/', help='Output directory for trained models')
    parser.add_argument('--fraction', type=float, default=1.0, help='Base sampling fraction for all events')
    parser.add_argument('--cont_fraction', type=float, default=1.0,
                        help='Additional continuum downscale at training time (default 1.0; '
                             'continuum is already downsampled at production by produceTrainingInputs.py)')
    parser.add_argument('--batch_size', type=int, default=None,
                        help='Batch size (default: 8192 for category network, 32768 for main network)')
    parser.add_argument('--num_workers', type=int, default=4, help='Number of DataLoader worker processes')
    parser.add_argument('--epochs', type=int, default=40, help='Number of epochs')
    parser.add_argument('--lr', type=float, default=1e-3, help='Initial learning rate (cosine annealed to 1e-6)')
    parser.add_argument('--weight_decay', type=float, default=1e-4, help='Weight decay for AdamW')
    parser.add_argument('--val_split', type=float, default=0.1, help='Validation split')
    parser.add_argument('--seed', type=int, default=42, help='Random seed')
    parser.add_argument('--disco_lambda', type=float, default=0.0,
                        help='Distance correlation penalty coefficient (0=disabled)')
    parser.add_argument('--label_smoothing', type=float, default=0.1,
                        help='Label smoothing for CrossEntropyLoss (0=disabled, default 0.1)')
    parser.add_argument('--use_sparse', action='store_true',
                        help='Use sparse data loading (memory-efficient but slower)')

    args = parser.parse_args()

    if args.batch_size is None:
        args.batch_size = 8192 if args.network == 'category' else 32768
    print(f"Batch size: {args.batch_size}")

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
    print(f"Calibration weight cap (90th percentile): {config.CALIB_WEIGHT_CAP:.4f}")
    features, event_scalars, has_inputs, mc_truth_cand = load_and_sample_data(
        args.input,
        fraction=args.fraction,
        cont_fraction=args.cont_fraction,
        random_state=args.seed,
    )
    is_cont, gen_pdg, bp_is_best, best_sigprob, best_bp_sigprob_iid, best_b0_sigprob_iid = event_scalars

    print(f"\nFeature matrix shape: {features.shape}")
    print(f"  Sparse matrix memory: {features.data.nbytes / 1024**2:.1f} MB")
    print(f"  Selected features (has_inputs): {len(has_inputs)}")

    # Build labels
    print("\nBuilding labels...")
    if args.network == 'category':
        labels = build_category_labels(is_cont, gen_pdg)
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
        charged_cat_bool = cat_outputs[:, 1] > cat_outputs[:, 0]
        charged_cat = charged_cat_bool.astype(np.float32)

        # Concatenate category outputs + charged_cat flag to features
        print("\nConcatenating category outputs to features...")
        features_with_cat = np.hstack([features_dense, cat_outputs, charged_cat.reshape(-1, 1)])
        features_dense = features_with_cat
        input_size = features_dense.shape[1]
        print(f"  New feature shape: {features_dense.shape}")

        # Build mode-prediction labels using category network prediction
        if mc_truth_cand is None:
            raise ValueError(
                "mc_truth_cand not found in training data. "
                "Re-collect training data with produceTrainingInputs.py."
            )
        labels, train_selection = build_mode_labels(
            mc_truth_cand, charged_cat_bool, is_cont, gen_pdg,
            best_bp_sigprob_iid, best_b0_sigprob_iid,
        )
        num_labels = MAIN_NUM_LABELS
        n_signal = int((labels < config.N_INPUT_IDS).sum())
        bg_bad = int((labels == MAIN_BG_BAD_TAG).sum())
        bg_dc1 = int((labels == MAIN_BG_CROSS_DC1).sum())
        bg_cont = int((labels == MAIN_BG_CONT).sum())
        n_tot = len(labels)
        print("  Main network label distribution (before train_selection):")
        print(f"    signal modes (0-{config.N_INPUT_IDS - 1}): {n_signal} ({n_signal / n_tot * 100:.1f}%)")
        print(f"    bad_tag ({MAIN_BG_BAD_TAG}):        {bg_bad} ({bg_bad / n_tot * 100:.1f}%)")
        print(f"    cross_deltaC1 ({MAIN_BG_CROSS_DC1}):  {bg_dc1} ({bg_dc1 / n_tot * 100:.1f}%)")
        print(f"    continuum ({MAIN_BG_CONT}):      {bg_cont} ({bg_cont / n_tot * 100:.1f}%)")

        mbc_values = None

        # Apply train_selection: drop events where the category-selected candidate type has no candidate
        n_before = len(features_dense)
        features_dense = features_dense[train_selection]
        labels = labels[train_selection]
        if mbc_values is not None:
            mbc_values = mbc_values[train_selection]
        n_dropped = n_before - int(train_selection.sum())
        print(f"  train_selection: dropped {n_dropped} events ({n_dropped / n_before * 100:.1f}%)")

    # Mbc values for DisCo loss (category network; main network handled above)
    if args.network == 'category':
        mbc_values = None

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
        drop_last = len(train_dataset) > args.batch_size
        train_loader = DataLoader(train_dataset, batch_size=args.batch_size, shuffle=True,
                                  collate_fn=sparse_collate_fn, num_workers=args.num_workers,
                                  drop_last=drop_last)
        val_loader = DataLoader(val_dataset, batch_size=args.batch_size, shuffle=False,
                                collate_fn=sparse_collate_fn, num_workers=args.num_workers)
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

        drop_last = len(train_dataset) > args.batch_size
        train_loader = DataLoader(train_dataset, batch_size=args.batch_size, shuffle=True,
                                  num_workers=args.num_workers, drop_last=drop_last)
        val_loader = DataLoader(val_dataset, batch_size=args.batch_size, shuffle=False,
                                num_workers=args.num_workers)

    # Create model
    print("\n" + "=" * 60)
    print("Creating model")
    print("=" * 60)
    model = MultiClassNet(input_size=input_size, num_labels=num_labels)
    model = model.to(device)

    print(f"  Input size: {input_size}")
    print(f"  Num labels: {num_labels}")
    print(f"  Parameters: {sum(p.numel() for p in model.parameters()):,}")

    # Loss and optimizer
    criterion = nn.CrossEntropyLoss(label_smoothing=args.label_smoothing)
    optimizer = optim.AdamW(model.parameters(), lr=args.lr, weight_decay=args.weight_decay)
    scheduler = optim.lr_scheduler.CosineAnnealingLR(optimizer, T_max=args.epochs, eta_min=1e-5)

    # Training loop
    print("\n" + "=" * 60)
    print("Training")
    print("=" * 60)
    if args.disco_lambda > 0:
        print(f"Using distance correlation with lambda={args.disco_lambda}")

    best_val_loss = float('inf')
    best_epoch = 0
    training_start = time.time()
    history = {'train_loss': [], 'val_loss': [], 'disco_loss': [], 'lr': []}
    patience_counter = 0
    early_stopping_patience = 10

    for epoch in range(args.epochs):
        epoch_start = time.time()
        current_lr = optimizer.param_groups[0]['lr']

        # Train
        train_loss, disco_loss = train_epoch(
            model, train_loader, criterion, optimizer, device,
            disco_lambda=args.disco_lambda
        )

        # Validate
        val_loss = evaluate(model, val_loader, criterion, device)

        epoch_time = time.time() - epoch_start

        # Scheduler step
        scheduler.step()
        new_lr = optimizer.param_groups[0]['lr']

        history['train_loss'].append(train_loss)
        history['val_loss'].append(val_loss)
        history['disco_loss'].append(disco_loss)
        history['lr'].append(current_lr)

        # Print progress
        if args.disco_lambda > 0:
            print(f"Epoch {epoch+1:3d}/{args.epochs}: "
                  f"train_loss={train_loss:.6f}, disco_loss={disco_loss:.6f}, "
                  f"val_loss={val_loss:.6f}, lr={current_lr:.2e}, time={epoch_time:.1f}s")
        else:
            print(f"Epoch {epoch+1:3d}/{args.epochs}: "
                  f"train_loss={train_loss:.6f}, val_loss={val_loss:.6f}, "
                  f"lr={current_lr:.2e}, time={epoch_time:.1f}s")
        if new_lr < current_lr:
            print(f"  -> LR reduced: {current_lr:.2e} -> {new_lr:.2e}")

        # Save best model (treat nan val_loss as always saving, for no-val-set runs)
        if not (val_loss >= best_val_loss):
            best_val_loss = val_loss
            best_epoch = epoch
            patience_counter = 0
            model_path = os.path.join(args.output, f'net_{args.network}.pt')
            torch.save({
                'epoch': epoch,
                'model_state_dict': model.state_dict(),
                'optimizer_state_dict': optimizer.state_dict(),
                'val_loss': val_loss,
                'train_loss': train_loss,
                'history': history,
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
        else:
            patience_counter += 1
            if patience_counter >= early_stopping_patience:
                print(f"  -> Early stopping at epoch {epoch + 1}")
                break

    print("\n" + "=" * 60)
    print("Training complete")
    print("=" * 60)
    training_time = time.time() - training_start
    minutes = int(training_time // 60)
    seconds = int(training_time % 60)
    print(f"Training time: {minutes}m {seconds}s")
    print(f"Best epoch: {best_epoch+1}")
    print(f"Best val loss: {best_val_loss:.6f}")
    print(f"Model saved to: {os.path.join(args.output, f'net_{args.network}.pt')}")

    # Update checkpoint with full history (best-epoch save only captured history up to that point)
    model_path = os.path.join(args.output, f'net_{args.network}.pt')
    final_ckpt = torch.load(model_path)
    final_ckpt['history'] = history
    torch.save(final_ckpt, model_path)

    # Evaluation on validation set
    if len(val_loader.dataset) == 0:
        print("\nNo validation set, skipping evaluation.")
        return

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
    print(f"\n  {'Class':<28} {'N':>8} {'Accuracy':>10} {'Mean P':>10} {'Median P':>10}")
    print(f"  {'-'*66}")

    if args.network == 'category':
        eval_classes = [(i, name) for i, name in enumerate(['B0', 'B+', 'continuum'])]
        for cls_idx, cls_name in eval_classes:
            cls_mask = val_labels == cls_idx
            n_cls = cls_mask.sum()
            if n_cls > 0:
                cls_probs = val_probs[cls_mask, cls_idx]
                pred_cls = np.argmax(val_probs[cls_mask], axis=1)
                accuracy = (pred_cls == cls_idx).mean()
                print(f"  {cls_name:<28} {n_cls:>8,} {accuracy:>10.4f} "
                      f"{cls_probs.mean():>10.4f} {np.median(cls_probs):>10.4f}")
    else:
        # Main network: show signal modes aggregate + 3 background classes
        signal_mask = val_labels < config.N_INPUT_IDS
        n_sig = signal_mask.sum()
        if n_sig > 0:
            sig_true = val_labels[signal_mask]
            sig_probs = val_probs[signal_mask][np.arange(n_sig), sig_true]
            sig_acc = (np.argmax(val_probs[signal_mask], axis=1) == sig_true).mean()
            cls_name = f'signal modes (0-{config.N_INPUT_IDS - 1})'
            print(f"  {cls_name:<28} {n_sig:>8,} {sig_acc:>10.4f} "
                  f"{sig_probs.mean():>10.4f} {np.median(sig_probs):>10.4f}")
        bg_classes = [
            (MAIN_BG_BAD_TAG, 'bad_tag'),
            (MAIN_BG_CROSS_DC1, 'cross_deltaC1'),
            (MAIN_BG_CONT, 'continuum'),
        ]
        for cls_idx, cls_name in bg_classes:
            cls_mask = val_labels == cls_idx
            n_cls = cls_mask.sum()
            if n_cls > 0:
                cls_probs = val_probs[cls_mask, cls_idx]
                pred_cls = np.argmax(val_probs[cls_mask], axis=1)
                accuracy = (pred_cls == cls_idx).mean()
                print(f"  {cls_name:<28} {n_cls:>8,} {accuracy:>10.4f} "
                      f"{cls_probs.mean():>10.4f} {np.median(cls_probs):>10.4f}")

    # Overall accuracy
    overall_acc = (np.argmax(val_probs, axis=1) == val_labels).mean()
    print(f"\n  Overall accuracy: {overall_acc:.4f}")


if __name__ == '__main__':
    main()
