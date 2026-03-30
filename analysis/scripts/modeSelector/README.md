# ModeSelector

ModeSelector is an event-level B meson classifier on top of the FEI.
The FEI assigns a signal probability (`sigProb`) to each B candidate independently, using only that candidate's own decay products.
ModeSelector instead uses the full set of FEI B candidates with `sigProb > 0.001` simultaneously to classify the event and produce a combined score.
Its main strength is separating B0 from B+ events, which significantly reduces crossfeed in analyses that do not fully reconstruct the signal side.

## Overview

The FEI reconstructs many B meson decay modes per event, each of which is assigned a signal probability.
The ModeSelector takes all candidates (but at most 1 per input_id slot, where each slot encodes a unique combination of B type, decay mode, and particle/antiparticle sign) together, encoded as a sparse feature matrix, and classifies the event into physics categories.

### Two-stage network

```
All B candidates ──> [Feature matrix] ──> Category network ──> B0 / B+ / continuum
                                               │
                                               ▼
                                         Main network  ──> 139-class output ──> BplusScore
```

**Category network** (3 outputs): classifies the event as B0, B+, or continuum.

**Main network** (139 outputs): uses features + category output to predict which specific
FEI decay mode produced the tag B (`N_INPUT_IDS + 3 = 139` classes total):

| Class range | Description |
|-------------|-------------|
| 0 to 135 | Signal modes: class index equals the candidate's `input_id` directly |
| 136 | `bad_tag` - combinatorial background |
| 137 | `cross_deltaC1` - BB crossfeed, different B charge type |
| 138 | `continuum` - qq̄ continuum events |

The final **`BplusScore`** is `sign * score`, where:
- `sign = +1` if the category network predicts B+, `sign = -1` if B0
- `score` is the maximum over `main_output[input_id]` among candidates
  present in the predicted sector for this event
  (`input_id < N_BP_MODES*2` for B+ prediction, `>= N_BP_MODES*2` for B0 prediction)
  (background classes 136-138 are excluded)

The module also monitors how often the top predicted-sector mode has no
candidate in the event, and logs missing-top-mode and fallback counts at the
end of the job.

If the predicted sector has no candidate at all, the event-level score uses a
defensive fallback:

`sum_mode_prob = sum(predicted-sector signal outputs) + bad_tag`

`fallback_score = max(0, 2 * (sum_mode_prob - 0.5))`

This maps sums below `0.5` to `0` so low-confidence events do not pick up an
artificial signed score.

Per-candidate **`modeSelector_eqSigProb`** = `main_output[input_id]`: the probability the
network assigns to that candidate's specific decay mode. Set only on candidates in the
predicted sector. The module also writes **`modeSelector_rank`** on the deduplicated
representative candidates: the predicted sector is ranked by `modeSelector_eqSigProb`,
while the non-predicted sector is ranked independently by `sigProb`.

---

## Files

### `config.py` -- shared configuration

Single source of truth for all constants, feature definitions, and calibration factors used
by both inference (`ModeSelectorModule.py`) and training (`train.py`), which must stay in sync.

Key contents:

| Constant | Value | Description |
|----------|-------|-------------|
| `N_BP_MODES` | 36 | Number of B+/B- FEI decay modes (dmID 0-35) |
| `N_B0_MODES` | 32 | Number of B0/anti-B0 FEI decay modes (dmID 0-31) |
| `N_INPUT_IDS` | 136 | Total input_id slots (`N_BP_MODES*2 + N_B0_MODES*2`) |
| `NUM_CAT_LABELS` | 3 | Category network output size |
| `DELTA_M_CUT` | (-0.05, 0.05) | D* delta mass difference window |
| `DELTA_P_THRESH` | 0.15 | Threshold for `mostcommonBTagDeltaP` in good-tag fallback truth |

Main network output size is `N_INPUT_IDS + 3 = 139` (fixed; derived from `N_INPUT_IDS`).
`NUM_MAIN_LABELS` in `config.py` is informational and not used for the runtime main-network output size.

**input_id encoding** - each FEI candidate is mapped to a unique slot:

