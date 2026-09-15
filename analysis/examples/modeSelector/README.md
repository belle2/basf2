# ModeSelector example scripts

Example basf2 steering scripts for the ModeSelector neural network module.
These scripts demonstrate how to run ModeSelector in inference mode and how to
produce training inputs from simulation data.
Both scripts expect FEI-skimmed uDST files as inputs, where the B meson candidate lists are present.

For a complete description of the ModeSelector module and all configuration
options, see `analysis/scripts/modeSelector/README.md`.

---

## Scripts

### `produceTrainingInputs.py` -- training data collection

Runs the ModeSelector in `training_mode=True` to extract feature arrays and MC
truth variables from simulation, without performing NN inference. The output
is used for offline training with `train.py`.

**Usage:**

```bash
basf2 produceTrainingInputs.py -- \
    --input <input_file.root> \
    --output <output_prefix> \
    [--cont-fraction 0.25]
```

| Argument | Default | Description |
|----------|---------|-------------|
| `--input` | validation file | Input ROOT file(s); defaults to `udst16_feiHadronic.root` from the validation data |
| `--output` | `modeSelector_training` | Output prefix for the `.root` file |
| `--cont-fraction` | `0.25` | Continuum keep fraction relative to the 30% BB base band |

Random seed is fixed to 1337 for reproducible `eventRandom` cuts. 
The training inputs keep the low `eventRandom` region with
`eventRandom < 0.3` for BB events and `eventRandom < 0.3 * cont_fraction` for
continuum events. The inference example uses the complementary high band
`eventRandom > 0.95`.
The same seed should be set to retain independent samples.

**Output file:**

The script writes `<output_prefix>.root` via two `variablesToNtuple` calls added
to the path (no manual file I/O happens inside the module -- see "Training mode"
in `analysis/scripts/modeSelector/README.md`). Writing through basf2's own
output modules is what makes this output downloadable when the script is run
with `gbasf2` (see "Grid submission" below).

**`events` tree** -- one row per event:
- **Features**: `ms_feat_0000` .. `ms_feat_1643`, the flattened 1644-value raw
  feature array (12 feature blocks x 136 input_ids, plus 12 event-level scalars:
  event shape variables, `ncandidates`, `max_input_id`, `scnd_max_input_id`,
  `__experiment__`). `config.HAS_INPUTS` (976 indices) selects the
  non-trivially-zero columns actually used for training.
- **Event flags**: `ms_tr_is_cont` (continuum flag), `ms_tr_gen_pdg` (generated
  tag-B PDG), `ms_tr_bp_is_best` (B+ candidate has highest overall sigProb),
  `ms_tr_best_sigprob` (highest sigProb across all candidates)
- **Generated decay**: `ms_tr_bp/b0_gen_decay_mode_id` (generated FEI mode
  index), `ms_tr_bp/b0_gen_fei_calib_weight` (FEI calibration weight from
  generated decay), `ms_tr_bp/b0_tag_is_gen` (truth-compatible tag PDG flag)
- **Best-candidate truth** (pre-filtered for truth-compatible tag PDG; fallback
  labels when no `isSignal==1` candidate passes `DeltaP < 0.15`):
  `ms_tr_best_bp/b0_iid` (input_id), `ms_tr_best_bp/b0_dp` (DeltaP),
  `ms_tr_best_bp/b0_sigprob_iid` (input_id of highest-sigProb candidate per sector)
- **Calibration weight**: `ms_tr_fei_calib_weight` (event-level FEI calibration
  weight: reco-based when a truth-compatible candidate with `DeltaP < threshold`
  exists, generated-decay-based otherwise)

**`sig_candidates` tree** -- one row per B+/B0 candidate (correctly reconstructed
candidates, `isSignal==1`, are the primary label source for
`train.build_mode_labels()`): `ms_sig_input_id` (set only on the deduplicated
best-per-`input_id` signal candidate, `NaN` otherwise), `ms_sig_btag_index`
(`mostcommonBTagIndex`), `ms_sig_delta_p` (`mostcommonBTagDeltaP`),
`ms_sig_sigprob` (`SignalProbability`). `train.py`'s loader joins these rows
back to their event via the automatic `__experiment__`/`__run__`/`__event__`
columns present on both trees and reassembles the same packed ragged arrays
the training pipeline has always used internally.

---

### Grid submission

`produceTrainingInputs.py` can be submitted directly with `gbasf2` -- the same
steering file is used locally and on the grid. Test locally first:

```bash
basf2 produceTrainingInputs.py -- --input <local_test_file.root> --output test_train -n 200
```

Then submit:

