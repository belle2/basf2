#!/usr/bin/env python3
"""
Convert produceTrainingInputs.py ROOT outputs into compact .npz shards for train.py.

Reading the ROOT training inputs directly is the dominant cost of a training run: the
'events' tree carries one branch per feature (1644 ms_feat_* branches, ~1667 total) split
into ~50k small baskets per file, which costs ~11 s per file even with an explicit branch
list. On the full v7 sample (14364 files, 320 GB) that is ~44 core-hours -- paid again for
every train.py invocation, i.e. twice per submission since the category and main networks
are trained in separate processes.

The features are only ~1.6% dense, so the same data stored as CSR is ~211 bytes/event:
the full sample becomes roughly 20 GB of .npz that loads in minutes. This script does the
conversion once, parallel over files within a job and over input directories across
HTCondor jobs.

No selection is applied here -- shards hold exactly what _load_root_training_file()
returns, so --fraction/--cont_fraction/--sigprob_thresh stay train-time knobs and do not
require reconverting.

Usage:
    # one shard from one gbasf2 dataset directory (the usual HTCondor job)
    python3 convert_training_inputs.py \
        --input '/path/to/ModeSelector_v7/ModeSelector_v7_ccbar_1/**/*.root' \
        --output /path/to/converted \
        --name ModeSelector_v7_ccbar_1

    # everything in one go, split into shards of 200 input files
    python3 convert_training_inputs.py \
        --input '/path/to/ModeSelector_v7/**/*.root' \
        --output /path/to/converted \
        --name ModeSelector_v7 --files_per_shard 200

Then train on the result:
    python3 train.py --input '/path/to/converted/*.npz' --network category --use_sparse
"""

import argparse
import glob
import os
import sys
import time
from concurrent.futures import ProcessPoolExecutor

from tqdm import tqdm

try:
    from modeSelector.train import concat_file_data, load_training_file, save_npz_shard
except Exception:
    # Same reason (and same deliberate breadth) as the config import in train.py: the
    # modeSelector package __init__ pulls in basf2 and ROOT, which a standalone venv does
    # not have, and pybasf2 fails with SystemError rather than ImportError when a basf2
    # PYTHONPATH is inherited. train.py sits next to this file, so import it by path.
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    from train import concat_file_data, load_training_file, save_npz_shard


def _load_one(input_file):
    """
    Load one input file, returning (data, error) so a single bad file cannot abort a job.

    Parameters:
        input_file (str): Path to a produceTrainingInputs.py ROOT file.

    Returns:
        tuple: (data, error). On success `data` is the per-file dict from
        _load_root_training_file() and `error` is None; on failure `data` is None and
        `error` is a message naming the file and the exception.
    """
    try:
        return load_training_file(input_file), None
    except Exception as exc:
        return None, f"{input_file}: failed to read input file ({exc})"


def convert_shard(input_files, out_path, n_workers):
    """
    Load `input_files` in parallel and write them as a single .npz shard.

    Parameters:
        input_files (list of str): Input files making up this shard.
        out_path (str): Output .npz path.
        n_workers (int): Number of loader processes.

    Returns:
        tuple: (n_events, n_failed) -- events written and inputs skipped as unreadable.
    """
    data_list = []
    errors = []

    with ProcessPoolExecutor(max_workers=n_workers) as executor:
        results = executor.map(_load_one, input_files)
        for data, error in tqdm(results, total=len(input_files),
                                desc=os.path.basename(out_path)):
            if error is not None:
                errors.append(error)
                continue
            data_list.append(data)

    for err in errors[:20]:
        print(f"  WARNING: {err}")
    if len(errors) > 20:
        print(f"  WARNING: ... and {len(errors) - 20} more unreadable files")

    if not data_list:
        raise ValueError(f"{out_path}: all {len(input_files)} input files failed to load.")

    merged = concat_file_data(data_list)
    del data_list

    os.makedirs(os.path.dirname(os.path.abspath(out_path)), exist_ok=True)
    save_npz_shard(out_path, merged)

    return merged['features'].shape[0], len(errors)


def main():
    """Parse command-line arguments and convert the requested inputs."""
    parser = argparse.ArgumentParser(
        description='Convert ModeSelector ROOT training inputs to .npz shards')
    parser.add_argument('--input', required=True, nargs='+',
                        help='One or more ROOT paths (shell glob or space-separated list). '
                             'Quote patterns containing ** so glob expands them recursively.')
    parser.add_argument('--output', required=True,
                        help='Output directory for the .npz shard(s)')
    parser.add_argument('--name', default='shard',
                        help='Base name for the output shard(s) (default: shard)')
    parser.add_argument('--files_per_shard', type=int, default=None,
                        help='Split the inputs into shards of this many files '
                             '(default: one shard for all inputs)')
    parser.add_argument('--num_workers', type=int, default=None,
                        help='Loader processes (default: min(16, cpu_count()))')
    parser.add_argument('--overwrite', action='store_true',
                        help='Rewrite shards that already exist (default: skip them)')

    args = parser.parse_args()

    expanded = []
    for pattern in args.input:
        matches = glob.glob(pattern, recursive=True)
        expanded.extend(matches if matches else [pattern])
    input_files = sorted(expanded)

    if not input_files:
        raise ValueError("No input files found.")

    n_workers = args.num_workers
    if n_workers is None:
        n_workers = min(16, os.cpu_count() or 1)
    n_workers = max(1, min(n_workers, len(input_files)))

    files_per_shard = args.files_per_shard or len(input_files)
    shards = [input_files[i:i + files_per_shard]
              for i in range(0, len(input_files), files_per_shard)]

    print(f"Input files:     {len(input_files)}")
    print(f"Shards to write: {len(shards)}")
    print(f"Loader workers:  {n_workers}")
    print(f"Output dir:      {args.output}")

    os.makedirs(args.output, exist_ok=True)

    total_events = 0
    total_failed = 0
    t_start = time.time()

    for i, shard_files in enumerate(shards):
        suffix = '' if len(shards) == 1 else f"_{i:04d}"
        out_path = os.path.join(args.output, f"{args.name}{suffix}.npz")

        if os.path.exists(out_path) and not args.overwrite:
            print(f"\nSkipping existing shard {out_path} (use --overwrite to rewrite)")
            continue

        print(f"\n[{i + 1}/{len(shards)}] {out_path} <- {len(shard_files)} files")
        t0 = time.time()
        n_events, n_failed = convert_shard(shard_files, out_path, n_workers)
        dt = time.time() - t0
        size_mb = os.path.getsize(out_path) / 1024**2
        print(f"  {n_events} events, {size_mb:.1f} MB, {dt:.0f} s "
              f"({dt / max(1, len(shard_files)):.1f} s/file)")
        total_events += n_events
        total_failed += n_failed

    dt = time.time() - t_start
    print(f"\nDone: {total_events} events in {len(shards)} shard(s), "
          f"{total_failed} unreadable file(s), {dt / 60:.1f} min")

    return 0


if __name__ == '__main__':
    sys.exit(main())
