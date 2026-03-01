#!/usr/bin/env python3
##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
ModeSelector Module for event-level B meson classification.

This module evaluates a two-stage neural network on FEI B meson candidates
to provide an improved signal probability score (BplusScore) that considers
information from all candidates in the event.

The module:
1. Collects features from all B candidates in the event
2. Runs a category network to classify as B0/B+/continuum
3. Runs a main network using category output to compute final scores
4. Stores results as ExtraInfo on the best candidate
"""

import basf2 as b2
import numpy as np
from modeSelector import config
from ROOT import Belle2
from variables import variables as vm


class ModeSelectorModule(b2.Module):
    """
    Event-level B meson classifier using neural networks.

    This module processes FEI B meson candidates and computes an improved
    signal probability score that leverages information from multiple
    candidates in the event.

    Args:
        particle_lists (list): List of B meson particle list names
        cat_model_path (str): Path to category network ONNX model
        main_model_path (str): Path to main network ONNX model
        output_variable (str): Name of ExtraInfo variable for output score
        store_event_info (bool): Whether to store event-level info
    """

    def __init__(
        self,
        particle_lists,
        cat_model_path=None,
        main_model_path=None,
        output_variable='BplusScore',
        payload_cat_model='ModeSelector_cat_model',
        payload_main_model='ModeSelector_main_model',
        training_mode=False,
        training_output='modeSelector_training.npz',
        debug=False,
        debug_max_events=10,
    ):
        super().__init__()
        #: Input particle lists
        self.particle_lists = particle_lists if isinstance(particle_lists, list) else [particle_lists]
        #: Path to category model
        self.cat_model_path = cat_model_path
        #: Path to main model
        self.main_model_path = main_model_path
        #: Output variable name
        self.output_variable = output_variable
        #: Payload name for category model
        self.payload_cat_model = payload_cat_model
        #: Payload name for main model
        self.payload_main_model = payload_main_model
        #: Training mode (save features + MC truth, skip NN inference)
        self.training_mode = training_mode
        #: Output file for training mode
        self.training_output = training_output
        #: Training data storage
        self.training_data = []
        #: Debug mode
        self.debug = debug
        #: Max events to debug
        self.debug_max_events = debug_max_events
        #: Event counter for debug
        self.event_count = 0
        #: Store debug features for comparison
        self.debug_features = []
        #: Flag to check event shape prerequisite once
        self._event_shape_checked = False
        #: Flag to check D* veto prerequisite once
        self._dstar_veto_checked = False

        # Feature configuration (from modeSelector.config)
        #: Number of input_id slots (B+ sector + B0 sector, each split by particle/antiparticle)
        self.n_input_ids = config.N_INPUT_IDS
        #: Event-level feature names
        self.event_features = config.EVENT_FEATURES

    def initialize(self):
        """Called at the beginning of processing."""
        # Build feature index mapping (needed for both inference and training)
        self._build_feature_indices()

        if self.training_mode:
            b2.B2INFO("ModeSelector: Running in TRAINING mode (saving features, no NN inference)")
            self.has_inputs = None
            return

        import onnxruntime as ort

        self.has_inputs = list(config.HAS_INPUTS)
        b2.B2INFO(f"ModeSelector: Using config.HAS_INPUTS ({len(self.has_inputs)} features kept)")

        # Load models from files or database
        if self.cat_model_path:
            self.cat_session = ort.InferenceSession(self.cat_model_path)
        else:
            db_accessor = Belle2.DBAccessorBase(
                Belle2.DBStoreEntry.c_RawFile, self.payload_cat_model, True
            )
            self.cat_session = ort.InferenceSession(db_accessor.getFilename())

        if self.main_model_path:
            self.main_session = ort.InferenceSession(self.main_model_path)
        else:
            db_accessor = Belle2.DBAccessorBase(
                Belle2.DBStoreEntry.c_RawFile, self.payload_main_model, True
            )
            self.main_session = ort.InferenceSession(db_accessor.getFilename())

        #: Category model input name
        self.cat_input_name = self.cat_session.get_inputs()[0].name
        #: Main model input name
        self.main_input_name = self.main_session.get_inputs()[0].name

        # Get input sizes from models
        self.cat_input_size = self.cat_session.get_inputs()[0].shape[1]
        self.main_input_size = self.main_session.get_inputs()[0].shape[1]

        b2.B2INFO(f"ModeSelector: Loaded category model (input size: {self.cat_input_size})")
        b2.B2INFO(f"ModeSelector: Loaded main model (input size: {self.main_input_size})")
        if self.has_inputs:
            b2.B2INFO(f"ModeSelector: Using {len(self.has_inputs)} selected features")

    def _build_feature_indices(self):
        """
        Build the feature index mapping from config.

        The training used sparse matrices with specific non-zero columns.
        We need to match that structure.
        """
        #: Feature block names and transformations (from config)
        self.feature_blocks = [(name, transform) for name, _, transform in config.FEATURE_BLOCKS]

        # Register basf2 aliases where alias name differs from variable string
        for name, var, _ in config.FEATURE_BLOCKS:
            if name != var:
                vm.addAlias(name, var)

        #: Cut range for D* delta mass difference
        self.deltaM_cut = config.DELTA_M_CUT

    def _get_input_id(self, particle):
        """
        Compute input_id from decay mode ID, B type, and particle/antiparticle sign.

        Encoding:
          B+ sector (input_ids 0 to 2*N_BP_MODES-1):
            anti-B+ (PDG=-521): dmID * 2 + 0
            B+      (PDG=+521): dmID * 2 + 1
          B0 sector (input_ids 2*N_BP_MODES to N_INPUT_IDS-1):
            anti-B0 (PDG=-511): N_BP_MODES*2 + dmID * 2 + 0
            B0      (PDG=+511): N_BP_MODES*2 + dmID * 2 + 1
        """
        dm_id = int(vm.evaluate('extraInfo(decayModeID)', particle))
        pdg = int(vm.evaluate('PDG', particle))
        is_particle = 1 if pdg > 0 else 0
        offset = 0 if abs(pdg) == 521 else config.N_BP_MODES * 2
        return offset + dm_id * 2 + is_particle

    def _extract_particle_features(self, particle):
        """
        Extract features from a single particle.

        Returns:
            tuple: (input_id, feature_dict)
        """
        input_id = self._get_input_id(particle)

        features = {}
        for name, transform in self.feature_blocks:
            val = vm.evaluate(name, particle)
            if np.isnan(val):
                val = None
            features[name] = val

        # Apply D* deltaMassDiff cuts
        for key in ['Dst0_deltaMassDiff', 'Dstp_deltaMassDiff']:
            val = features[key]
            if val is not None:
                if val < self.deltaM_cut[0] or val > self.deltaM_cut[1]:
                    features[key] = None

        return input_id, features

    def _build_feature_array(self, candidates_data, event_features):
        """
        Build the feature array for neural network input.

        Args:
            candidates_data: List of (input_id, features) tuples
            event_features: Dict of event-level features

        Returns:
            numpy.ndarray: Feature array ready for NN input
        """
        # Initialize feature matrix (n_feature_blocks * n_input_ids)
        n_blocks = len(self.feature_blocks)
        feature_matrix = np.zeros((n_blocks, self.n_input_ids), dtype=np.float32)

        # Deduplicate by input_id: keep candidate with highest sigProb
        best_by_input_id = {}
        for input_id, features in candidates_data:
            if input_id < 0 or input_id >= self.n_input_ids:
                continue
            sig_prob = features.get('sigProb')
            if sig_prob is None:
                sig_prob = -1
            if input_id not in best_by_input_id or sig_prob > best_by_input_id[input_id][1]:
                best_by_input_id[input_id] = (features, sig_prob)

        # Fill in per-candidate features using deduplicated candidates
        for input_id, (features, _) in best_by_input_id.items():
            for block_idx, (feat_name, transform) in enumerate(self.feature_blocks):
                val = features.get(feat_name)
                if val is not None:
                    feature_matrix[block_idx, input_id] = transform(val)

        # Flatten to 1D (matches training format)
        flat_features = feature_matrix.flatten()

        # Add event-level features
        event_feat_values = [
            event_features.get(name, 0.0) for name in self.event_features
        ]

        # Add ncandidates / 10 (all candidates before deduplication)
        n_candidates = len(candidates_data)
        event_feat_values.append(n_candidates / 10.0)

        # Find best candidate (highest sigProb) using deduped data
        if best_by_input_id:
            # Best candidate overall (highest sigProb among unique input_ids)
            max_input_id = max(best_by_input_id.keys(), key=lambda k: best_by_input_id[k][1])

            # Best candidate in B+ sector (input_id < N_BP_MODES * 2) and B0 sector (>= N_BP_MODES * 2)
            bp_sector_ids = {k: v for k, v in best_by_input_id.items() if k < config.N_BP_MODES * 2}
            b0_sector_ids = {k: v for k, v in best_by_input_id.items() if k >= config.N_BP_MODES * 2}

            best_is_bp = max_input_id < config.N_BP_MODES * 2
            if best_is_bp and b0_sector_ids:
                scnd_max_input_id = max(b0_sector_ids.keys(), key=lambda k: b0_sector_ids[k][1])
            elif not best_is_bp and bp_sector_ids:
                scnd_max_input_id = max(bp_sector_ids.keys(), key=lambda k: bp_sector_ids[k][1])
            else:
                scnd_max_input_id = -10
        else:
            max_input_id = 0
            scnd_max_input_id = -10

        event_feat_values.append(max_input_id / 50.0)
        event_feat_values.append(scnd_max_input_id / 50.0)

        # Add experiment number (from EventMetaData)
        event_meta = Belle2.PyStoreObj('EventMetaData')
        experiment = int(event_meta.getExperiment()) if event_meta.isValid() else 0
        event_feat_values.append(experiment / 10.0)

        # Concatenate all features
        all_features = np.concatenate([flat_features, np.array(event_feat_values, dtype=np.float32)])

        return all_features, max_input_id, n_candidates

    def _get_event_features(self):
        """Extract event-level features from EventShapeContainer."""
        event_features = {}

        # Event shape variables are stored in EventShapeContainer
        event_shape = Belle2.PyStoreObj('EventShapeContainer')
        if event_shape.isValid():
            obj = event_shape.obj()
            # Map feature names to EventShapeContainer methods
            if 'sphericity' in self.event_features:
                # Sphericity = 3/2 * (lambda2 + lambda3)
                event_features['sphericity'] = 1.5 * (obj.getSphericityEigenvalue(1) + obj.getSphericityEigenvalue(2))
            if 'thrust' in self.event_features:
                event_features['thrust'] = obj.getThrust()
            if 'thrustAxisCosTheta' in self.event_features:
                thrust_axis = obj.getThrustAxis()
                # Compute cosTheta = z / |r|
                import math
                r = math.sqrt(thrust_axis.X()**2 + thrust_axis.Y()**2 + thrust_axis.Z()**2)
                event_features['thrustAxisCosTheta'] = thrust_axis.Z() / r if r > 0 else 0.0
            if 'aplanarity' in self.event_features:
                # Aplanarity = 1.5 * smallest sphericity eigenvalue
                event_features['aplanarity'] = 1.5 * obj.getSphericityEigenvalue(2)
            if 'foxWolframR2' in self.event_features:
                # R2 = H2/H0
                h0 = obj.getFWMoment(0)
                h2 = obj.getFWMoment(2)
                event_features['foxWolframR2'] = h2 / h0 if h0 != 0 else 0.0
            if 'harmonicMomentThrust0' in self.event_features:
                event_features['harmonicMomentThrust0'] = obj.getHarmonicMomentThrust(0)
            if 'harmonicMomentThrust1' in self.event_features:
                event_features['harmonicMomentThrust1'] = obj.getHarmonicMomentThrust(1)
            if 'harmonicMomentThrust2' in self.event_features:
                event_features['harmonicMomentThrust2'] = obj.getHarmonicMomentThrust(2)

        return event_features

    #: MC truth variables to evaluate on best candidates (for training labels)
    TRAINING_MC_VARS = config.TRAINING_MC_VARS

    def _extract_mc_truth(self, particle):
        """Extract MC truth variables from a particle for training labels."""
        if particle is None:
            return {name: np.nan for name in self.TRAINING_MC_VARS}
        truth = {}
        for var_name in self.TRAINING_MC_VARS:
            val = vm.evaluate(var_name, particle)
            truth[var_name] = float(val) if not np.isnan(val) else np.nan
        return truth

    def _print_debug_info(self, candidates_data, event_features, all_features, max_input_id):
        """Print debug information for preprocessing."""
        # Get event identification
        event_meta = Belle2.PyStoreObj('EventMetaData')
        if event_meta.isValid():
            exp = event_meta.getExperiment()
            run = event_meta.getRun()
            evt = event_meta.getEvent()
            event_id_str = f"exp={exp}, run={run}, evt={evt}"
        else:
            event_id_str = "unknown"

        print(f"\n{'='*60}")
        print(f"DEBUG Event {self.event_count} ({event_id_str})")
        print(f"{'='*60}")
        print(f"Number of candidates: {len(candidates_data)}")
        print(f"Max input_id (best candidate): {max_input_id}")

        print("\n--- Candidates ---")
        for i, (input_id, features) in enumerate(candidates_data):
            print(f"  Candidate {i}: input_id={input_id}")
            for key, val in features.items():
                print(f"    {key}: {val}")

        print("\n--- Event Features ---")
        for key, val in event_features.items():
            print(f"  {key}: {val}")

        print("\n--- Feature Array (non-zero, first 5 blocks) ---")
        n_blocks = len(self.feature_blocks)
        for block_idx in range(min(5, n_blocks)):
            block_name = self.feature_blocks[block_idx][0]
            start = block_idx * self.n_input_ids
            end = start + self.n_input_ids
            block_data = all_features[start:end]
            non_zero = [(j, v) for j, v in enumerate(block_data) if v != 0]
            if non_zero:
                print(f"  {block_name}: {non_zero[:10]}...")

        # Print last few features (event-level)
        n_candidate_features = n_blocks * self.n_input_ids
        event_feat_start = n_candidate_features
        print(f"\n--- Event-level features (indices {event_feat_start}+) ---")
        event_feat_names = self.event_features + ['ncandidates/10', 'max_input_id/50', 'scnd_max_input_id/50', '__experiment__/10']
        for i, name in enumerate(event_feat_names):
            idx = event_feat_start + i
            if idx < len(all_features):
                print(f"  {name}: {all_features[idx]}")

        print(f"\n--- Total feature array shape: {len(all_features)} ---")

    def terminate(self):
        """Called at the end of processing."""
        if self.training_mode and self.training_data:
            self._save_training_data()

        if self.debug and self.debug_features:
            # Save debug features and NN outputs to file
            debug_file = 'modeSelector_debug_features.npz'
            feature_arrays = np.array([d['all_features'] for d in self.debug_features])
            cat_outputs = np.array([d['cat_output'] for d in self.debug_features])
            main_outputs = np.array([d['main_output'] for d in self.debug_features])
            np.savez(debug_file,
                     features=feature_arrays,
                     cat_outputs=cat_outputs,
                     main_outputs=main_outputs,
                     charged_cat=[d['charged_cat'] for d in self.debug_features],
                     bp_score=[d['bp_score'] for d in self.debug_features],
                     n_candidates=[d['n_candidates'] for d in self.debug_features],
                     max_input_ids=[d['max_input_id'] for d in self.debug_features],
                     exp=[d['exp'] for d in self.debug_features],
                     run=[d['run'] for d in self.debug_features],
                     evt=[d['evt'] for d in self.debug_features])
            print(f"\n[DEBUG] Saved {len(self.debug_features)} events to {debug_file}")

    def _save_training_data(self):
        """Save collected training data to npz."""
        from scipy import sparse

        n_events = len(self.training_data)
        feature_arrays = np.array([d['all_features'] for d in self.training_data])

        # Convert to sparse for efficient storage
        sparse_features = sparse.csr_matrix(feature_arrays)

        # Event metadata
        exp = np.array([d['exp'] for d in self.training_data])
        run = np.array([d['run'] for d in self.training_data])
        evt = np.array([d['evt'] for d in self.training_data])
        max_input_ids = np.array([d['max_input_id'] for d in self.training_data])
        n_candidates = np.array([d['n_candidates'] for d in self.training_data])

        # MC truth for best B+ and B0 candidates
        mc_var_names = self.TRAINING_MC_VARS
        bp_truth = np.array([
            [d['bp_mc_truth'][v] for v in mc_var_names]
            for d in self.training_data
        ])
        b0_truth = np.array([
            [d['b0_mc_truth'][v] for v in mc_var_names]
            for d in self.training_data
        ])

        sparse.save_npz(self.training_output.replace('.npz', '_features.npz'), sparse_features)
        np.savez(self.training_output,
                 exp=exp, run=run, evt=evt,
                 max_input_ids=max_input_ids,
                 n_candidates=n_candidates,
                 bp_truth=bp_truth,
                 b0_truth=b0_truth,
                 mc_var_names=mc_var_names)

        print(f"\n[TRAINING] Saved {n_events} events to {self.training_output}")
        print(f"[TRAINING] Sparse features: {self.training_output.replace('.npz', '_features.npz')}")
        print(f"[TRAINING] MC truth variables: {mc_var_names}")

    def event(self):
        """Called for each event."""
        # Check that EventShapeCalculator was run (once per job)
        if not self._event_shape_checked:
            self._event_shape_checked = True
            _es = Belle2.PyStoreObj('EventShapeContainer')
            if not _es.isValid():
                b2.B2FATAL(
                    "ModeSelector: EventShapeContainer not found. "
                    "Run the EventShapeCalculator module before ModeSelector."
                )

        # Collect all candidates from all particle lists
        candidates_data = []
        best_bp = None
        best_bp_sig = -1
        best_b0 = None
        best_b0_sig = -1

        for list_name in self.particle_lists:
            plist = Belle2.PyStoreObj(list_name)
            if not plist.isValid():
                continue

            for i in range(plist.getListSize()):
                particle = plist.obj().getParticle(i)

                # Check that DstarVeto was run (once per job, on first particle)
                if not self._dstar_veto_checked:
                    self._dstar_veto_checked = True
                    if not particle.hasExtraInfo('Dst0_deltaMassDiff'):
                        b2.B2FATAL(
                            "ModeSelector: D* veto ExtraInfo (Dst0_deltaMassDiff) not found on B candidates. "
                            "Run the DstarVeto module before ModeSelector."
                        )

                input_id, features = self._extract_particle_features(particle)
                candidates_data.append((input_id, features))

                # Track best B+ and B0 by sigProb
                sig_prob = features.get('sigProb', -1) or -1
                pdg = abs(int(vm.evaluate('PDG', particle)))
                if pdg == 521 and sig_prob > best_bp_sig:
                    best_bp_sig = sig_prob
                    best_bp = particle
                elif pdg == 511 and sig_prob > best_b0_sig:
                    best_b0_sig = sig_prob
                    best_b0 = particle

        if not candidates_data:
            return

        # Get event-level features
        event_features = self._get_event_features()

        # Build feature array
        all_features, max_input_id, n_unique_candidates = self._build_feature_array(candidates_data, event_features)

        # Training mode: save features + MC truth, skip NN inference
        if self.training_mode:
            event_meta = Belle2.PyStoreObj('EventMetaData')
            entry = {
                'exp': event_meta.getExperiment() if event_meta.isValid() else -1,
                'run': event_meta.getRun() if event_meta.isValid() else -1,
                'evt': event_meta.getEvent() if event_meta.isValid() else -1,
                'all_features': all_features.copy(),
                'max_input_id': max_input_id,
                'n_candidates': n_unique_candidates,
                'bp_mc_truth': self._extract_mc_truth(best_bp),
                'b0_mc_truth': self._extract_mc_truth(best_b0),
            }
            self.training_data.append(entry)
            return

        # --- Inference mode ---

        # Select features based on has_inputs (indices used during training)
        if self.has_inputs is not None:
            features = all_features[self.has_inputs]
        else:
            features = all_features

        # Verify feature size matches model expectation
        if len(features) != self.cat_input_size:
            b2.B2FATAL(
                f"ModeSelector: category model input size mismatch: "
                f"got {len(features)} features, model expects {self.cat_input_size}. "
                f"The ONNX model and current config must match."
            )

        # Run category network
        cat_input = features.reshape(1, -1).astype(np.float32)
        cat_output = self.cat_session.run(None, {self.cat_input_name: cat_input})[0][0]

        # Determine predicted category (0=B0, 1=B+, 2=continuum)
        charged_cat = 1.0 if cat_output[1] > cat_output[0] else 0.0

        # Build main network input (features + cat_output + charged_cat)
        main_features = np.concatenate([features, cat_output, [charged_cat]])

        if len(main_features) != self.main_input_size:
            b2.B2FATAL(
                f"ModeSelector: main model input size mismatch: "
                f"got {len(main_features)} features, model expects {self.main_input_size}. "
                f"The ONNX model and current config must match."
            )

        main_input = main_features.reshape(1, -1).astype(np.float32)
        main_output = self.main_session.run(None, {self.main_input_name: main_input})[0][0]

        # Extract scores
        # Main network outputs (6 classes after softmax):
        #   [0] bad_tag, [1] is_target_neutral, [2] cross_deltaC1,
        #   [3] cross_internal, [4] continuum, [5] is_target_charged
        bp_score = float(main_output[5] - main_output[1])

        # eqSigProb: maps Bp_score to [0, 1] range
        eq_sig_prob_bp = 0.5 + bp_score / 2
        eq_sig_prob_b0 = 0.5 - bp_score / 2

        # Store eqSigProb on best B+ and B0 candidates (candidate-specific)
        if best_bp is not None:
            best_bp.addExtraInfo(f'{self.output_variable}_eqSigProb', eq_sig_prob_bp)
        if best_b0 is not None:
            best_b0.addExtraInfo(f'{self.output_variable}_eqSigProb', eq_sig_prob_b0)

        # Store event-level outputs in EventExtraInfo
        event_extra_info = Belle2.PyStoreObj('EventExtraInfo')
        if not event_extra_info.isValid():
            event_extra_info.create()
        event_extra_info.addExtraInfo(self.output_variable, bp_score)
        event_extra_info.addExtraInfo(f'{self.output_variable}_catB0', float(cat_output[0]))
        event_extra_info.addExtraInfo(f'{self.output_variable}_catBp', float(cat_output[1]))
        event_extra_info.addExtraInfo(f'{self.output_variable}_catCont', float(cat_output[2]))

        # Debug output (after NN inference so we can save outputs too)
        if self.debug:
            event_meta = Belle2.PyStoreObj('EventMetaData')
            if event_meta.isValid():
                self.debug_features.append({
                    'exp': event_meta.getExperiment(),
                    'run': event_meta.getRun(),
                    'evt': event_meta.getEvent(),
                    'all_features': all_features.copy(),
                    'max_input_id': max_input_id,
                    'n_candidates': n_unique_candidates,
                    'cat_output': cat_output.copy(),
                    'main_output': main_output.copy(),
                    'charged_cat': charged_cat,
                    'bp_score': bp_score,
                })

            if self.event_count < self.debug_max_events:
                self._print_debug_info(candidates_data, event_features, all_features, max_input_id)

            self.event_count += 1
