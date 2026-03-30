#!/usr/bin/env python3
"""
Training script for ModeSelector neural networks.

This script trains the two-stage ModeSelector:
1. Category network (B0 vs B+ vs continuum classification)
2. Main network (signal vs background classification using category output)

Usage:
    # Train category network (single file)
    python3 train.py --input modeSelector_training.npz --network category --output networks/

    # Train category network (multiple files)
    python3 train.py --input modeSelector_training_*.npz --network category --output networks/

    # Train main network (requires trained category network)
    python3 train.py --input modeSelector_training.npz --network main --cat_model networks/net_category.pt --output networks/
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

#: Main network label index for bad-tag background
MAIN_BG_BAD_TAG = config.N_INPUT_IDS
#: Main network label index for cross-deltaC1 background
MAIN_BG_CROSS_DC1 = config.N_INPUT_IDS + 1
#: Main network label index for continuum background
MAIN_BG_CONT = config.N_INPUT_IDS + 2
#: Total number of main network output labels
MAIN_NUM_LABELS = config.N_INPUT_IDS + 3


def load_and_sample_data(input_files, fraction=1.0, cont_fraction=1.0,
                         sigprob_thresh=config.DEFAULT_FEI_SIGPROB_THRESHOLD,
                         random_state=None):
    """
    Load training data and apply optional global downsampling.

    Parameters:
        input_files (str or list of str): Path(s) to modeSelector_training.npz file(s).
        fraction (float): Uniform BB sampling fraction applied after loading (default 1.0).
        cont_fraction (float): Additional continuum downscale relative to fraction (default 1.0).
        sigprob_thresh (float): Minimum signal probability threshold. Default: 0.001.
        random_state (int): Random seed.

    Returns:
        features (sparse matrix): Sampled and filtered feature matrix (only has_inputs columns).
        event_scalars (tuple): (is_cont, gen_pdg, bp_is_best, best_sigprob,
            best_bp_sigprob_iid, best_b0_sigprob_iid) -- per-event compact MC truth scalars;
            int8/int16/float32 arrays of shape (n_events,).
        has_inputs (list of int): Selected feature indices (non-zero features, excluding Mbc).
        mc_truth_cand (tuple): (best_bp_iid, best_bp_dp, best_b0_iid, best_b0_dp) --
            per-event arrays of shape (n_events,). best_bp_iid/b0_iid are int16 with
            sentinel -1 when no qualifying candidate exists; best_bp_dp/b0_dp are float32
            with sentinel inf. Pre-filtered: truth-compatible tag PDG and is_cont != 1.
        sig_truth (tuple): (sig_input_ids_values, sig_input_ids_offsets,
            sig_btag_index_values, sig_delta_p_values, sig_sigprob_values) --
            packed ragged arrays of per-event isSignal==1 candidates on deduplicated
            input_ids. Event i slice is values[offsets[i]:offsets[i+1]] and aligned
            across all *_values arrays.
        calib_inputs (tuple): (bp_tag_is_gen, b0_tag_is_gen, bp_gen_dm_id, b0_gen_dm_id,
            bp_gen_calib_w, b0_gen_calib_w, stored_fei_calib_w) -- per-event arrays for FEI
            calibration weight computation. bp/b0_gen_dm_id are int16 with sentinel -1
            (missing) or 999 (rest calibration). stored_fei_calib_w is the pre-computed
            event-level weight from the npz, used to verify recomputed weights in
            compute_event_weights. bp/b0_gen_calib_w are float32 stored calibration weights
            from generatedDecayWeights.
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

    parent_dirs = sorted({os.path.dirname(os.path.abspath(f)) for f in input_files})
    print("Resolved input parent directories:")
    for d in parent_dirs:
        print(f"  {d}")

    # Preflight: verify all archives have required keys before parallel loading starts.
    required_keys = (
        'is_cont', 'gen_pdg', 'bp_is_best', 'best_sigprob',
        'best_bp_sigprob_iid', 'best_b0_sigprob_iid',
        'bp_tag_is_gen', 'b0_tag_is_gen',
        'bp_gen_decay_mode_id', 'b0_gen_decay_mode_id',
        'bp_gen_fei_calib_weight', 'b0_gen_fei_calib_weight',
        'fei_calib_weight',
        'best_bp_iid', 'best_bp_dp', 'best_b0_iid', 'best_b0_dp',
        'sig_input_ids_values', 'sig_input_ids_offsets',
        'sig_btag_index_values', 'sig_delta_p_values', 'sig_sigprob_values'
    )
    preflight_errors = []
    for input_file in input_files:
        features_file = input_file.replace('.npz', '_features.npz')
        if not os.path.exists(features_file):
            preflight_errors.append(
                f"{input_file}: missing companion sparse features file '{features_file}'"
            )
            continue
        try:
            data = np.load(input_file, allow_pickle=True)
            keys = set(data.files)
        except Exception as exc:
            preflight_errors.append(f"{input_file}: failed to read npz ({exc})")
            continue
        missing = [k for k in required_keys if k not in keys]
        if missing:
            preflight_errors.append(
                f"{input_file}: missing keys {missing}; available keys: {sorted(keys)}"
            )

    if preflight_errors:
        msg = ["Input preflight failed. Fix input files before training."]
        msg.extend(preflight_errors[:20])
        if len(preflight_errors) > 20:
            msg.append(f"... and {len(preflight_errors) - 20} more files")
        raise ValueError("\n".join(msg))

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
        bp_gen_decay_mode_id = data['bp_gen_decay_mode_id']
        b0_gen_decay_mode_id = data['b0_gen_decay_mode_id']
        bp_gen_fei_calib_weight = data['bp_gen_fei_calib_weight']
        b0_gen_fei_calib_weight = data['b0_gen_fei_calib_weight']
        fei_calib_weight = data['fei_calib_weight']

        best_bp_iid = data['best_bp_iid']
        best_bp_dp = data['best_bp_dp']
        best_b0_iid = data['best_b0_iid']
        best_b0_dp = data['best_b0_dp']
        sig_input_ids_values = data['sig_input_ids_values']
        sig_input_ids_offsets = data['sig_input_ids_offsets']
        sig_btag_index_values = data['sig_btag_index_values']
        sig_delta_p_values = data['sig_delta_p_values']
        sig_sigprob_values = data['sig_sigprob_values']

        # Keep events where the best candidate passes the sigProb threshold.
        # Applies to all events including continuum.
        presel = best_sigprob > sigprob_thresh

        sample_prob = np.full(len(best_sigprob), fraction, dtype=np.float32)
        sample_prob[is_cont == 1] *= cont_fraction
        sample_prob = np.minimum(sample_prob, 1.0)

        file_rng = np.random.default_rng(seed)
        sampled = (file_rng.random(len(best_sigprob)) < sample_prob) & presel

        keep_events = np.flatnonzero(sampled).astype(np.int32)

        def _subset_packed(values, offsets, keep):
            out_offsets = np.empty(len(keep) + 1, dtype=np.int32)
            out_offsets[0] = 0
            chunks = []
            total_len = 0
            for i, evt_idx in enumerate(keep):
                start = int(offsets[evt_idx])
                end = int(offsets[evt_idx + 1])
                chunk = values[start:end]
                chunks.append(chunk)
                total_len += (end - start)
                out_offsets[i + 1] = total_len
            if total_len > 0:
                out_values = np.concatenate(chunks).astype(values.dtype, copy=False)
            else:
                out_values = np.empty(0, dtype=values.dtype)
            return out_values, out_offsets

        sig_iid_v_s, sig_off_s = _subset_packed(sig_input_ids_values, sig_input_ids_offsets, keep_events)
        sig_btag_v_s, _ = _subset_packed(sig_btag_index_values, sig_input_ids_offsets, keep_events)
        sig_dp_v_s, _ = _subset_packed(sig_delta_p_values, sig_input_ids_offsets, keep_events)
        sig_sigprob_v_s, _ = _subset_packed(sig_sigprob_values, sig_input_ids_offsets, keep_events)

        return (feats[sampled],
                is_cont[sampled], gen_pdg[sampled], bp_is_best[sampled], best_sigprob[sampled],
                best_bp_sigprob_iid[sampled], best_b0_sigprob_iid[sampled],
                best_bp_iid[sampled], best_bp_dp[sampled],
                best_b0_iid[sampled], best_b0_dp[sampled],
                sig_iid_v_s, sig_off_s, sig_btag_v_s, sig_dp_v_s, sig_sigprob_v_s,
                bp_tag_is_gen[sampled], b0_tag_is_gen[sampled],
                bp_gen_decay_mode_id[sampled], b0_gen_decay_mode_id[sampled],
                bp_gen_fei_calib_weight[sampled], b0_gen_fei_calib_weight[sampled],
                fei_calib_weight[sampled])

    features_list = []
    is_cont_list, gen_pdg_list, bp_is_best_list, best_sigprob_list = [], [], [], []
    best_bp_sigprob_iid_list, best_b0_sigprob_iid_list = [], []
    best_bp_iid_list, best_bp_dp_list = [], []
    best_b0_iid_list, best_b0_dp_list = [], []
    sig_input_ids_values_list, sig_input_ids_offsets_list = [], []
    sig_btag_index_values_list, sig_delta_p_values_list, sig_sigprob_values_list = [], [], []
    bp_tag_is_gen_list, b0_tag_is_gen_list = [], []
    bp_gen_dm_id_list, b0_gen_dm_id_list = [], []
    bp_gen_calib_w_list, b0_gen_calib_w_list = [], []
    stored_fei_calib_w_list = []

    n_workers = min(32, len(input_files))
    with ThreadPoolExecutor(max_workers=n_workers) as executor:
        future_list = [executor.submit(_process_one_file, (f, s))
                       for f, s in zip(input_files, seeds)]
        results = [f.result() for f in tqdm(future_list, desc="Loading files")]

    for result in results:
        (feats,
         is_cont_r, gen_pdg_r, bp_is_best_r, best_sigprob_r,
         best_bp_sigprob_iid_r, best_b0_sigprob_iid_r,
         bp_iid, bp_dp, b0_iid, b0_dp,
         sig_iid_v, sig_off, sig_btag_v, sig_dp_v, sig_sigprob_v,
         bp_tg, b0_tg, bp_gd, b0_gd, bp_gcw, b0_gcw, stored_fcw) = result
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
        sig_input_ids_values_list.append(sig_iid_v)
        sig_input_ids_offsets_list.append(sig_off)
        sig_btag_index_values_list.append(sig_btag_v)
        sig_delta_p_values_list.append(sig_dp_v)
        sig_sigprob_values_list.append(sig_sigprob_v)
        bp_tag_is_gen_list.append(bp_tg)
        b0_tag_is_gen_list.append(b0_tg)
        bp_gen_dm_id_list.append(bp_gd)
        b0_gen_dm_id_list.append(b0_gd)
        bp_gen_calib_w_list.append(bp_gcw)
        b0_gen_calib_w_list.append(b0_gcw)
        stored_fei_calib_w_list.append(stored_fcw)

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

    n_total = features.shape[1]

    # Stage 1: All-zero columns
    nonzero_cols = set(np.unique(features.nonzero()[1]))
    all_zero_cols = set(range(n_total)) - nonzero_cols

    # Stage 2: Exclude blocks that should not be active network inputs.
    # Mbc is excluded to avoid output correlation. With skipTreeFit=True,
    # Dst0_chiProb and Dstp_chiProb do not add independent information beyond
    # Bdaughter_chiProb, so they are excluded as well.
    n_input_ids = config.N_INPUT_IDS  # 136
    excluded_block_ranges = {
        'Dst0_chiProb': (n_input_ids * 7, n_input_ids * 8),
        'Dstp_chiProb': (n_input_ids * 8, n_input_ids * 9),
        'Mbc': (n_input_ids * 10, n_input_ids * 11),
    }
    excluded_cols = set()
    for start, end in excluded_block_ranges.values():
        excluded_cols.update(range(start, end))

    computed_has_inputs = sorted(set(range(n_total)) - (all_zero_cols | excluded_cols))
    n_computed_remove = n_total - len(computed_has_inputs)
    print(f"  All-zero columns:  {len(all_zero_cols)}")
    for block_name, (start, end) in excluded_block_ranges.items():
        print(f"  {block_name} block [{start}, {end}): {end - start} columns")
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
        print("    Using recomputed HAS_INPUTS for this run.")
        has_inputs = computed_has_inputs
    else:
        print(f"  HAS_INPUTS verified OK ({len(config.HAS_INPUTS)} kept, "
              f"{n_total - len(config.HAS_INPUTS)} removed)")
        has_inputs = list(config.HAS_INPUTS)

    # Apply feature selection
    features = features[:, has_inputs]

    mc_truth_cand = (
        np.concatenate(best_bp_iid_list),
        np.concatenate(best_bp_dp_list),
        np.concatenate(best_b0_iid_list),
        np.concatenate(best_b0_dp_list),
    )
    if sig_input_ids_offsets_list:
        global_offsets = [0]
        shift = 0
        for file_offsets in sig_input_ids_offsets_list:
            global_offsets.extend((file_offsets[1:] + shift).tolist())
            shift = global_offsets[-1]
        sig_input_ids_offsets = np.asarray(global_offsets, dtype=np.int32)
    else:
        sig_input_ids_offsets = np.zeros(features.shape[0] + 1, dtype=np.int32)

    if sig_input_ids_values_list:
        sig_input_ids_values = np.concatenate(sig_input_ids_values_list).astype(np.int16, copy=False)
        sig_btag_index_values = np.concatenate(sig_btag_index_values_list).astype(np.int16, copy=False)
        sig_delta_p_values = np.concatenate(sig_delta_p_values_list).astype(np.float32, copy=False)
        sig_sigprob_values = np.concatenate(sig_sigprob_values_list).astype(np.float32, copy=False)
    else:
        sig_input_ids_values = np.empty(0, dtype=np.int16)
        sig_btag_index_values = np.empty(0, dtype=np.int16)
        sig_delta_p_values = np.empty(0, dtype=np.float32)
        sig_sigprob_values = np.empty(0, dtype=np.float32)

    if len(sig_input_ids_offsets) != features.shape[0] + 1:
        raise ValueError(
            f"sig_input_ids_offsets length mismatch: got {len(sig_input_ids_offsets)}, "
            f"expected {features.shape[0] + 1}"
        )
    expected_n = int(sig_input_ids_offsets[-1])
    for name, arr in (
        ('sig_input_ids_values', sig_input_ids_values),
        ('sig_btag_index_values', sig_btag_index_values),
        ('sig_delta_p_values', sig_delta_p_values),
        ('sig_sigprob_values', sig_sigprob_values),
    ):
        if len(arr) != expected_n:
            raise ValueError(
                f"{name} length mismatch: got {len(arr)}, expected {expected_n} from offsets"
            )

    sig_truth = (
        sig_input_ids_values,
        sig_input_ids_offsets,
        sig_btag_index_values,
        sig_delta_p_values,
        sig_sigprob_values,
    )
    event_scalars = (is_cont, gen_pdg, bp_is_best, best_sigprob,
                     best_bp_sigprob_iid, best_b0_sigprob_iid)
    calib_inputs = (
        np.concatenate(bp_tag_is_gen_list),
        np.concatenate(b0_tag_is_gen_list),
        np.concatenate(bp_gen_dm_id_list),
        np.concatenate(b0_gen_dm_id_list),
        np.concatenate(bp_gen_calib_w_list),
        np.concatenate(b0_gen_calib_w_list),
        np.concatenate(stored_fei_calib_w_list),
    )
    return (features, event_scalars, has_inputs, mc_truth_cand, sig_truth, calib_inputs)


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
        """Build the network graph for the given input size and number of output classes."""
        super().__init__()

        #: Activation function applied between all linear layers
        self.activation = nn.ReLU()

        if num_labels <= 10:
            #: Fully connected network layers
            self.network = nn.Sequential(
                nn.Linear(input_size, 128),
                self.activation,
                nn.Linear(128, 128),
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
        """Run a forward pass through the network."""
        return self.network(x)


def train_epoch(model, train_loader, criterion, optimizer, device, disco_lambda=0.0):
    """
    Train for one epoch with optional distance correlation penalty.

    Parameters:
        model (nn.Module): Model to train.
        train_loader (DataLoader): Training data loader. Batch tuple formats supported:
            2-tuple (features, labels): no weights, no DisCo;
            3-tuple (features, labels, weights): weighted loss, no DisCo;
            4-tuple (features, labels, mbc, weights): weighted loss + DisCo.
        criterion (nn.Module): Loss function with reduction='none' (per-sample losses required).
        optimizer (torch.optim.Optimizer): Optimizer.
        device (torch.device): Device to use.
        disco_lambda (float): Coefficient for distance correlation loss (0 = disabled).

    Returns:
        tuple: (train_loss, disco_loss) -- average (weighted) classification loss and
            average distance correlation loss (0.0 if disabled).
    """
    model.train()
    total_loss = 0
    total_disco = 0
    n_batches = 0

    for batch in train_loader:
        # Unpack batch based on tuple length
        if len(batch) == 4:
            data, target, batch_mbc, sample_weights = batch
            batch_mbc = batch_mbc.to(device)
            sample_weights = sample_weights.to(device)
        elif len(batch) == 3:
            data, target, sample_weights = batch
            batch_mbc = None
            sample_weights = sample_weights.to(device)
        else:
            data, target = batch
            batch_mbc = None
            sample_weights = None

        data, target = data.to(device), target.to(device)

        optimizer.zero_grad()
        output = model(data)

        # Classification loss (per-sample when criterion has reduction='none')
        loss_per_sample = criterion(output, target)
        if sample_weights is not None:
            cls_loss = (loss_per_sample * sample_weights).mean()
        else:
            cls_loss = loss_per_sample.mean()

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
    """Evaluate model, applying per-sample weights when present.

    Expects criterion with reduction='none'. Weights are read from the last
    batch element when the batch has more than 2 elements.
    """
    model.eval()
    total_loss = 0

    with torch.no_grad():
        for batch in loader:
            data = batch[0].to(device)
            target = batch[1].to(device)
            output = model(data)
            loss_per_sample = criterion(output, target)
            if len(batch) > 2:
                sample_weights = batch[-1].to(device)
                loss = (loss_per_sample * sample_weights).mean()
            else:
                loss = loss_per_sample.mean()
            total_loss += loss.item() * len(data)

    if len(loader.dataset) == 0:
        return float('nan')
    return total_loss / len(loader.dataset)


def distance_corr(var_1, var_2, normedweight=None, power=1):
    """
    Computes distance correlation between var_1 and var_2.

    The distance correlation is a measure of dependence between two random variables.
    It is zero if and only if the variables are independent.

    Parameters:
        var_1 (torch.Tensor): First variable, shape (n,) (e.g., Mbc).
        var_2 (torch.Tensor): Second variable, shape (n,) (e.g., classifier output).
        normedweight (torch.Tensor, optional): Per-example weight, shape (n,), should sum to n.
            If None, uses uniform weights.
        power (int): Exponent for distance correlation (default 1).

    Returns:
        torch.Tensor: Distance correlation coefficient (scalar).
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


def build_mode_labels(mc_truth_cand, sig_truth, charged_cat, is_cont, gen_pdg,
                      delta_p_thresh=0.15):
    """
    Build mode-prediction labels (0 to N_INPUT_IDS+2) from MC truth.

    Classes:
    - 0 to N_INPUT_IDS-1: signal mode; class index equals the candidate's input_id
    - N_INPUT_IDS+0 (136): bad_tag
    - N_INPUT_IDS+1 (137): cross_deltaC1 (|mostcommonBTagPDG| != |PDG sector|)
    - N_INPUT_IDS+2 (138): continuum

    Signal label assignment per event:
    1. Use charged_cat to select predicted B sector (B+ or B0).
    2. If exactly one candidate in that sector has isSignal==1: choose it.
    3. If multiple isSignal==1 candidates:
       - If all have same mostcommonBTagIndex: choose largest input_id.
       - If they have different mostcommonBTagIndex: choose smallest deltaP.
         Tie-break by larger sigProb, then larger input_id.
    4. If no isSignal==1 candidate in predicted sector, fall back to current
       deltaP-based logic from compact truth scalars.

    Parameters:
        mc_truth_cand (tuple): (best_bp_iid, best_bp_dp, best_b0_iid, best_b0_dp) --
            per-event arrays from load_and_sample_data. best_bp_iid/b0_iid are int16
            with sentinel -1; best_bp_dp/b0_dp are float32 with sentinel inf.
        sig_truth (tuple): (sig_input_ids_values, sig_input_ids_offsets,
            sig_btag_index_values, sig_delta_p_values, sig_sigprob_values) --
            packed ragged isSignal==1 candidate metadata.
        charged_cat (ndarray of bool): True if category network predicts B+, False for B0.
        is_cont (ndarray of int8): 1 if continuum event (from best overall candidate), else 0.
        gen_pdg (ndarray of int16): mostcommonBTagPDG of best overall candidate (0 for continuum).
        delta_p_thresh (float): Threshold for mostcommonBTagDeltaP. Default: 0.15.

    Returns:
        tuple: (labels, train_selection, stats) where labels is ndarray of int64 with mode
            labels (0 to N_INPUT_IDS+2), train_selection is ndarray of bool for events kept
            for main-network training, and stats is a dict of label assignment counters.
    """
    n_events = len(is_cont)
    bp_threshold = config.N_BP_MODES * 2

    (sig_input_ids_values, sig_input_ids_offsets,
     sig_btag_index_values, sig_delta_p_values, sig_sigprob_values) = sig_truth

    if len(sig_input_ids_offsets) != n_events + 1:
        raise ValueError(
            f"sig_input_ids_offsets length mismatch: got {len(sig_input_ids_offsets)}, expected {n_events + 1}"
        )

    # --- Per-sector best candidate lookup ---
    best_bp_iid, best_bp_dp, best_b0_iid, best_b0_dp = mc_truth_cand
    best_iid = np.where(charged_cat, best_bp_iid, best_b0_iid).astype(np.int64)
    best_dp = np.where(charged_cat, best_bp_dp, best_b0_dp)

    labels = np.full(n_events, MAIN_BG_BAD_TAG, dtype=np.int64)
    assigned = np.zeros(n_events, dtype=bool)

    stats = {
        'single_signal': 0,
        'multi_same_btag': 0,
        'multi_diff_btag': 0,
        'fallback': 0,
    }

    for evt in range(n_events):
        start = int(sig_input_ids_offsets[evt])
        end = int(sig_input_ids_offsets[evt + 1])
        if start >= end:
            continue

        evt_iids = sig_input_ids_values[start:end]
        evt_btag_idx = sig_btag_index_values[start:end]
        evt_dp = sig_delta_p_values[start:end]
        evt_sigprob = sig_sigprob_values[start:end]

        if charged_cat[evt]:
            sector_mask = evt_iids < bp_threshold
        else:
            sector_mask = evt_iids >= bp_threshold

        if not np.any(sector_mask):
            continue

        sector_iids = evt_iids[sector_mask]
        sector_btag_idx = evt_btag_idx[sector_mask]
        sector_dp = evt_dp[sector_mask]
        sector_sigprob = evt_sigprob[sector_mask]

        if len(sector_iids) == 1:
            labels[evt] = int(sector_iids[0])
            assigned[evt] = True
            stats['single_signal'] += 1
            continue

        if len(np.unique(sector_btag_idx)) == 1:
            labels[evt] = int(np.max(sector_iids))
            assigned[evt] = True
            stats['multi_same_btag'] += 1
            continue

        best_tuple = None
        best_label = None
        for iid_val, dp_val, sigprob_val in zip(sector_iids, sector_dp, sector_sigprob):
            key = (float(dp_val), -float(sigprob_val), -int(iid_val))
            if (best_tuple is None) or (key < best_tuple):
                best_tuple = key
                best_label = int(iid_val)
        labels[evt] = best_label
        assigned[evt] = True
        stats['multi_diff_btag'] += 1

    # --- Fallback labels for events without an isSignal==1 target ---
    fallback_mask = ~assigned
    stats['fallback'] = int(fallback_mask.sum())
    has_target = fallback_mask & (best_iid >= 0) & (best_dp < delta_p_thresh)
    labels[has_target] = best_iid[has_target]

    no_target = fallback_mask & (~has_target)
    if no_target.any():
        sector_abs_pdg = np.where(charged_cat, 521, 511)

        nt = no_target
        labels[nt & (is_cont == 1)] = MAIN_BG_CONT
        cross_dc1 = nt & (is_cont == 0) & (np.abs(gen_pdg.astype(np.int32)) != sector_abs_pdg)
        labels[cross_dc1] = MAIN_BG_CROSS_DC1
        # remaining no_target events keep bad_tag (set as default above)

    # Keep all preselected events. Predicted-sector-empty events are retained and
    # trained against the fallback background labels above.
    train_selection = np.ones(n_events, dtype=bool)

    assert labels.min() >= 0 and labels.max() <= MAIN_BG_CONT, (
        f"Labels out of range [0, {MAIN_BG_CONT}]: "
        f"min={labels.min()}, max={labels.max()}"
    )
    return labels, train_selection, stats


class SparseDataset(torch.utils.data.Dataset):
    """Dataset for sparse feature matrices.

    Converts each row to dense on-the-fly during batching.
    Slower than dense loading but significantly more memory-efficient.
    """

    def __init__(self, features_sparse, labels, mbc_values=None, extra_features=None,
                 weights=None):
        """
        Parameters:
            features_sparse (scipy.sparse matrix): Sparse feature matrix of shape
                (n_samples, n_features). CSR format recommended.
            labels (np.ndarray): Target labels, shape (n_samples,).
            mbc_values (np.ndarray, optional): Mbc values per event for DisCo loss,
                shape (n_samples,).
            extra_features (np.ndarray, optional): Additional dense features concatenated
                per sample, shape (n_samples, n_extra).
            weights (np.ndarray, optional): Per-event loss weights, shape (n_samples,)
                (e.g. FEI calibration weights).
        """
        #: Feature matrix in CSR format
        self.features = features_sparse.tocsr()
        #: Target class labels
        self.labels = labels
        #: Mbc values per event for DisCo loss, or None
        self.mbc_values = mbc_values
        #: Additional dense features concatenated at retrieval time, or None
        self.extra_features = extra_features
        #: Per-event loss weights, or None
        self.weights = weights

    def __len__(self):
        """Return the number of samples in the dataset."""
        return self.features.shape[0]

    def __getitem__(self, idx):
        """Return one sample (features, label, [mbc, [weight]]) for the given index."""
        # Convert sparse row to dense 1D tensor
        feature_row = self.features[idx].toarray().astype(np.float32).squeeze()
        if self.extra_features is not None:
            feature_row = np.concatenate([feature_row, self.extra_features[idx]]).astype(np.float32)
        feature_row = torch.from_numpy(feature_row)
        label = torch.tensor(self.labels[idx], dtype=torch.long)

        if self.mbc_values is not None:
            mbc = torch.tensor(self.mbc_values[idx], dtype=torch.float32)
            if self.weights is not None:
                w = torch.tensor(self.weights[idx], dtype=torch.float32)
                return feature_row, label, mbc, w
            return feature_row, label, mbc

        if self.weights is not None:
            w = torch.tensor(self.weights[idx], dtype=torch.float32)
            return feature_row, label, w

        return feature_row, label


def sparse_collate_fn(batch):
    """Collate function for SparseDataset with optional Mbc and weight tensors."""
    if len(batch[0]) == 4:
        features, labels, mbc, weights = zip(*batch)
        return (torch.stack(features), torch.stack(labels),
                torch.stack(mbc), torch.stack(weights))
    if len(batch[0]) == 3:
        features, labels, third = zip(*batch)
        return torch.stack(features), torch.stack(labels), torch.stack(third)
    features, labels = zip(*batch)
    return torch.stack(features), torch.stack(labels)


def build_category_labels(is_cont, gen_pdg):
    """
    Build category labels (B0=0, B+=1, continuum=2) from compact MC truth scalars.

    Parameters:
        is_cont (ndarray of int8): 1 if continuum event, 0 otherwise.
        gen_pdg (ndarray of int16): mostcommonBTagPDG of the best-sigProb candidate
            (0 for continuum).

    Returns:
        ndarray of int64: Category labels (0=B0, 1=B+, 2=continuum).
    """
    abs_gen_pdg = np.abs(gen_pdg.astype(np.int32))
    known_tag = (abs_gen_pdg == 511) | (abs_gen_pdg == 521)
    return np.where(
        is_cont | (~known_tag),
        2,
        np.where(abs_gen_pdg == 521, 1, 0)
    ).astype(np.int64)


def compute_event_weights(event_scalars, mc_truth_cand, calib_inputs,
                          delta_p_thresh=config.DELTA_P_THRESH):
    """
    Compute per-event FEI calibration weights for the training loss.

    Uses the calibration weight for the highest-sigProb candidate's decay mode
    when that candidate has a truth-compatible tag PDG and DeltaP below
    delta_p_thresh; falls back to the generated decay mode weight otherwise.
    Continuum events always receive weight config.FEI_CALIB_CONT.

    Parameters:
        event_scalars (tuple): From load_and_sample_data: (is_cont, gen_pdg, bp_is_best,
            best_sigprob, best_bp_sigprob_iid, best_b0_sigprob_iid).
        mc_truth_cand (tuple): From load_and_sample_data: (best_bp_iid, best_bp_dp,
            best_b0_iid, best_b0_dp).
        calib_inputs (tuple): From load_and_sample_data: (bp_tag_is_gen, b0_tag_is_gen,
            bp_gen_dm_id, b0_gen_dm_id, bp_gen_calib_w, b0_gen_calib_w, stored_fei_calib_w).
        delta_p_thresh (float): DeltaP threshold for tag quality (default: config.DELTA_P_THRESH).

    Returns:
        ndarray of float32: Per-event FEI calibration weights, shape (n_events,).
    """
    is_cont, gen_pdg, bp_is_best, _, best_bp_sigprob_iid, best_b0_sigprob_iid = event_scalars
    _, best_bp_dp, _, best_b0_dp = mc_truth_cand
    (bp_tag_is_gen, b0_tag_is_gen, bp_gen_dm_id, b0_gen_dm_id,
     bp_gen_calib_w, b0_gen_calib_w, stored_fei_calib_w) = calib_inputs

    # Build per-sector lookup arrays (index = dmID, value = calibration weight)
    bp_calib_map = config.get_fei_calibration_map(521)
    bp_calib_rest = config.get_fei_calibration_rest(521)
    bp_lookup = np.full(config.N_BP_MODES, bp_calib_rest, dtype=np.float32)
    for dm, w in bp_calib_map.items():
        if 0 <= dm < config.N_BP_MODES:
            bp_lookup[dm] = w

    b0_calib_map = config.get_fei_calibration_map(511)
    b0_calib_rest = config.get_fei_calibration_rest(511)
    b0_lookup = np.full(config.N_B0_MODES, b0_calib_rest, dtype=np.float32)
    for dm, w in b0_calib_map.items():
        if 0 <= dm < config.N_B0_MODES:
            b0_lookup[dm] = w

    # Determine primary sector per event (sector with overall best sigProb candidate)
    use_bp = (bp_is_best == 1)
    tag_is_gen = np.where(use_bp, bp_tag_is_gen.astype(np.int8),
                          b0_tag_is_gen.astype(np.int8))
    best_dp = np.where(use_bp, best_bp_dp, best_b0_dp)
    sigprob_iid = np.where(use_bp,
                           best_bp_sigprob_iid.astype(np.int32),
                           best_b0_sigprob_iid.astype(np.int32))

    # use_reco: best-sigProb candidate has truth-compatible tag PDG, good DeltaP,
    # and a reconstructed candidate is present in this sector
    use_reco = (tag_is_gen == 1) & (best_dp < delta_p_thresh) & (sigprob_iid >= 0)

    # Start with continuum weight for all events; overwrite BB below
    weights = np.full(len(is_cont), config.FEI_CALIB_CONT, dtype=np.float32)
    bb_mask = (is_cont != 1)
    bp_threshold = config.N_BP_MODES * 2

    # --- Reco path: decode decay mode from best sigprob input_id ---
    reco_mask = bb_mask & use_reco

    reco_bp = reco_mask & use_bp
    if reco_bp.any():
        dm = np.clip(best_bp_sigprob_iid[reco_bp].astype(np.int32) // 2,
                     0, config.N_BP_MODES - 1)
        weights[reco_bp] = bp_lookup[dm]

    reco_b0 = reco_mask & ~use_bp
    if reco_b0.any():
        dm = np.clip(
            (best_b0_sigprob_iid[reco_b0].astype(np.int32) - bp_threshold) // 2,
            0, config.N_B0_MODES - 1
        )
        weights[reco_b0] = b0_lookup[dm]

    # --- Gen path: look up by generated decay mode id ---
    gen_mask = bb_mask & ~use_reco
    if gen_mask.any():
        abs_pdg = np.abs(gen_pdg.astype(np.int32))
        gen_dm = np.where(use_bp,
                          bp_gen_dm_id.astype(np.int32),
                          b0_gen_dm_id.astype(np.int32))

        gen_bp = gen_mask & (abs_pdg == 521)
        if gen_bp.any():
            dm = gen_dm[gen_bp]
            w = np.full(int(gen_bp.sum()), bp_calib_rest, dtype=np.float32)
            valid = (dm >= 0) & (dm < config.N_BP_MODES)
            if valid.any():
                w[valid] = bp_lookup[dm[valid]]
            weights[gen_bp] = w

        gen_b0 = gen_mask & (abs_pdg == 511)
        if gen_b0.any():
            dm = gen_dm[gen_b0]
            w = np.full(int(gen_b0.sum()), b0_calib_rest, dtype=np.float32)
            valid = (dm >= 0) & (dm < config.N_B0_MODES)
            if valid.any():
                w[valid] = b0_lookup[dm[valid]]
            weights[gen_b0] = w

        # Verify recomputed gen weights against stored values
        stored_gen_w = np.where(use_bp,
                                bp_gen_calib_w.astype(np.float32),
                                b0_gen_calib_w.astype(np.float32))
        check_mask = gen_mask & ((abs_pdg == 521) | (abs_pdg == 511))
        if check_mask.any():
            recomputed = weights[check_mask]
            stored = stored_gen_w[check_mask]
            mismatch = np.abs(recomputed - stored) > 1e-4
            if mismatch.any():
                print(
                    f"  WARNING: {int(mismatch.sum())} gen-path weight mismatches "
                    f"(recomputed vs stored). Max delta: "
                    f"{float(np.abs(recomputed - stored).max()):.6f}"
                )

    diff = np.abs(weights - stored_fei_calib_w.astype(np.float32))
    n_mismatch = int((diff > 1e-5).sum())
    if n_mismatch > 0:
        print(
            f"  [WARNING] compute_event_weights: {n_mismatch}/{len(weights)} events "
            f"have stored/recomputed weight mismatch "
            f"(max diff={diff.max():.6f})"
        )

    print(
        f"  Event weights: mean={weights.mean():.4f}, "
        f"min={weights.min():.4f}, max={weights.max():.4f}"
    )
    print(
        f"  Reco path: {int(reco_mask.sum())} events, "
        f"Gen path: {int(gen_mask.sum())} events, "
        f"Continuum: {int((is_cont == 1).sum())} events"
    )
    return weights


def generate_category_outputs(cat_model, features, batch_size, device, use_sparse):
    """
    Generate category softmax outputs for all samples.

    Parameters:
        cat_model (nn.Module): Trained category model in eval mode.
        features (scipy.sparse matrix or np.ndarray): Input feature matrix.
        batch_size (int): Batch size for inference.
        device (torch.device): Device to use.
        use_sparse (bool): Whether features is sparse and should be densified batch-wise.

    Returns:
        np.ndarray: Category softmax outputs with shape (n_samples, 3).
    """
    print("\nGenerating category outputs...")
    n_samples = features.shape[0]
    cat_outputs = []
    with torch.no_grad():
        for i in tqdm(range(0, n_samples, batch_size), desc="  Processing batches"):
            batch_slice = slice(i, i + batch_size)
            if use_sparse:
                batch_np = features[batch_slice].toarray().astype(np.float32)
            else:
                batch_np = features[batch_slice]
            batch = torch.from_numpy(batch_np).to(device)
            cat_out = torch.softmax(cat_model(batch), dim=1)
            cat_outputs.append(cat_out.cpu().numpy())
    return np.vstack(cat_outputs)


def main():
    """Parse command-line arguments and run the requested training."""
    parser = argparse.ArgumentParser(description='Train ModeSelector networks')
    parser.add_argument('--input', required=True, nargs='+',
                        help='One or more modeSelector_training.npz paths (shell glob or space-separated list)')
    parser.add_argument('--network', choices=['category', 'main'], required=True,
                        help='Which network to train')
    parser.add_argument('--cat_model', help='Trained category model (required for main network)')
    parser.add_argument('--output', default='networks/', help='Output directory for trained models')
    parser.add_argument('--fraction', type=float, default=1.0,
                        help='Optional uniform BB downsampling fraction after loading inputs')
    parser.add_argument('--cont_fraction', type=float, default=1.0,
                        help='Additional continuum downscale relative to --fraction at training time')
    parser.add_argument('--batch_size', type=int, default=None,
                        help='Batch size (default: 16384 for category network, 32768 for main network)')
    parser.add_argument('--num_workers', type=int, default=None,
                        help='Number of DataLoader worker processes (default: auto = min(8, max(1, cpu_count//2)))')
    parser.add_argument('--epochs', type=int, default=50, help='Number of epochs')
    parser.add_argument('--lr', type=float, default=5e-4, help='Initial learning rate')
    parser.add_argument('--lr_schedule', choices=['constant', 'cosine'], default='cosine',
                        help='Learning rate schedule (default: cosine)')
    parser.add_argument('--eta_min', type=float, default=1e-5,
                        help='Minimum learning rate for cosine schedule (default 1e-5)')
    parser.add_argument('--weight_decay', type=float, default=2e-4, help='Weight decay for AdamW')
    parser.add_argument('--val_split', type=float, default=0.3, help='Validation split')
    parser.add_argument('--seed', type=int, default=42, help='Random seed')
    parser.add_argument('--disco_lambda', type=float, default=0.0,
                        help='Distance correlation penalty coefficient (0=disabled)')
    parser.add_argument('--label_smoothing', type=float, default=0,
                        help='Label smoothing for CrossEntropyLoss (0=disabled, default)')
    parser.add_argument('--use_sparse', action='store_true',
                        help='Use sparse data loading (memory-efficient but slower)')

    args = parser.parse_args()

    if args.batch_size is None:
        args.batch_size = 2**14 if args.network == 'category' else 2**15
    print(f"Batch size: {args.batch_size}")
    if args.num_workers is None:
        cpu_count = os.cpu_count() or 1
        resolved_num_workers = min(8, max(1, cpu_count // 2))
    else:
        resolved_num_workers = args.num_workers
    print(f"DataLoader workers: {resolved_num_workers}")

    # Set random seeds
    np.random.seed(args.seed)
    torch.manual_seed(args.seed)

    # Device
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    print(f"Using device: {device}")

    # Create output directory
    os.makedirs(args.output, exist_ok=True)

    # Load input data and apply optional post-production downsampling
    print("\n" + "=" * 60)
    print("Loading data")
    print("=" * 60)
    features, event_scalars, has_inputs, mc_truth_cand, sig_truth, calib_inputs = load_and_sample_data(
        args.input,
        fraction=args.fraction,
        cont_fraction=args.cont_fraction,
        random_state=args.seed,
    )
    is_cont, gen_pdg, bp_is_best, best_sigprob, best_bp_sigprob_iid, best_b0_sigprob_iid = event_scalars

    print("\nComputing event weights...")
    event_weights = compute_event_weights(event_scalars, mc_truth_cand, calib_inputs)

    print(f"\nFeature matrix shape: {features.shape}")
    print(f"  Sparse matrix memory: {features.data.nbytes / 1024**2:.1f} MB")
    print(f"  Selected features (has_inputs): {len(has_inputs)}")
    sparse_extra_features = None

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

        if not args.use_sparse:
            print("\nConverting to dense arrays...")
            features_dense = features.toarray().astype(np.float32)
            cat_outputs = generate_category_outputs(
                cat_model, features_dense, args.batch_size, device, use_sparse=False
            )
        else:
            print("\nUsing sparse data loading (memory-efficient)...")
            features_dense = None
            cat_outputs = generate_category_outputs(
                cat_model, features, args.batch_size, device, use_sparse=True
            )

        print(f"  Category outputs shape: {cat_outputs.shape}")
        print("  Category predictions:")
        cat_preds = np.argmax(cat_outputs, axis=1)
        print(f"    B0:        {(cat_preds == 0).sum()} ({(cat_preds == 0).mean() * 100:.1f}%)")
        print(f"    B+:        {(cat_preds == 1).sum()} ({(cat_preds == 1).mean() * 100:.1f}%)")
        print(f"    Continuum: {(cat_preds == 2).sum()} ({(cat_preds == 2).mean() * 100:.1f}%)")

        # Compute charged_cat flag: 1 if B+ is predicted over B0 (matches ModeSelectorModule inference)
        charged_cat_bool = cat_outputs[:, 1] > cat_outputs[:, 0]
        charged_cat = charged_cat_bool.astype(np.float32)

        # Add category outputs + charged_cat flag to features
        print("\nAppending category outputs to input features...")
        cat_augments = np.hstack([cat_outputs, charged_cat.reshape(-1, 1)]).astype(np.float32)
        input_size = features.shape[1] + cat_augments.shape[1]
        print(f"  New feature size: {input_size}")
        if features_dense is not None:
            features_dense = np.hstack([features_dense, cat_augments]).astype(np.float32)

        # Build mode-prediction labels using category network prediction
        if mc_truth_cand is None or sig_truth is None:
            raise ValueError(
                "Required main-network truth arrays not found in training data. "
                "Re-collect training data with produceTrainingInputs.py."
            )
        labels, train_selection, label_stats = build_mode_labels(
            mc_truth_cand, sig_truth, charged_cat_bool, is_cont, gen_pdg,
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
        print("  Label assignment branches:")
        print(f"    isSignal single candidate: {label_stats['single_signal']}")
        print(f"    isSignal multi, same btag index: {label_stats['multi_same_btag']}")
        print(f"    isSignal multi, different btag index: {label_stats['multi_diff_btag']}")
        print(f"    fallback deltaP/background logic: {label_stats['fallback']}")

        mbc_values = None

        # Apply train_selection. The main network currently keeps all preselected
        # events, including predicted-sector-empty fallback-background cases.
        n_before = len(labels)
        if features_dense is not None:
            features_dense = features_dense[train_selection]
        else:
            features = features[train_selection]
            sparse_extra_features = cat_augments[train_selection]
        labels = labels[train_selection]
        event_weights = event_weights[train_selection]
        if mbc_values is not None:
            mbc_values = mbc_values[train_selection]
        n_dropped = n_before - int(train_selection.sum())
        print(
            f"  train_selection: dropped {n_dropped} events "
            f"({n_dropped / n_before * 100:.1f}%); predicted-sector-empty events are kept"
        )

    # Mbc values for DisCo loss (category network; main network handled above)
    if args.network == 'category':
        mbc_values = None

    # Train/val split
    print(f"\nSplitting train/val (val_split={args.val_split})...")
    n_events = len(labels) if features_dense is None else len(features_dense)
    n_val = int(n_events * args.val_split)
    n_train = n_events - n_val

    indices = np.random.permutation(n_events)
    train_idx = indices[:n_train]
    val_idx = indices[n_train:]

    # Split Mbc values and event weights
    if mbc_values is not None:
        mbc_train = mbc_values[train_idx]
        mbc_val = mbc_values[val_idx]
    else:
        mbc_train = None
        mbc_val = None

    w_train = event_weights[train_idx]
    w_val = event_weights[val_idx]

    print(f"  Train: {n_train} events")
    print(f"  Val:   {n_val} events")

    # Create dataloaders (sparse or dense)
    if features_dense is None:
        # Sparse loading (category and main networks)
        mbc_train_np = mbc_train if mbc_train is not None else None
        mbc_val_np = mbc_val if mbc_val is not None else None
        train_extra = sparse_extra_features[train_idx] if sparse_extra_features is not None else None
        val_extra = sparse_extra_features[val_idx] if sparse_extra_features is not None else None
        train_dataset = SparseDataset(features[train_idx], labels[train_idx],
                                      mbc_train_np, train_extra, w_train)
        val_dataset = SparseDataset(features[val_idx], labels[val_idx],
                                    mbc_val_np, val_extra, w_val)
        drop_last = len(train_dataset) > args.batch_size
        train_loader = DataLoader(train_dataset, batch_size=args.batch_size, shuffle=True,
                                  collate_fn=sparse_collate_fn, num_workers=resolved_num_workers,
                                  drop_last=drop_last)
        val_loader = DataLoader(val_dataset, batch_size=args.batch_size, shuffle=False,
                                collate_fn=sparse_collate_fn, num_workers=resolved_num_workers)
    else:
        # Dense loading (default)
        X_train = torch.from_numpy(features_dense[train_idx])
        y_train = torch.from_numpy(labels[train_idx])
        X_val = torch.from_numpy(features_dense[val_idx])
        y_val = torch.from_numpy(labels[val_idx])
        w_train_t = torch.from_numpy(w_train)
        w_val_t = torch.from_numpy(w_val)

        if mbc_train is not None:
            # Include mbc in dataset so shuffling aligns correctly with DisCo
            train_dataset = TensorDataset(X_train, y_train, mbc_train, w_train_t)
            val_dataset = TensorDataset(X_val, y_val, mbc_val, w_val_t)
        else:
            train_dataset = TensorDataset(X_train, y_train, w_train_t)
            val_dataset = TensorDataset(X_val, y_val, w_val_t)

        drop_last = len(train_dataset) > args.batch_size
        train_loader = DataLoader(train_dataset, batch_size=args.batch_size, shuffle=True,
                                  num_workers=resolved_num_workers, drop_last=drop_last)
        val_loader = DataLoader(val_dataset, batch_size=args.batch_size, shuffle=False,
                                num_workers=resolved_num_workers)

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
    # reduction='none' gives per-sample losses; weights are applied in train_epoch and evaluate
    criterion = nn.CrossEntropyLoss(label_smoothing=args.label_smoothing, reduction='none')
    optimizer = optim.AdamW(model.parameters(), lr=args.lr, weight_decay=args.weight_decay)
    scheduler = None
    if args.lr_schedule == 'cosine':
        scheduler = optim.lr_scheduler.CosineAnnealingLR(
            optimizer, T_max=args.epochs, eta_min=args.eta_min
        )

    # Training loop
    print("\n" + "=" * 60)
    print("Training")
    print("=" * 60)
    if args.disco_lambda > 0:
        print(f"Using distance correlation with lambda={args.disco_lambda}")
    print("Training configuration:")
    print(f"  epochs: {args.epochs}")
    print(f"  batch_size: {args.batch_size}")
    print(f"  lr: {args.lr:.2e}")
    print(f"  lr_schedule: {args.lr_schedule}")
    if args.lr_schedule == 'cosine':
        print(f"  eta_min: {args.eta_min:.2e}")
    print(f"  label_smoothing: {args.label_smoothing}")

    best_val_loss = float('inf')
    best_epoch = 0
    training_start = time.time()
    history = {'train_loss': [], 'val_loss': [], 'disco_loss': [], 'lr': []}
    patience_counter = 0
    early_stopping_patience = 5

    for epoch in range(args.epochs):
        epoch_start = time.time()
        current_lr = optimizer.param_groups[0]['lr']

        # Train
        train_loss, disco_loss = train_epoch(
            model, train_loader, criterion, optimizer, device,
            disco_lambda=args.disco_lambda
        )

        val_loss = evaluate(model, val_loader, criterion, device)

        epoch_time = time.time() - epoch_start

        # Scheduler step
        if scheduler is not None:
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
                    'lr': args.lr,
                    'lr_schedule': args.lr_schedule,
                    'eta_min': args.eta_min,
                    'label_smoothing': args.label_smoothing,
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