| Sector | input_id range | PDG | Formula |
|--------|---------------|-----|---------|
| B+ sector | 0 to 2*N_BP_MODES-1 (0-71) | -521 (anti-B+) | `dmID * 2 + 0` |
| B+ sector | 0 to 2*N_BP_MODES-1 (0-71) | +521 (B+) | `dmID * 2 + 1` |
| B0 sector | 2*N_BP_MODES to N_INPUT_IDS-1 (72-135) | -511 (anti-B0) | `N_BP_MODES*2 + dmID * 2 + 0` |
| B0 sector | 2*N_BP_MODES to N_INPUT_IDS-1 (72-135) | +511 (B0) | `N_BP_MODES*2 + dmID * 2 + 1` |

**`FEATURE_BLOCKS`** - 12 feature blocks (9 currently used), each of size 136, one slot per input_id:

| Block | Name | basf2 variable | Transform |
|-------|------|---------------|-----------|
| 0 | `sigProb` | `extraInfo(SignalProbability)` | `1 + val×100` |
| 1 | `chiProb` | `chiProb` | `2 + val` |
| 2 | `Bdaughter_sigProb` | `daughter(0, extraInfo(SignalProbability))` | `1 + val×10` |
| 3 | `Bdaughter2_sigProb` | `daughter(1, extraInfo(SignalProbability))` | `1 + val×10` |
| 4 | `Bdaughter_chiProb` | `daughter(0, chiProb)` | `2 + val` |
| 5 | `Dst0_deltaMassDiff` | `extraInfo(Dst0_deltaMassDiff)` | `1 + val×20 + 0.5` |
| 6 | `Dstp_deltaMassDiff` | `extraInfo(Dstp_deltaMassDiff)` | `1 + val×20 + 0.5` |
| 7 | `Dst0_chiProb` | `extraInfo(Dst0_chiProb)` | `2 + val` - **excluded from training for `skipTreeFit=True`** |
| 8 | `Dstp_chiProb` | `extraInfo(Dstp_chiProb)` | `2 + val` - **excluded from training for `skipTreeFit=True`** |
| 9 | `deltaE` | `deltaE` | `1 + val×5 + 0.75` |
| 10 | `Mbc` | `Mbc` | `1 + (val−5.23)×20` - **excluded from training** |
| 11 | `cosTBTO` | `cosTBTO` | `1 + val` |

Blocks 7-8 (`Dst0_chiProb`, `Dstp_chiProb`, indices 952-1223) are excluded via
`HAS_INPUTS` for the current default `skipTreeFit=True` setup: when populated,
they duplicate `Bdaughter_chiProb`, and otherwise they are not available for
veto-reconstructed D* candidates. Block 10 (Mbc, indices 1360-1495) is excluded
to prevent correlation with the output.

**`EVENT_FEATURES`** - 12 event-level scalars appended after the flat feature matrix
(indices 1632-1643):

| Index | Name | Source | Transform |
|-------|------|--------|-----------|
| 1632 | `sphericity` | `EventShapeContainer` | raw |
| 1633 | `thrust` | `EventShapeContainer` | raw |
| 1634 | `thrustAxisCosTheta` | `EventShapeContainer` | raw |
| 1635 | `aplanarity` | `EventShapeContainer` | raw |
| 1636 | `foxWolframR2` | `EventShapeContainer` | raw |
| 1637 | `harmonicMomentThrust0` | `EventShapeContainer` | raw |
| 1638 | `harmonicMomentThrust1` | `EventShapeContainer` | raw |
| 1639 | `harmonicMomentThrust2` | `EventShapeContainer` | raw |
| 1640 | `ncandidates` | total number of B candidates before deduplication | `val / 10` |
| 1641 | `max_input_id` | input_id of best candidate (highest sigProb) | `val / 50` |
| 1642 | `scnd_max_input_id` | best input_id from the other B type; -10 if absent | `val / 50` |
| 1643 | `__experiment__` | `EventMetaData` | `val / 10` |

In inference mode, the raw `EventMetaData` experiment id is accepted only for
values present in the training sample. Any other value (such as `1003`), 
is replaced with a default value. Both accepted and default value are defined in `config.py`.

**`HAS_INPUTS`** - Hardcoded list of feature indices to keep (all-zero columns
removed, blocks 7-8 and 10 excluded). To update, recompute with `train.py`
(writes `has_inputs_recomputed.txt` on mismatch) and paste the result into `config.py`.

