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

The package is split by audience: `__init__.py`, `ModeSelectorModule.py`, `dstarVeto.py`
and `generatedDecayWeights.py` are what an analysis uses at run time and need only basf2
and numpy. The scripts under `training/` produce and export the models and pull in torch,
uproot and scipy; nothing imports them at inference. `config.py` is shared by both and
stays at the top level.

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
| `DEFAULT_CAT_PAYLOAD` | `modeSelector_cat_model_perf_c1` | Default DB payload name for the category model, derived from `MODEL_CONTRACT_VERSION` |
| `DEFAULT_MAIN_PAYLOAD` | `modeSelector_main_model_perf_c1` | Default DB payload name for the main model, derived from `MODEL_CONTRACT_VERSION` |
| `payload_names(version)` | function | (category, main) payload names for any contract version |
| `MODEL_CONTRACT_VERSION` | 1 | Contract version between the code and its payloads, bumped by hand |
| `SUPPORTED_CONTRACT_VERSIONS` | {1} | Contract versions the module can run (current plus older ones with a kept code path) |
| `FEATURE_VAR_PREFIX` | `msfeat_` | Prefix for the raw-feature entries in the weightfile variable list |
| `MAIN_EXTRA_VARS` | 4 names | The main network's extra inputs (3 category outputs + charged flag) |

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

**`EVENT_FEATURES`** - the 8 event-shape names read from `EventShapeContainer`
(indices 1632-1639). Together with four scalars the module computes itself
(`ncandidates`, `max_input_id`, `scnd_max_input_id`, `__experiment__`) these make up
the 12 event-level values appended after the flat feature matrix (indices 1632-1643):

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
removed, blocks 7-8 and 10 excluded), currently 976 indices. The list depends on
which input_ids the training sample actually populates, so it has to be
regenerated whenever the training dataset changes: `train.py` recomputes it from
the data, and on mismatch it warns, writes `has_inputs_recomputed.txt` and
proceeds with the recomputed list for that run. Paste that file's contents into
`config.py` afterwards so the next training starts from the right selection.

Payloads carry their own feature selection (see `convert_to_onnx.py` below), so at
inference this list is only a fallback for pre-convention weightfiles and the
`skip_nn_evaluation` path, and `train.py` uses the recomputed selection either way.
A retraining can therefore change the selection without a software release, and a
stale `config.HAS_INPUTS` cannot silently mis-select columns for a payload. It can
be dropped once no pre-convention payloads remain.

**`MODEL_CONTRACT_VERSION`** - The version of the agreement between the ModeSelector
code and a payload. Bump it whenever the code changes anything affecting what a
payload receives or how its outputs are interpreted: the feature array, the
input_id encoding, which candidate represents an input_id slot, the preselection,
or how the outputs become scores. It is not a software version, so unrelated
changes elsewhere must not bump it.