```bash
gbasf2 produceTrainingInputs.py -p modeSelector_train_<date> -s <release> \
       -i <grid input dataset LPN> \
       -- --output modeSelector_training

gb2_ds_get modeSelector_train_<date>
```

gbasf2 splits jobs per input file, so the download will contain many small
`<output_prefix>.root` files (one per job). Point `train.py` at all of them with
a glob:

```bash
python3 analysis/scripts/modeSelector/train.py \
    --input modeSelector_train_<date>/**/*.root --network category --use_sparse
```

`--input` is not passed explicitly on the grid job itself: gbasf2 overrides the
`RootInput` file list with the job's assigned grid input file regardless of what
`inputMdstList` was given in the script, so the script's `--input` default is
never used on the grid. See `online_book/computing/gbasf2.rst`
for the general gbasf2 workflow (dataset discovery, monitoring, downloading).

---

### `plot_training.py` -- training diagnostics

Plots training diagnostics from one or more `.pt` checkpoints.

**Usage:**

```bash
python3 plot_training.py networks/net_category.pt networks/net_main.pt --output plots/
```

| Argument | Default | Description |
|----------|---------|-------------|
| `checkpoints` | required | One or more `.pt` checkpoint paths |
| `--output` | (interactive) | Directory to save PDF plots; omit to show interactively |

**Plots produced per checkpoint:**

| File | Contents |
|------|----------|
| `<name>_loss.pdf` | Train/val loss curves and learning rate schedule per epoch |
| `<name>_weights.pdf` | Weight value histograms per layer |
| `<name>_importance.pdf` | L2 norm of first-layer weights per input feature (top 10 annotated with global feature index) |

---

### `applyModeSelector.py` -- inference

Applies the trained ModeSelector neural network to FEI hadronic tag output and
saves candidate-level and event-level scores to parquet tables.

**Usage:**

```bash
basf2 applyModeSelector.py -- [options]
```

| Argument | Default | Description |
|----------|---------|-------------|
| `--input` | validation file | Input ROOT file(s); defaults to `udst16_feiHadronic.root` from the validation data |
| `--output` | `modeSelector_output` | Output file prefix |
| `--cat-model` | unset | Category MVA ONNX weightfile (`.root`, from `convert_to_onnx.py`). Omit to use payloads |
| `--main-model` | unset | Main MVA ONNX weightfile (`.root`, from `convert_to_onnx.py`). Omit to use payloads |
| `--cat-payload-name` | derived | Conditions DB payload name for the category model; omit to use the name derived from the contract version |
| `--main-payload-name` | derived | Conditions DB payload name for the main model; omit to use the name derived from the contract version |
| `--globaltag` | unset | Additional globaltag holding the payloads, prepended to the analysis globaltag |
| `--data` | off | Run on data: keep a fixed 10% `eventRandom` sample and drop MC-only output columns |

**Output files:** `<output>.pq`, written via `VariablesToTable` from
`b2pandas_utils`. The table stores merged B+ and B0 candidates as the daughter
of `Upsilon(4S):all`, so output columns are prefixed with `B_`.

| Column | MC only | Description |
|--------|---------|-------------|
| `Mbc`, `deltaE`, `M` | no | Basic kinematics |
| `cosTBTO` | no | Continuum suppression variable |
| `sigProb` | no | FEI signal probability (`extraInfo(SignalProbability)`) |
| `dmID` | no | FEI decay mode ID (`extraInfo(decayModeID)`) |
| `modeSelector_eqSigProb` | no | Mode probability `main_output[input_id]` (predicted sector only) |
| `BplusScore` | no | Event-level score = `sign * score`, where `score` is the max predicted-sector candidate probability if a candidate exists, else `max(0, 2 * (sum(predicted-sector signal outputs + bad_tag) - 0.5))` |
| `modeSelector_catB0` | no | Category network B0 probability |
| `modeSelector_catBp` | no | Category network B+ probability |
| `modeSelector_catCont` | no | Category network continuum probability |
| `PDG` | no | PDG code of reconstructed B candidate (511=B0, 521=B+) |
| `Dstp_deltaMassDiff`, `Dst0_deltaMassDiff` | no | D* veto mass difference |
| `Dstp_chiProb`, `Dst0_chiProb` | no | D* veto vertex fit quality |
| `sigProb_rank` | no | Rank within list by sigProb (1 = best; all sigProb>0.001 candidates kept) |
| `modeSelector_rank` | no | Sector-local rank: 1 for highest `modeSelector_eqSigProb` in predicted sector; 1 for highest `sigProb` in non-predicted sector (crossfeed-enhanced control sample) |
| `isBestCandidate_sigProb` | no | 1 for the rank-1 sigProb candidate in the sector with the highest sigProb |
| `eventRandom` | no | Event-level random variable (for reproducible sampling) |
| `genDecayModeID` | yes | Generated FEI decay mode ID, `999` for rest, and `-1` for continuum or missing generated `B` truth; calibrated-only by default |
| `genFEICalibWeight` | yes | FEI calibration weight from the generated decay identified by `mostcommonBTagIndex` |
| `modeSelector_feiCalibWeight` | yes | Event-level FEI calibration weight from the reco path of the overall highest-sigProb candidate, chosen by comparing the best B+ and best B0 candidates; stored only when that candidate has truth-compatible tag PDG and DeltaP < threshold; `NaN` when reco conditions are not met; only written when `store_fei_calib_weight=True` |
| `isSignal` | yes | MC truth match flag |
| `isContinuumEvent` | yes | 1 for continuum events, 0 for BB |
| `mostcommonBTagDeltaP`, `mostcommonBTagPDG` | yes | MC B-tag truth variables |