**`FEI_CALIB_*`** - Per-decay-mode FEI calibration weights for two sigProb
working points (0.001, 0.01).
The calibration corresponds to [release 8, run1 + run2](https://gitlab.desy.de/belle2/performance/correction-tables/-/tree/20260126/MC16/FEI/hadronic?ref_type=tags).
 `config.DEFAULT_FEI_SIGPROB_THRESHOLD` selects which set is used. 
 `FEI_CALIB_CONT` is the weight for continuum events (default `1.0`).

### conditions database payloads

When `cat_model_path` and `main_model_path` are omitted, models are loaded from
the conditions database via payloads.

To create a local payload database use:

```bash
python3 convert_to_onnx.py --input-dir networks/ --output-dir onnx/ --add-payloads
```

This creates `localdb/database.txt`. To test, prepend it in a steering file with:

```python
basf2.conditions.prepend_testing_payloads('localdb/database.txt')
```

### `generatedDecayWeights.py` -- add generated-decay calibration weights

Adds candidate-level generated-decay calibration information after FEI selection and MC truth
matching:

```python
import modeSelector

modeSelector.addGeneratedDecayWeights(
    bp_list='B+:feiHadronic',
    b0_list='B0:feiHadronic',
    use_calibrated_dmids_only=True,
    store_btag_candidate_signature=False,
    path=my_path,
)
```

The module derives the generated decay for the `B` identified by
`mostcommonBTagIndex`, matches it to the FEI `dmID` convention via the
generated final-state content, and stores:

- `extraInfo(genDecayModeID)`
- `extraInfo(genFEICalibWeight)`

Stored `genDecayModeID` values:
- calibrated FEI `dmID` for modes with an explicit calibration factor
- `999` for all other generated `B0` or `B+` decays using the sector `rest`
  calibration
- `-1` for continuum or missing generated `B` truth (a `B2ERROR` is emitted
  for any non-continuum candidate that reaches this case)

`genFEICalibWeight` stores the corresponding calibration weight (`1.0` for
continuum or missing generated truth). Set `use_calibrated_dmids_only=False`
to store the exact generated FEI `dmID` for all matched modes instead of
collapsing uncalibrated modes to `999`.

---

### `dstarVeto.py` -- D* veto reconstruction

Identifies FEI candidates where a `B → D X` was reconstructed but the true decay was
`B → D* X`. Adds D* mass difference and vertex-fit quality as `ExtraInfo` on B candidates,
which the ModeSelector uses as features (blocks 5–8).
It registers basf2 variable aliases used throughout the veto reconstruction
(`deltaMassDiff`, `deltaMassDiffInvM`, `dmID`, etc.).

```python
from modeSelector.dstarVeto import addDstarVeto

addDstarVeto(
    particleLists=['B+:feiHadronic', 'B0:feiHadronic'],
    path=my_path,
    deltaMassDiffCut=(-0.02, 0.02),  # GeV
    dMassCut=(-0.03, 0.03),          # GeV
    skipTreeFit=True,                # False = much slower but provides chiProb
)
```

**For each B candidate list**, two types are handled:

1. **Already has D\* daughter** (`PDG ∈ {413, 423}`): reads the `deltaMassDiff` alias
   (`massDifference(0) − trueMassDiff`) and chiProb from the existing D* candidate directly.
   chiProb is always available here (from the FEI vertex fit), regardless of `skipTreeFit`.
2. **Has D0 or D+ daughter**: reconstructs D\* candidates from ROE soft pions/pi0s.
   - `D0` → tries `D*+ → D0 π+` and `D*0 → D0 π0`
   - `D+` → tries `D*+ → D+ π0`

**ExtraInfo fields added to B candidates:**

| Field | Meaning |
|-------|---------|
| `Dstp_deltaMassDiff` | `massDifference(0) − trueMassDiff` for D*+ |
| `Dstp_chiProb` | Vertex fit χ² probability for D*+ (NaN if `skipTreeFit` and no pre-existing D*) |
| `Dst0_deltaMassDiff` | Same for D*0 |
| `Dst0_chiProb` | Same for D\*0 (NaN if `skipTreeFit` and no pre-existing D\*) |

**Key parameters:**
- `skipTreeFit=True` (default): skip `treeFit` for reconstructed D* candidates —
  large speedup at slight accuracy cost. When `True`, `Dst*_chiProb` is not
  written for veto-reconstructed candidates and is manually excluded from `HAS_INPUTS`.
- `deltaMassDiffCut`: window on reconstructed minus true D* mass difference
  (default `(-0.02, 0.02)` GeV).

---

### `ModeSelectorModule.py` -- prerequisites

**Candidate preselections** must be applied before running the module.
The cuts `Mbc > 5.23`, `-0.15 < deltaE < 0.1`, and `cosTBTO < 0.9` were used
both during training and in the FEI calibration and must be reproduced at inference
time. `cosTBTO` requires the rest of event and continuum suppression to be built first.
The exact cut values are defined in `config.py` as `PRESELECTION_*` constants.
The module checks compliance at runtime and issues a `B2WARNING` at the end of the
job if any candidates fail a cut.

Two modules must run before `ModeSelectorModule` in the basf2 path:

1. **`EventShapeCalculator`** - computes event-shape variables (`sphericity`, `thrust`, etc.)
   stored in `EventShapeContainer`. The 8 event-shape features (indices 1632-1639) are read from there.
2. **`DstarVeto`** (`dstarVeto.py`), runs automatically when modeSelector is called - 
   adds `Dst0_deltaMassDiff`, `Dstp_deltaMassDiff`, `Dst0_chiProb`, `Dstp_chiProb` as
   `ExtraInfo` on B candidates (raw feature blocks 5-8).
   With the current default `skipTreeFit=True`, only blocks 5-6 remain active
   network inputs by construction; blocks 7-8 are still produced but excluded from
   `HAS_INPUTS`.

If either prerequisite is missing, `ModeSelectorModule` calls `B2FATAL` on the first event
and stops the job immediately. Continuing with absent features would corrupt the network input
with silent zeros.

---

### `ModeSelectorModule.py` -- basf2 module for inference and training-data collection

The module has two modes of operation:

#### Inference mode (default)

Runs the two-stage NN on each event and writes scores.

Both models are loaded through the basf2 MVA Expert framework
(`Belle2::MVA::AbstractInterface` / `ONNXExpert` / `SingleDataset`).
Models must be stored as basf2 MVA weightfiles (ROOT binary format); use
`convert_to_onnx.py` to produce them.

```python
import modeSelector

modeSelector.modeSelector(
    bp_list='B+:feiHadronic',
    b0_list='B0:feiHadronic',
    payload_cat_model='<cat_payload_name>',
    payload_main_model='<main_payload_name>',
    output_variable='BplusScore',
    cat_model_path=None,   # omit or pass None to load from the conditions DB
    main_model_path=None,  # omit or pass None to load from the conditions DB
    path=my_path,
)
```

**Key parameters:**

| Parameter | Default | Description |
|-----------|---------|-------------|
| `bp_list` | required | B+ meson list name |
| `b0_list` | required | B0 meson list name |
| `payload_cat_model` | `'modeSelector_cat_model_v2'` | DB payload name for category model |
| `payload_main_model` | `'modeSelector_main_model_v2'` | DB payload name for main model |
| `output_variable` | `'BplusScore'` | EventExtraInfo name for the main signed score |
| `cat_model_path` | `None` | Path to basf2 MVA weightfile for the category model, as produced by `convert_to_onnx.py` (overrides DB); do not pass a raw `.onnx` file |
| `main_model_path` | `None` | Path to basf2 MVA weightfile for the main model, as produced by `convert_to_onnx.py` (overrides DB); do not pass a raw `.onnx` file |
| `training_mode` | `False` | Switch to training-data collection mode |
| `skip_nn_evaluation` | `False` | Skip NN inference and use placeholder outputs (debug only) |
| `store_fei_calib_weight` | `False` | Compute and store `modeSelector_feiCalibWeight` (based on best FEI candidate); requires `mostcommonBTagPDG` and `mostcommonBTagDeltaP` for truth matching; MC only |
| `addDstarVetoReco` | `True` | Add D* veto reconstruction before the NN; pass `False` if already added separately |

**Outputs written:**

| Variable | Location | Description |
|----------|----------|-------------|
| `{output_variable}` | `EventExtraInfo` | BplusScore = `sign * score`, where `score` is the max predicted-sector candidate probability if a candidate exists, else `max(0, 2 * (sum(predicted-sector signal outputs + bad_tag) - 0.5))` |
| `modeSelector_catB0` | `EventExtraInfo` | Category network prob B0 |
| `modeSelector_catBp` | `EventExtraInfo` | Category network prob B+ |
| `modeSelector_catCont` | `EventExtraInfo` | Category network prob continuum |
| `modeSelector_feiCalibWeight` | `EventExtraInfo` | FEI calibration weight from the best candidate (truth-compatible tag PDG and DeltaP < threshold); `FEI_CALIB_CONT` for continuum; `NaN` when reco conditions are not met; only written when `store_fei_calib_weight=True` |
| `modeSelector_eqSigProb` | `ExtraInfo` on predicted-sector candidates | Mode probability `main_output[input_id]` |
| `modeSelector_rank` | `ExtraInfo` on deduplicated candidates | Sector-local rank: predicted uses `modeSelector_eqSigProb`, else `sigProb` |

`modeSelector_eqSigProb` is set only on candidates in the predicted sector (B+ if
`BplusScore > 0`, B0 if `BplusScore < 0`). `modeSelector_rank` is written on the
module's deduplicated representatives only, using `modeSelector_eqSigProb` in the
predicted sector and `sigProb` in the non-predicted sector. The two sectors are
ranked independently.
If the predicted sector has no candidate in an event, `BplusScore` is computed
with the normalized fallback described above.

The non-predicted sector contains candidates of the opposite B type to what the
category network predicts. These form a crossfeed-enhanced control sample.
To restrict to the predicted sector only, require `BplusScore > 0` for B+ analyses
or `BplusScore < 0` for B0 analyses.

#### Training mode

```python
modeSelector.modeSelector(
    bp_list='B+:feiHadronic',
    b0_list='B0:feiHadronic',
    training_mode=True,
    training_output='modeSelector_training.npz',
    path=my_path,
)
```

Collects features and MC truth per event; writes at `terminate()`:
- `modeSelector_training_features.npz` - sparse CSR feature matrix (shape `N x 1644`)
- `modeSelector_training.npz` - event metadata + MC truth arrays (compressed)

Arrays stored in `modeSelector_training.npz`:

| Array | Shape | dtype | Sentinel | Description |
|-------|-------|-------|----------|-------------|
| `is_cont` | `(N,)` | int8 | - | 1 if continuum event (from best overall candidate) |
| `gen_pdg` | `(N,)` | int16 | -1 | `mostcommonBTagPDG` of best overall candidate; -1 for continuum or missing |
| `bp_is_best` | `(N,)` | int8 | - | 1 if best-sigProb candidate is in B+ sector |
| `best_sigprob` | `(N,)` | float32 | - | sigProb of the best overall candidate |
| `best_bp_sigprob_iid` | `(N,)` | int16 | -1 | input_id of best-sigProb B+ candidate |
| `best_b0_sigprob_iid` | `(N,)` | int16 | -1 | input_id of best-sigProb B0 candidate |
| `bp_tag_is_gen` | `(N,)` | int8 | 0 | best-sigProb B+: 1 if tag PDG is truth-compatible |
| `b0_tag_is_gen` | `(N,)` | int8 | 0 | best-sigProb B0: 1 if tag PDG is truth-compatible |
| `fei_calib_weight` | `(N,)` | float32 | 1.0 | pre-computed event-level FEI calibration weight (same logic as `compute_event_weights`; used for cross-validation in `train.py`) |
| `bp_gen_decay_mode_id` | `(N,)` | int16 | -1 | best-sigProb B+: generated FEI decay mode id (999=rest, -1=missing) |
| `b0_gen_decay_mode_id` | `(N,)` | int16 | -1 | best-sigProb B0: generated FEI decay mode id (999=rest, -1=missing) |
| `bp_gen_fei_calib_weight` | `(N,)` | float32 | 1.0 | best-sigProb B+: stored FEI calibration weight from generated decay |
| `b0_gen_fei_calib_weight` | `(N,)` | float32 | 1.0 | best-sigProb B0: stored FEI calibration weight from generated decay |
| `best_bp_iid` | `(N,)` | int16 | -1 | input_id of best is_target candidate in B+ sector |
| `best_bp_dp` | `(N,)` | float32 | inf | mostcommonBTagDeltaP of that candidate |
| `best_b0_iid` | `(N,)` | int16 | -1 | input_id of best is_target candidate in B0 sector |
| `best_b0_dp` | `(N,)` | float32 | inf | mostcommonBTagDeltaP of that candidate |
| `sig_input_ids_values` | `(K,)` | int16 | - | Packed input_id values for deduplicated candidates with `isSignal == 1` |
| `sig_input_ids_offsets` | `(N+1,)` | int32 | - | Ragged offsets for `sig_input_ids_values` per event |
| `sig_btag_index_values` | `(K,)` | int16 | -1 | Packed `mostcommonBTagIndex` aligned with `sig_input_ids_values` |
| `sig_delta_p_values` | `(K,)` | float32 | inf | Packed `mostcommonBTagDeltaP` aligned with `sig_input_ids_values` |
| `sig_sigprob_values` | `(K,)` | float32 | -1 | Packed `extraInfo(SignalProbability)` aligned with `sig_input_ids_values` |

`best_bp_iid/dp` and `best_b0_iid/dp` store the best is_target candidate per
sector (smallest `mostcommonBTagDeltaP`, truth-compatible tag PDG,
non-continuum). The `delta_p_thresh` cut is not applied at collection time;
it stays in `train.build_mode_labels()`. For packed ragged arrays, event `i` is read
as `values[offsets[i]:offsets[i+1]]`.

---

### `train.py` -- neural network training

Trains the category and main networks from the `.npz` files produced in training mode.
Training inputs are produced by running `analysis/examples/modeSelector/produceTrainingInputs.py`
with `training_mode=True` (see `analysis/examples/modeSelector/README.md` for details).

#### Usage

```bash
# Step 1: train category network (B0 vs B+ vs continuum)
python3 train.py \
    --input modeSelector_training*.npz \
    --network category \
    --use_sparse

# Step 2: train main network (requires trained category network)
python3 train.py \
    --input modeSelector_training*.npz \
    --network main \
    --cat_model networks/net_category.pt \
    --use_sparse
```

`--input` accepts one or more `.npz` paths (space-separated or glob pattern, quoted or unquoted). `_features.npz` files are automatically excluded. Files are loaded in parallel using a thread pool. For each `modeSelector_training_X.npz`, a matching `modeSelector_training_X_features.npz` must exist in the same directory.

Per-event preselection: for each event, the candidate with the highest sigProb is identified (across B+ and B0 lists). The event is kept only if that candidate satisfies `sigProb > 0.001`. 
This matches the skim selection threshold, so no events are removed in practice.

**Main network label assignment and event selection**

The category network is run on the preselected events first. Its prediction
(`charged_cat = cat_output[:, 1] > cat_output[:, 0]`) selects the predicted B sector
per event (B+ or B0). Labels are assigned from `build_mode_labels()` in `train.py`:

1. For each event, select `best_bp_iid/dp` or `best_b0_iid/dp` depending on `charged_cat`.
2. Select isSignal candidates in the predicted sector from packed arrays.
3. If exactly one candidate exists: label = that `input_id`.
4. If multiple candidates exist:
   - if all have the same `mostcommonBTagIndex`: pick the largest `input_id`
   - if `mostcommonBTagIndex` differs: pick smallest `mostcommonBTagDeltaP`,
     tie-break by higher `sigProb`, then by larger `input_id`
5. If no isSignal candidate exists in predicted sector, fallback to `mostcommonBTagDeltaP` based logic:
   - if `best_iid >= 0` and `DeltaP < 0.15` (`delta_p_thresh`): label = `best_iid`
   - else assign background class:
     - 138 (`continuum`): `is_cont == 1`
     - 137 (`cross_deltaC1`): `|gen_pdg| != |PDG of predicted sector|`
     - 136 (`bad_tag`): all other BB events

After label assignment, all preselected events are kept for main-network
training. If the category-predicted sector has no reconstructed candidate,
the event remains in training and uses the fallback label assignment above.

#### Arguments

| Argument     | Default | Description |
|--------------|---------|-------------|
| `--input` | required | One or more `.npz` paths (space-separated or glob); `_features.npz` files excluded automatically |
| `--network` | required | `category` or `main` |
| `--cat_model` | - | Trained category `.pt` (required for `--network main`) |
| `--output` | `networks/` | Directory for saved models |
| `--epochs` | `50` | Training epochs |
| `--fraction` | `1.0` | Uniform BB downsampling fraction |
| `--cont_fraction` | `1.0` | Additional continuum downscale relative to `--fraction` |
| `--disco_lambda` | `0.0` | DisCo penalty coefficient (0 to disable) |
| `--label_smoothing` | `0.0` | Label smoothing for CrossEntropyLoss (0 to disable) |
| `--use_sparse` | flag | Sparse data loading (lower peak memory, slower) |
| `--batch_size` | `None` | Batch size; defaults to 16384 (category) or 32768 (main) |

`--use_sparse` is off by default, but in practice it is required for large training datasets.
Without it, the full feature matrix is converted to a dense array and loaded into RAM at once,
which easily exceeds available memory for training datasets of ~100M events or more.
With `--use_sparse`, rows are converted to dense on-the-fly during batching, which is
significantly slower but feasible in terms of memory.

**Network architecture (`MultiClassNet`):**

Two architectures are selected automatically by `num_labels`:

- `num_labels <= 10`, category network with 3 classes:
  `input -> 128 -> 128(dropout 0.2) -> 64(dropout 0.15) -> 32(dropout 0.1) -> 16 -> N`
- `num_labels > 10`, main network with 139 classes:
  `input -> 256 -> 128(dropout 0.1) -> 128(dropout 0.1) -> 64(dropout 0.05) -> N`

ReLU activations, Xavier initialization (gain=0.5, bias=0.01).

**Training details:**
- Training is performed on calibrated run-dependent MC (MCrd), FEI-skimmed
  (sigProb > 0.001), with FEI calibration weights applied as per-event weights.
- The training dataset already contains only 25% of the available continuum events:
  `produceTrainingInputs.py` uses `--cont-fraction 0.25` by default when collecting
  training inputs. The `--cont_fraction` argument in `train.py` is an additional
  downscale applied on top; in standard usage it is left at 1.0.
- Optimiser: AdamW, `CosineAnnealingLR`, gradient clipping at norm 1.0
- Early stopping: patience 5 epochs on validation loss
- **Per-event FEI calibration weights** are applied to the loss. Derived from
  the overall highest-sigProb candidate:
  - **Reco path** (when `tag_is_gen == 1` and `DeltaP < DELTA_P_THRESH`):
    weight looked up by reconstructed dmID: `input_id // 2` for B+,
    `(input_id - N_BP_MODES*2) // 2` for B0 (see input_id encoding above)
  - **Gen path** (fallback): uses `genDecayModeID`; 999 falls back
    to the sector rest weight
  - Continuum: always `config.FEI_CALIB_CONT` (= 1.0)

#### Training time

The v2 models were trained on ~132.5M events on a GPU node (NAF cluster):

| Network | Epochs | Time/epoch | Total |
|---------|--------|------------|-------|
| Category | 24 (early stopping) | ~20 min | ~8 h |
| Main | 45 (early stopping) | ~28 min | ~21 h |

**Saved checkpoint** (`net_category.pt` / `net_main.pt`):
`epoch`, `model_state_dict`, `optimizer_state_dict`, `val_loss`, `train_loss`, `history`, `has_inputs`, `config`.

`history` is a dict with keys `train_loss`, `val_loss`, `disco_loss`, `lr` (one entry per epoch),
used by `analysis/examples/modeSelector/plot_training.py`.

---

### `convert_to_onnx.py` -- PyTorch to ONNX conversion

Converts trained `.pt` checkpoints to ONNX for use in basf2 (via `onnxruntime`).
It can also copy the exported ONNX files into `localdb/database.txt` as
conditions payloads.

```bash
python3 convert_to_onnx.py --input-dir networks/ --output-dir onnx/ --add-payloads \
        --cat-payload-name=<cat_payload_name> --main-payload-name=<main_payload_name>
```

- Reads `net_category.pt` and `net_main.pt` from `--input-dir`; writes
  `modeSelector_cat.onnx`, `modeSelector_cat.root`, `modeSelector_main.onnx`, and
  `modeSelector_main.root` to `--output-dir`. The `.root` files are the basf2 MVA
  weightfiles passed via `cat_model_path` / `main_model_path`.
- Packages each `.onnx` into a basf2 MVA weightfile using
  `basf2_mva_util.create_onnx_mva_weightfile()`. `ModeSelectorModule` fills
  the feature vector manually; the weightfile contains only dummy variable names.
- With `--add-payloads`, creates `localdb/database.txt` with the payload names specified.
- Wraps the network with `nn.Softmax` before export (ONNX model outputs probabilities).
- Input size and output classes are derived automatically from the checkpoint.
- `--cat-payload-name` / `--main-payload-name` set the payload names written
  into `localdb/database.txt`. These must match the `payload_cat_model` /
  `payload_main_model` arguments passed to `modeSelector.modeSelector()`, currently defaults to `'modeSelector_cat_model_v2'` and `'modeSelector_main_model_v2'`.
- `--first-exp`, `--first-run`, `--final-exp`, `--final-run` control the
  interval of validity for local payload entries.

For a practical end-to-end workflow using the example steering files, see
`analysis/examples/modeSelector/README.md`.