**`FEI_CALIB_*`** - Per-decay-mode FEI calibration weights for two sigProb
working points (0.001, 0.01).
The calibration corresponds to [release 8, run1 + run2](https://gitlab.desy.de/belle2/performance/correction-tables/-/tree/20260126/MC16/FEI/hadronic?ref_type=tags).
 `config.DEFAULT_FEI_SIGPROB_THRESHOLD` selects which set is used. 
 `FEI_CALIB_CONT` is the weight for continuum events (default `1.0`).

### conditions database payloads

When `cat_model_path` and `main_model_path` are omitted, models are loaded from
the conditions database via payloads.

In this section, **the module** means the ModeSelector code that consumes the payloads
and implements one contract version, and a **release** means the light release shipping
it, which is what users switch.

**Payload naming.** The payload names are derived from the contract version the
module implements: `modeSelector_cat_model_perf_c1` and
`modeSelector_main_model_perf_c1` (`config.DEFAULT_CAT_PAYLOAD` /
`config.DEFAULT_MAIN_PAYLOAD`). They carry no training or campaign version: a new
training for the same contract is uploaded as a new revision under the same name and
selected by the performance globaltag. The `_perf` part marks the payloads as coming
from a performance globaltag rather than from the analysis globaltag.

The contract suffix `_cN` lets a single performance globaltag serve several releases.
It can hold `_c1` and `_c2` payloads side by side, and the module requests the payloads
for the contract version it implements, so the module in an older release keeps working
with a newer performance globaltag, or a newer version of it, as long as that globaltag
still provides payloads for its contract version.
It then gets the newest training available for that contract.

This follows the conditions database convention that campaign-dependent payloads
belong in campaign-dependent globaltags rather than being distinguished by a tag in
the payload name. The contract suffix is not such a tag: it identifies which inputs a
model consumes and how its outputs are read.

When loading from the conditions database, the module also looks for payloads of up
to ten newer contract versions (`config.payload_names()`). If one exists it emits a
warning that a newer contract, shipped with a newer release, uses a different model,
and refers to the ModeSelector
performance recommendations for what changed and whether switching is worthwhile.
A missing payload only costs a metadata lookup; the newer file is fetched only if it
exists. Passing local weightfiles skips this check.

Because the name no longer identifies the training, two safeguards are built into
the payload itself:

- the weightfile records the **feature selection** its model was trained on, so
  the payload does not depend on `config.HAS_INPUTS` being in sync
- the weightfile records the **contract version** (`Weightfile::addContractVersion`,
  read back with `hasContractVersion` / `getContractVersion`), checked against the
  supported contract versions when it is loaded (see below)

A further check compares the model's output class count (`m_nClasses`) against what
the module interprets: 3 for the category network, `N_INPUT_IDS + 3` for the main
network. The outputs are read positionally, so a model with a different number of
classes would be misread rather than rejected; a mismatch is fatal.

Another check compares the weightfile identifiers of the category and main models,
which hold the training name. The main network takes the category outputs as inputs, so
a pair from different trainings would run but give wrong results, and it would pass every
other check since both share the contract version. A mismatch is fatal.

All of these are written by `convert_to_onnx.py` and logged at the start of a job
together with the training name:

```
[INFO] ModeSelector: category model training 'mc16rd_v3' (contract v1)
[INFO] ModeSelector: Using 976 selected features from the weightfile
```

**Supported contract versions.** `config.SUPPORTED_CONTRACT_VERSIONS` lists the
contract versions the module can run. It always contains `MODEL_CONTRACT_VERSION`;
an older version stays in it only while the module still provides that contract's
code path, meaning everything that differs between contract versions (feature
construction, expected output classes, output interpretation) branches on
`ModeSelectorModule.contract_version`. When the models are loaded:

- a payload for the current contract version runs normally;
- a payload for an older supported version runs with that contract's behaviour, and
  the module warns that a newer contract exists, referring to the ModeSelector
  performance recommendations for what changed and whether switching is worthwhile;
- a payload for a newer version, or an older one that is no longer supported, is fatal;
- the category and main models must share one contract version.

`config.contract_consistency_error()` requires that the current version is supported
and that no newer version is listed; it is checked by `convert_to_onnx.py` before
exporting and at the start of a job. At the moment only contract version 1 exists, so there is no older code
path yet.

An older contract is loaded by naming its payloads explicitly, for example
`payload_cat_model=config.payload_names(1)[0]`. Any explicitly given payload name emits
a warning that the default payloads are not used; names superseded by local
weightfiles are not reported.

Weightfiles exported before this (with placeholder variable names) still load:
the module warns, treats them as contract version 1 (fatal if that version is no longer
supported) and falls back to `config.HAS_INPUTS`.

**Intervals of validity.** ModeSelector currently uses one training for all datasets (the
experiment number is a network input), so its payloads have a single open-ended iov
(`0,0,-1,-1`, the `convert_to_onnx.py` default). The module still looks up the payloads
in `beginRun()` and, whenever the checksum of either one changes, reloads both and
repeats all the checks above (contract version, same training, output classes, feature
selection). Dataset-dependent trainings can therefore be uploaded as further revisions
under the same name with different iovs, without changing the module. Local weightfiles
(`cat_model_path` / `main_model_path`) are loaded once in `initialize()`.

To create a local payload database use:

```bash
python3 training/convert_to_onnx.py --input-dir networks/ --output-dir onnx/ --add-payloads
```

This creates `localdb/database.txt`. To test, prepend it in a steering file with:

```python
basf2.conditions.prepend_testing_payloads('localdb/database.txt')
```

Once validated locally, the payloads are uploaded to a globaltag with
`b2conditionsdb` (`b2conditionsdb tag create DEV <tag> "<description>"` once,
then `b2conditionsdb upload <tag> localdb/database.txt`). A globaltag must
be moved out of the `OPEN` state (for example to `TESTING`) before basf2 accepts
it for processing. Payloads are immutable, so replacing a model means deleting
the old iovs (`b2conditionsdb iovs delete <tag>`, with the tag reopened to
`OPEN`) and uploading again, which creates a new revision.

The payloads are not in the official analysis globaltag yet, so
`getAnalysisGlobaltag()` alone will not resolve them. Until they are, prepend the
globaltag holding them in addition to the analysis globaltag:

```python
basf2.conditions.prepend_globaltag('<tag holding the modeSelector payloads>')
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

The pi0s are reconstructed from photons passing the cuts of the former 50% efficiency
May 2020 pi0 selection, with the beam background and fake photon MVAs evaluated using
the `MC16rd` weights. Photon pairs with `0.105 < InvM < 0.150` are fitted with a mass
constraint (`kFit`), and the pi0s used in the veto must satisfy `useCMSFrame(p) < 0.5`
and, for both photons, `beamBackgroundSuppression > 0.5` and `fakePhotonSuppression > 0.1`.

The ROE loop runs on private `Xsd` particles that each wrap one B candidate, not on
the B candidates themselves. A particle can only have one ROE, so building it on the B
candidates would make the veto reuse an ROE the user built on the same list (e.g. for
continuum suppression), and a user ROE built after the veto would be skipped for some
candidates. The wrapper's ROE contains the same tracks and clusters as an ROE of the B
candidate, and the results are copied to the B candidates with
`variablesToDaughterExtraInfo`. User ROEs on the input lists are therefore independent
of the veto, in either order.

**ExtraInfo fields added to B candidates:**

| Field | Meaning |
|-------|---------|
| `Dstp_deltaMassDiff` | `massDifference(0) − trueMassDiff` for D*+ |
| `Dstp_chiProb` | Vertex fit χ² probability for D*+ (NaN if `skipTreeFit` and no pre-existing D*) |
| `Dst0_deltaMassDiff` | Same for D*0 |
| `Dst0_chiProb` | Same for D\*0 (NaN if `skipTreeFit` and no pre-existing D\*) |

**Key parameters:**
- `skipTreeFit=True` (default): skip `treeFit` for reconstructed D* candidates, a
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
| `payload_cat_model` | `None` | DB payload name for category model; `None` derives it from the contract version (normal use). An explicit name always warns; it can also select a payload for an older supported contract version |
| `payload_main_model` | `None` | Same for the main model |
| `output_variable` | `'BplusScore'` | EventExtraInfo name for the main signed score |
| `cat_model_path` | `None` | Path to basf2 MVA weightfile for the category model, as produced by `convert_to_onnx.py` (overrides DB); do not pass a raw `.onnx` file |
| `main_model_path` | `None` | Path to basf2 MVA weightfile for the main model, as produced by `convert_to_onnx.py` (overrides DB); do not pass a raw `.onnx` file |
| `training_mode` | `False` | Switch to training-data collection mode |
| `skip_nn_evaluation` | `False` | Skip NN inference and use placeholder outputs (debug only) |
| `store_fei_calib_weight` | `False` | Compute and store `modeSelector_feiCalibWeight` (based on best FEI candidate); requires `mostcommonBTagPDG` and `mostcommonBTagDeltaP` for truth matching; MC only |
| `addDstarVetoReco` | `True` | Add D* veto reconstruction before the NN; pass `False` if already added separately |
| `debug` | `False` | Print the feature vector and network outputs for the first few events |
| `debug_max_events` | `10` | How many events `debug` prints |

**Outputs written:**

| Variable | Location | Description |
|----------|----------|-------------|
| `{output_variable}` | `EventExtraInfo` | BplusScore = `sign * score`, where `score` is the max predicted-sector candidate probability if a candidate exists, else `max(0, 2 * (sum(predicted-sector signal outputs + bad_tag) - 0.5))` |
| `modeSelector_catB0` | `EventExtraInfo` | Category network prob B0 |
| `modeSelector_catBp` | `EventExtraInfo` | Category network prob B+ |
| `modeSelector_catCont` | `EventExtraInfo` | Category network prob continuum |
| `modeSelector_feiCalibWeight` | `EventExtraInfo` | FEI calibration weight from the reco path of the overall highest-sigProb candidate in the event, chosen by comparing the best B+ and best B0 candidates; stored only when that candidate has truth-compatible tag PDG and DeltaP < threshold; `FEI_CALIB_CONT` for continuum; `NaN` when reco conditions are not met; only written when `store_fei_calib_weight=True` |
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
    path=my_path,
)
```

Skips NN inference and instead exposes per-event features and MC truth via
`EventExtraInfo`/`ExtraInfo`, for a `variablesToNtuple` call in the steering
script to dump to a ROOT file. No file I/O happens inside the module itself, so
the output is written by basf2's own output modules and is downloadable when
this is run via `gbasf2` (see `analysis/examples/modeSelector/produceTrainingInputs.py`
and its README for the full steering script and a grid submission walkthrough).

Values written per event:

| ExtraInfo name | Location | Description |
|-----------------|----------|-------------|
| `modeSelector_feat_0000` .. `modeSelector_feat_1643` | `EventExtraInfo` | The 1644 raw features (flattened `12 blocks x 136 input_ids` + 12 event-level scalars), same layout as the inference-mode feature array |
| `modeSelector_tr_<name>` | `EventExtraInfo` | 17 per-event truth/label scalars: `is_cont`, `gen_pdg`, `bp_gen_decay_mode_id`, `b0_gen_decay_mode_id`, `bp_gen_fei_calib_weight`, `b0_gen_fei_calib_weight`, `bp_is_best`, `best_sigprob`, `best_bp_sigprob_iid`, `best_b0_sigprob_iid`, `bp_tag_is_gen`, `b0_tag_is_gen`, `fei_calib_weight`, `best_bp_iid`, `best_bp_dp`, `best_b0_iid`, `best_b0_dp` (same semantics as the legacy npz arrays of the same names) |
| `modeSelector_trainSigInputId` | `ExtraInfo` on the deduplicated best-per-`input_id` candidate | Set (to that candidate's `input_id`) only on candidates with `isSignal == 1`; unset (NaN when read back) on all other candidates |

`modeSelector_trainSigInputId` is set directly on the underlying `Particle`, so
it is visible through any `ParticleList` that references the same candidate
(e.g. a merged B+/B0 list built for a per-candidate ntuple dump).

---

### `training/train.py` -- neural network training

Trains the category and main networks from the ROOT files produced in training mode.
Training inputs are produced by running `analysis/examples/modeSelector/produceTrainingInputs.py`
with `training_mode=True` (see `analysis/examples/modeSelector/README.md` for details,
including grid submission).

#### Usage

```bash
# Step 1: train category network (B0 vs B+ vs continuum)
python3 training/train.py \
    --input modeSelector_training*.root \
    --network category \
    --use_sparse

# Step 2: train main network (requires trained category network)
python3 training/train.py \
    --input modeSelector_training*.root \
    --network main \
    --cat_model networks/net_category.pt \
    --use_sparse
```

`--input` accepts one or more `.root` paths, or `.npz` shards written by
`convert_training_inputs.py` (space-separated or glob pattern, quoted or unquoted; the two
may be mixed). Patterns are expanded with `glob.glob(..., recursive=True)`, so `**` descends
into subdirectories (e.g. `<project>/**/*.root` for a downloaded gbasf2 project, whose
outputs are nested one level below each dataset's `sub00`). Files are loaded in parallel
using a process pool (`--num_workers`). Each ROOT file must contain the `events` and
`sig_candidates` trees written by `produceTrainingInputs.py`; `load_and_sample_data()` joins
`sig_candidates` rows back to their event via the `(__experiment__, __run__, __event__)`
triplet written to both trees, then reassembles the same packed ragged `sig_*` arrays the
training pipeline used to read directly from the legacy npz format. A grid submission (see
the examples README) produces many small `.root` files (one per gbasf2 job); glob all of
them as `--input`.

**For anything beyond a few hundred ROOT files, convert first** with
`convert_training_inputs.py` (see below) and train on the resulting `.npz` shards. Reading
the raw ROOT inputs costs ~11 s per file and is paid again on every `train.py` invocation,
i.e. twice per full training (category, then main).

**Unreadable inputs**: files that fail to open/read (corrupted or incomplete grid downloads)
or are missing required trees/branches are skipped with a printed warning rather than
aborting the run. The check happens in the loader worker on the already-open file, so a bad
input costs one open rather than a separate serial pass over every file. Training only
aborts if *all* input files fail to load.

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
| `--input` | required | One or more `.root` or `.npz` paths (space-separated or glob) |
| `--network` | required | `category` or `main` |
| `--cat_model` | - | Trained category `.pt` (required for `--network main`) |
| `--output` | `networks/` | Directory for saved models |
| `--epochs` | `50` | Training epochs |
| `--fraction` | `1.0` | Uniform BB downsampling fraction |
| `--cont_fraction` | `1.0` | Additional continuum downscale relative to `--fraction` |
| `--disco_lambda` | `0.0` | DisCo penalty coefficient (0 to disable) |
| `--label_smoothing` | `0.0` | Label smoothing for CrossEntropyLoss (0 to disable) |
| `--use_sparse` | flag | Sparse data loading (lower peak memory, slower) |
| `--lr` | `5e-4` | Initial learning rate |
| `--lr_schedule` | `cosine` | `cosine` or `constant` |
| `--eta_min` | `1e-5` | Final learning rate for the cosine schedule |
| `--weight_decay` | `2e-4` | AdamW weight decay |
| `--val_split` | `0.3` | Fraction of events held out for validation |
| `--seed` | `42` | Random seed |
| `--batch_size` | `None` | Batch size; defaults to 16384 (category) or 32768 (main) |
| `--num_workers` | `None` | Worker processes for input loading and the DataLoader; default `min(8, max(1, cpu_count//2))` |

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

The current models were trained on 91,658,960 events on a GPU node (NAF cluster):

| Network | Epochs | Total time | Best epoch | Best val loss | Parameters | Input size |
|---------|--------|------------|------------|----------------|------------|------------|
| Category | 31 (early stopping) | 337 m 21 s (~5.6 h) | 26 | 0.790033 | 152,483 | 976 |
| Main | 46 (early stopping) | 516 m 59 s (~8.6 h) | 41 | 0.952339 | 317,835 | 980 |

`net_main.pt`'s input size (980) is `has_inputs` (976) + `cat_output` (3) +
`charged_cat` (1), appended by `train.py` before training the main network.
Overall validation accuracy was 0.6495 (category) and 0.6037 (main); see the
class-level breakdown in the training job log for the confusable classes
(`cross_deltaC1` in particular).

Those numbers assume a CUDA-capable `torch`. Check that training really runs on the GPU
before submitting a long job -- `train.py` prints the device it selected at startup, and

```bash
python3 -c "import torch; print(torch.__version__, torch.cuda.is_available())"
```

should report `True`. The torch shipped with the basf2 externals is CPU-only, so use a
separate environment with a CUDA build.

**Training does not need basf2 at all.** It uses only torch/numpy/scipy/uproot plus the
constants in `config.py`, which imports nothing; `train.py` and `convert_training_inputs.py`
fall back to loading `config.py` directly by path when the `modeSelector` package (whose
`__init__` pulls in basf2 and ROOT) is unavailable. So the supported setup is a standalone
venv on a data disk, e.g.:

```bash
/usr/bin/python3.9 -m venv /data/dust/user/<user>/modeSelector_venv
/data/dust/user/<user>/modeSelector_venv/bin/pip install \
    --index-url https://download.pytorch.org/whl/cu124 torch==2.5.0
/data/dust/user/<user>/modeSelector_venv/bin/pip install numpy scipy uproot tqdm
```

Put it on a data disk, not in AFS home -- it is ~6 GB. Run the scripts with the venv's
interpreter and no `b2setup`. If the batch job uses `getenv = true`, `unset PYTHONPATH
PYTHONHOME LD_LIBRARY_PATH` in the job script first: an inherited basf2 `PYTHONPATH` makes
`pybasf2` visible to the venv interpreter, where it fails to initialise with `SystemError`
(the import fallbacks catch this, but a clean environment is better).

Loading the inputs is a separate cost from training and dominates when reading raw ROOT
files -- see `convert_training_inputs.py` below.

**Saved checkpoint** (`net_category.pt` / `net_main.pt`):
`epoch`, `model_state_dict`, `optimizer_state_dict`, `val_loss`, `train_loss`, `history`, `has_inputs`, `config`.

`history` is a dict with keys `train_loss`, `val_loss`, `disco_loss`, `lr` (one entry per epoch),
used by `analysis/examples/modeSelector/plot_training.py`.

---

### `training/convert_training_inputs.py` -- ROOT to .npz shard conversion

Converts `produceTrainingInputs.py` ROOT outputs into compact `.npz` shards that `train.py`
reads directly. Run this once per training-input production; then point `train.py --input`
at the shards.

**Why**: the `events` tree stores one branch per feature (1644 `ms_feat_*`, 1667 total),
split into ~50k small baskets per file. Reading one file costs ~11 s regardless of its size
(~18 MB), so the full v7 sample (14364 files, 320 GB) takes ~44 core-hours to read -- paid
again on every `train.py` invocation. The features are only ~1.6% dense, so the same events
stored as CSR come to ~260 bytes/event: the full sample becomes ~32 GB of `.npz` that loads
in minutes.

No selection is applied during conversion. Shards hold exactly what the ROOT loader returns,
so `--fraction`, `--cont_fraction` and the sigProb preselection remain train-time knobs and
changing them does not require reconverting.

```bash
# one shard per gbasf2 dataset directory (parallelise over directories with HTCondor)
python3 training/convert_training_inputs.py \
    --input '<project>/ModeSelector_v7_ccbar_1/**/*.root' \
    --output /path/to/converted \
    --name ModeSelector_v7_ccbar_1

# or everything at once, split into shards of 200 input files
python3 training/convert_training_inputs.py \
    --input '<project>/**/*.root' \
    --output /path/to/converted \
    --name ModeSelector_v7 --files_per_shard 200

# then train
python3 training/train.py --input '/path/to/converted/*.npz' --network category --use_sparse
```

Quote glob patterns containing `**` so that Python expands them recursively rather than the
shell. Unreadable inputs are skipped with a warning, as in `train.py`.

| Argument | Default | Description |
|----------|---------|-------------|
| `--input` | required | One or more ROOT paths (space-separated or glob) |
| `--output` | required | Output directory for the `.npz` shard(s) |
| `--name` | `shard` | Base name for the output shard(s) |
| `--files_per_shard` | `None` | Split inputs into shards of this many files (default: one shard) |
| `--num_workers` | `None` | Loader processes; default `min(16, cpu_count())` |
| `--overwrite` | flag | Rewrite existing shards (default: skip them, so a partial run resumes) |

**Shard format** (`NPZ_SHARD_VERSION = 1`): the CSR feature matrix as `feat_data` /
`feat_indices` / `feat_indptr` / `feat_shape`, the 17 per-event truth arrays under their
`EVENT_TRUTH_FIELDS` names, and the packed ragged `sig_*` arrays with
`sig_input_ids_offsets` rebased across the merged files. Written uncompressed: the data is
already small and load speed matters more than size. `train.py` refuses shards whose
`format_version` does not match and tells you to reconvert.

---

### `training/convert_to_onnx.py` -- PyTorch to ONNX conversion

Converts trained `.pt` checkpoints to ONNX for use in basf2 (via `onnxruntime`).
It can also copy the exported ONNX files into `localdb/database.txt` as
conditions payloads.

```bash
python3 training/convert_to_onnx.py --input-dir networks/ --output-dir onnx/ --add-payloads \
        --cat-payload-name=<cat_payload_name> --main-payload-name=<main_payload_name>
```

- Reads `net_category.pt` and `net_main.pt` from `--input-dir`; writes
  `modeSelector_cat.onnx`, `modeSelector_cat.root`, `modeSelector_main.onnx`, and
  `modeSelector_main.root` to `--output-dir`. The `.root` files are the basf2 MVA
  weightfiles passed via `cat_model_path` / `main_model_path`.
- Packages each `.onnx` into a basf2 MVA weightfile using
  `basf2_mva_util.create_onnx_mva_weightfile()`. `ModeSelectorModule` fills the
  feature vector manually, so the variable names are not resolved through the
  VariableManager and are used to carry metadata instead: each selected raw
  feature index is stored as `msfeat_<index>` (taken from the checkpoint's own
  `has_inputs`, not from `config.HAS_INPUTS`), followed for the main network by
  the four names in `config.MAIN_EXTRA_VARS`.
- Adds the contract version (`config.MODEL_CONTRACT_VERSION`) as an extra weightfile
  element, and stores the training name from `--identifier` as the weightfile
  identifier.
- Refuses to export if the two checkpoints were trained on different feature
  selections, since that means they come from different trainings.
- Before converting anything, refuses to run if the contract configuration is
  inconsistent (`config.contract_consistency_error()`: the current version is missing
  from `SUPPORTED_CONTRACT_VERSIONS` or a newer version is listed), or if
  `--add-payloads` is given and
  `localdb/database.txt` already exists, since adding payloads appends to it and would
  leave stale entries with overlapping iovs.
- With `--add-payloads`, creates `localdb/database.txt` with the payload names specified.
- Wraps the network with `nn.Softmax` before export (ONNX model outputs probabilities).
- Input size and output classes are derived automatically from the checkpoint.
- Validates each export by running a fixed-seed batch of 4 through both the
  PyTorch model and `onnxruntime`, comparing probabilities with
  `rtol=1e-4, atol=1e-6` and requiring the predicted class to be identical.
  The tolerances are float32-appropriate: the two backends order their matmul
  accumulations differently, which shifts individual probabilities of the
  139-class main network by a few 1e-6. `np.allclose` defaults (`atol=1e-8`)
  are meant for float64 and reject roughly 8% of runs at random.
- `--cat-payload-name` / `--main-payload-name` set the payload names written
  into `localdb/database.txt`. When omitted they are derived from the contract
  version (`config.DEFAULT_CAT_PAYLOAD` / `config.DEFAULT_MAIN_PAYLOAD`), which are
  the names the module requests for this contract version. Exporting under any other
  name prints a warning, since the module will not request it.
- `--identifier` names the training (for example `mc16rd_v3`); it is stored as the
  weightfile identifier and logged at inference time. Defaults to `unspecified`, so
  set it for anything that gets uploaded.
- `--first-exp`, `--first-run`, `--final-exp`, `--final-run` control the
  interval of validity for local payload entries. Keep the open-ended default: the module
  does not reload payloads when the iov changes (see "Intervals of validity" above).

For a practical end-to-end workflow using the example steering files, see
`analysis/examples/modeSelector/README.md`.