**Selection and ranking**

`modeSelector_eqSigProb` is written only on candidates in the predicted sector
(B+ if `BplusScore > 0`, B0 if `BplusScore < 0`). `sigProb_rank` is the pure
sigProb ranking, while `modeSelector_rank` is written by the module on the
deduplicated candidates. The example then keeps at most 2 candidates per list:
`[sigProb_rank == 1] or [modeSelector_rank == 1]`.
`isBestCandidate_sigProb` is 1 for the rank-1 sigProb candidate in whichever
sector (B+ or B0) has the higher rank-1 sigProb; it is stored for offline
global best candidate selection based purely on sigProb.

The signed category choice can be reconstructed offline from
`modeSelector_catBp > modeSelector_catB0`. If the predicted sector has no
candidate, `BplusScore` falls back to the normalized sum of the
predicted-sector signal classes plus `bad_tag`:
`max(0, 2 * (sum_mode_prob - 0.5))`.
To restrict to the predicted sector only, require `BplusScore > 0` (B+ analyses)
or `BplusScore < 0` (B0 analyses).

**Sampling**

Random seed is fixed to 1337 for reproducible `eventRandom` cuts. In MC mode the
script keeps events `eventRandom > 0.95`, disjoint from the low `eventRandom`
training sample. After candidate selection, `addGeneratedDecayWeights(...)` stores
`genDecayModeID` and `genFEICalibWeight`. With `--data`, MC truth matching and
MC-only output columns are skipped, and `eventRandom < 0.1` is applied instead.

**Model loading**

Omit `--cat-model` and `--main-model` to load models from the conditions database. Pass local MVA ONNX weightfile paths to override.

The payload names carry no training version: which training is used is decided by
the performance globaltag that is prepended, not by `--cat-payload-name` /
`--main-payload-name`. Leave them unset in normal use; giving a name always emits a
`B2WARNING`. Naming the payloads explicitly is also how an older contract version still
supported by the module is loaded. See "conditions database payloads" in
`analysis/scripts/modeSelector/README.md`.

The script prepends `getAnalysisGlobaltag()`, which does not contain the
ModeSelector payloads, so the performance globaltag serving them has to be
given explicitly:

```bash
basf2 applyModeSelector.py -- --globaltag <tag holding the modeSelector payloads>
```

Alternatively point `--cat-model` / `--main-model` at the local
`modeSelector_cat.root` / `modeSelector_main.root` weightfiles produced by
`convert_to_onnx.py`, which needs no conditions database access at all.

---

## End-to-end training workflow

```
1. Produce training inputs:
   basf2 analysis/examples/modeSelector/produceTrainingInputs.py

2. Train category network:
   python3 analysis/scripts/modeSelector/train.py \
           --input training_data/*.root --network category --use_sparse

3. Train main network:
   python3 analysis/scripts/modeSelector/train.py \
           --input training_data/*.root --network main --cat_model networks/net_category.pt --use_sparse
           
4. Plot training diagnostics
   python3 analysis/examples/modeSelector/plot_training.py networks/net_category.pt networks/net_main.pt

5. Export to ONNX and produce payloads:
   python3 analysis/scripts/modeSelector/convert_to_onnx.py \
           --input-dir networks/ --output-dir onnx/ --add-payloads

6. Apply to MC/data for testing:
   basf2 analysis/examples/modeSelector/applyModeSelector.py
```

See `analysis/scripts/modeSelector/README.md` for detailed documentation of
the module behavior and the supporting scripts.
