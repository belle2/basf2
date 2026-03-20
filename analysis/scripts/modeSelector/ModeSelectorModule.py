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
4. Stores results as ExtraInfo
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
    TR_EVENT_FIELDS = ('all_features',)
    TR_EVENT_BEST_FIELDS = ('best_bp_iid', 'best_bp_dp', 'best_b0_iid', 'best_b0_dp')
    TR_BEST_FIELDS = ('pdg', 'dm', 'sigprob', 'is_cont', 'tag_pdg', 'gen_dm_id', 'gen_calib_weight')
    AUXILIARY_OUTPUT_PREFIX = 'modeSelector'

    def __init__(
        self,
        particle_lists,
        cat_model_path=None,
        main_model_path=None,
        output_variable='BplusScore',
        payload_cat_model='modeSelector_cat_model_v2',
        payload_main_model='modeSelector_main_model_v2',
        training_mode=False,
        skip_nn_evaluation=False,
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
        #: Skip NN inference and fill deterministic placeholder outputs
        self.skip_nn_evaluation = skip_nn_evaluation
        #: Output file for training mode
        self.training_output = training_output
        #: Training storage (columnar buffers)
        self._tr_event = {name: [] for name in self.TR_EVENT_FIELDS}
        self._tr_event_best = {name: [] for name in self.TR_EVENT_BEST_FIELDS}
        self._tr_bp = {name: [] for name in self.TR_BEST_FIELDS}
        self._tr_b0 = {name: [] for name in self.TR_BEST_FIELDS}
        self._tr_sig_input_ids = []
        self._tr_sig_btag_index = []
        self._tr_sig_delta_p = []
        self._tr_sig_sigprob = []
        #: Debug mode
        self.debug = debug
        #: Max events to debug
        self.debug_max_events = debug_max_events
        #: Event counter for debug
        self.event_count = 0
        #: Flag to check event shape prerequisite once
        self._event_shape_checked = False
        #: Flag to check D* veto prerequisite once
        self._dstar_veto_checked = False
        #: Number of events where predicted sector had no candidate in the event
        self._empty_predicted_sector_count = 0
        #: Number of inference events processed
        self._inference_event_count = 0
        #: Number of high-confidence inference events (abs(BplusScore) > threshold)
        self._high_conf_event_count = 0
        #: Number of events where sector top-output input_id has no candidate in the event
        self._missing_top_mode_count = 0
        #: Number of high-confidence events where sector top-output input_id has no candidate
        self._missing_top_mode_high_conf_count = 0
        #: Number of high-confidence events where fallback was used
        self._empty_predicted_sector_high_conf_count = 0

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

        if self.skip_nn_evaluation:
            self.has_inputs = list(config.HAS_INPUTS)
            self.cat_input_size = len(self.has_inputs)
            self.main_input_size = self.cat_input_size + 4
            b2.B2INFO("ModeSelector: Running with NN evaluation disabled")
            b2.B2INFO(
                f"ModeSelector: Using placeholder outputs with {self.cat_input_size} selected features"
            )
            return

        import onnxruntime as ort

        self.has_inputs = list(config.HAS_INPUTS)
        b2.B2INFO(f"ModeSelector: Using config.HAS_INPUTS ({len(self.has_inputs)} features kept)")

        # Use single-threaded execution to avoid thread pool contention with
        # other ONNX sessions in the same basf2 path (e.g. MVAMultipleExperts).
        sess_opts = ort.SessionOptions()
        sess_opts.intra_op_num_threads = 1
        sess_opts.inter_op_num_threads = 1
        sess_opts.execution_mode = ort.ExecutionMode.ORT_SEQUENTIAL

        # Load models from files or database
        if self.cat_model_path:
            self.cat_session = ort.InferenceSession(self.cat_model_path, sess_opts)
        else:
            db_accessor = Belle2.DBAccessorBase(
                Belle2.DBStoreEntry.c_RawFile, self.payload_cat_model, True
            )
            self.cat_session = ort.InferenceSession(db_accessor.getFilename(), sess_opts)

        if self.main_model_path:
            self.main_session = ort.InferenceSession(self.main_model_path, sess_opts)
        else:
            db_accessor = Belle2.DBAccessorBase(
                Belle2.DBStoreEntry.c_RawFile, self.payload_main_model, True
            )
            self.main_session = ort.InferenceSession(db_accessor.getFilename(), sess_opts)

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

    @staticmethod
    def _assign_rank_extra_info(particle_score_input_id_triples, variable_name):
        """Assign deterministic descending ranks to the given particles."""
        ranked_pairs = sorted(
            particle_score_input_id_triples,
            key=lambda triple: (-triple[1], triple[2])
        )
        for rank, (particle, _, _) in enumerate(ranked_pairs, start=1):
            particle.addExtraInfo(variable_name, rank)

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

    def _append_training_row(self, block, row):
        """Append a row dict into a columnar training block."""
        for key in block:
            block[key].append(row[key])

    def _block_to_arrays(self, block, dtype_map=None):
        """Convert a columnar training block (lists) into numpy arrays."""
        arrays = {}
        for key, values in block.items():
            if dtype_map and key in dtype_map:
                arrays[key] = np.asarray(values, dtype=dtype_map[key])
            else:
                arrays[key] = np.asarray(values)
        return arrays

    def _compute_fei_calib_weight(self, best_bp, best_b0):
        """
        Compute event-level FEI calibration weight from the best candidates.

        Uses the reco path (truth-compatible tag PDG and DeltaP < DELTA_P_THRESH)
        when applicable, falling back to the gen path (generated decay mode ID)
        and finally the sector rest weight. Returns FEI_CALIB_CONT for continuum
        events or when generated-decay annotation marked the candidate as
        continuum or missing generated B truth.

        Requires extraInfo(genDecayModeID) to be available on candidates;
        add addGeneratedDecayWeights before ModeSelector in the basf2 path.
        If genDecayModeID is unavailable at this point, the function returns
        NaN to distinguish that case from the explicit sentinel value -1.
        """
        bp_sig = -1.0
        b0_sig = -1.0
        if best_bp is not None:
            v = vm.evaluate('extraInfo(SignalProbability)', best_bp)
            if not np.isnan(v):
                bp_sig = float(v)
        if best_b0 is not None:
            v = vm.evaluate('extraInfo(SignalProbability)', best_b0)
            if not np.isnan(v):
                b0_sig = float(v)

        best = best_bp if bp_sig >= b0_sig else best_b0
        if best is None:
            return config.FEI_CALIB_CONT

        is_cont_v = vm.evaluate('isContinuumEvent', best)
        if np.isnan(is_cont_v) or int(is_cont_v) == 1:
            return config.FEI_CALIB_CONT

        pdg_v = vm.evaluate('PDG', best)
        dm_v = vm.evaluate('extraInfo(decayModeID)', best)
        tag_pdg_v = vm.evaluate('mostcommonBTagPDG', best)
        dp_v = vm.evaluate('mostcommonBTagDeltaP', best)
        gen_dm_v = vm.evaluate('extraInfo(genDecayModeID)', best)

        if np.isnan(pdg_v) or np.isnan(dm_v):
            return config.FEI_CALIB_CONT

        abs_pdg = abs(int(pdg_v))
        if abs_pdg not in (511, 521):
            return config.FEI_CALIB_CONT

        calib_map = config.get_fei_calibration_map(abs_pdg)
        calib_rest = config.get_fei_calibration_rest(abs_pdg)

        # Reco path: truth-compatible tag PDG and low DeltaP
        tag_is_gen = (not np.isnan(tag_pdg_v)) and config.truth_tag_matches_pdg(pdg_v, tag_pdg_v, dm_v)
        dp_ok = (not np.isnan(dp_v)) and float(dp_v) < config.DELTA_P_THRESH
        if tag_is_gen and dp_ok:
            iid = self._get_input_id(best)
            if abs_pdg == 521:
                dm = max(0, min(iid // 2, config.N_BP_MODES - 1))
            else:
                dm = max(0, min((iid - config.N_BP_MODES * 2) // 2, config.N_B0_MODES - 1))
            return calib_map.get(dm, calib_rest)

        # Gen path: use generated decay mode ID if available.
        # -1 is the explicit sentinel for continuum or missing generated B truth.
        if not np.isnan(gen_dm_v):
            if int(gen_dm_v) == -1:
                return config.FEI_CALIB_CONT
            if int(gen_dm_v) > 0:
                return calib_map.get(int(gen_dm_v), calib_rest)
            return np.nan
        return np.nan

    def _get_placeholder_outputs(self, features):
        """Return deterministic placeholder category and main-network outputs."""
        cat_output = np.array([0.2, 0.7, 0.1], dtype=np.float32)
        main_output = np.full(config.N_INPUT_IDS + 3, 0.01, dtype=np.float32)
        main_output[config.N_INPUT_IDS] = 0.005
        main_output[config.N_INPUT_IDS + 1] = 0.002
        main_output[config.N_INPUT_IDS + 2] = 0.001
        return cat_output, main_output

    def _extract_mc_truth_scalars(self, particle):
        """Extract compact MC truth scalars used by training output."""
        if particle is None:
            return {
                'pdg': np.nan,
                'dm': np.nan,
                'sigprob': np.nan,
                'is_cont': np.nan,
                'tag_pdg': np.nan,
                'gen_dm_id': np.nan,
                'gen_calib_weight': np.nan,
                'dp': np.nan,
            }

        raw = {
            'pdg': vm.evaluate('PDG', particle),
            'dm': vm.evaluate('extraInfo(decayModeID)', particle),
            'sigprob': vm.evaluate('extraInfo(SignalProbability)', particle),
            'is_cont': vm.evaluate('isContinuumEvent', particle),
            'tag_pdg': vm.evaluate('mostcommonBTagPDG', particle),
            'gen_dm_id': vm.evaluate('extraInfo(genDecayModeID)', particle),
            'gen_calib_weight': vm.evaluate('extraInfo(genFEICalibWeight)', particle),
            'dp': vm.evaluate('mostcommonBTagDeltaP', particle),
        }
        return {
            name: np.nan if np.isnan(value) else value
            for name, value in raw.items()
        }

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
        if self.training_mode and self._tr_event['all_features']:
            self._save_training_data()

        if self._inference_event_count > 0:
            missing_base_count = self._inference_event_count - self._empty_predicted_sector_count
            missing_frac = 0.0
            if missing_base_count > 0:
                missing_frac = self._missing_top_mode_count / missing_base_count
            high_conf_missing_base_count = self._high_conf_event_count - self._empty_predicted_sector_high_conf_count
            high_conf_missing_frac = 0.0
            if high_conf_missing_base_count > 0:
                high_conf_missing_frac = self._missing_top_mode_high_conf_count / high_conf_missing_base_count
            b2.B2INFO(
                "ModeSelector: missing top mode among event candidates "
                "(excluding events with no predicted-sector candidate): "
                f"{self._missing_top_mode_count}/{missing_base_count} event(s) "
                f"({100.0 * missing_frac:.3f}%), high-confidence (>{config.HIGH_CONF_BPLUSSCORE_ABS}) "
                f"{self._missing_top_mode_high_conf_count}/{high_conf_missing_base_count} event(s) "
                f"({100.0 * high_conf_missing_frac:.3f}%)."
            )
            if high_conf_missing_frac > config.MONITOR_WARN_FRACTION:
                b2.B2WARNING(
                    "ModeSelector: missing top mode high-confidence fraction exceeds threshold "
                    f"({100.0 * high_conf_missing_frac:.3f}% > {100.0 * config.MONITOR_WARN_FRACTION:.3f}%)."
                )

        if self._inference_event_count > 0:
            fallback_frac = self._empty_predicted_sector_count / self._inference_event_count
            high_conf_fallback_frac = 0.0
            if self._high_conf_event_count > 0:
                high_conf_fallback_frac = self._empty_predicted_sector_high_conf_count / self._high_conf_event_count
            b2.B2INFO(
                "ModeSelector: predicted sector had no candidate in "
                f"{self._empty_predicted_sector_count}/{self._inference_event_count} event(s) "
                f"({100.0 * fallback_frac:.3f}%), high-confidence (>{config.HIGH_CONF_BPLUSSCORE_ABS}) "
                f"{self._empty_predicted_sector_high_conf_count}/{self._high_conf_event_count} event(s) "
                f"({100.0 * high_conf_fallback_frac:.3f}%); used score fallback."
            )
            if high_conf_fallback_frac > config.MONITOR_WARN_FRACTION:
                b2.B2WARNING(
                    "ModeSelector: predicted-sector-empty high-confidence fraction exceeds threshold "
                    f"({100.0 * high_conf_fallback_frac:.3f}% > {100.0 * config.MONITOR_WARN_FRACTION:.3f}%)."
                )

    def _save_training_data(self):
        """Save collected training data to npz."""
        from scipy import sparse

        ev = self._block_to_arrays(
            self._tr_event,
            dtype_map={'all_features': np.float32}
        )
        ev_best = self._block_to_arrays(
            self._tr_event_best,
            dtype_map={
                'best_bp_iid': np.int16,
                'best_bp_dp': np.float32,
                'best_b0_iid': np.int16,
                'best_b0_dp': np.float32,
            }
        )
        bp = self._block_to_arrays(self._tr_bp, dtype_map={key: np.float32 for key in self.TR_BEST_FIELDS})
        b0 = self._block_to_arrays(self._tr_b0, dtype_map={key: np.float32 for key in self.TR_BEST_FIELDS})

        n_events = len(ev['all_features'])
        feature_arrays = ev['all_features']

        # Convert to sparse for efficient storage
        sparse_features = sparse.csr_matrix(feature_arrays)

        best_bp_iid = ev_best['best_bp_iid']
        best_bp_dp = ev_best['best_bp_dp']
        best_b0_iid = ev_best['best_b0_iid']
        best_b0_dp = ev_best['best_b0_dp']

        # Compact per-event MC truth scalars (14 bytes/event)
        bp_pdg = bp['pdg']
        b0_pdg = b0['pdg']
        bp_dm = bp['dm']
        b0_dm = b0['dm']
        bp_sig = np.nan_to_num(bp['sigprob'], nan=-1.0)
        b0_sig = np.nan_to_num(b0['sigprob'], nan=-1.0)
        bp_is_cont = bp['is_cont']
        b0_is_cont = b0['is_cont']
        bp_gen_pdg = bp['tag_pdg']
        b0_gen_pdg = b0['tag_pdg']
        bp_gen_dm_id = np.nan_to_num(bp['gen_dm_id'], nan=-1).astype(np.int16)
        b0_gen_dm_id = np.nan_to_num(b0['gen_dm_id'], nan=-1).astype(np.int16)
        bp_gen_calib_weight = np.nan_to_num(bp['gen_calib_weight'], nan=1.0).astype(np.float32)
        b0_gen_calib_weight = np.nan_to_num(b0['gen_calib_weight'], nan=1.0).astype(np.float32)

        bp_is_best = (bp_sig >= b0_sig).astype(np.int8)
        best_sigprob = np.maximum(bp_sig, b0_sig).astype(np.float32)

        # choosing from Bp/B0 to have at least one candidate and not return NaN
        is_cont_f = np.where(bp_is_best == 1, bp_is_cont, b0_is_cont)
        is_cont = (is_cont_f == 1.0).astype(np.int8)

        gen_pdg_f = np.where(bp_is_best == 1, bp_gen_pdg, b0_gen_pdg)
        gen_pdg = np.where(is_cont == 1, -1, np.nan_to_num(gen_pdg_f, nan=-1)).astype(np.int16)

        bp_dm_i = np.nan_to_num(bp_dm, nan=-1).astype(np.int16)
        b0_dm_i = np.nan_to_num(b0_dm, nan=-1).astype(np.int16)

        best_bp_sigprob_iid = np.full(n_events, -1, dtype=np.int16)
        best_b0_sigprob_iid = np.full(n_events, -1, dtype=np.int16)
        bp_valid = (~np.isnan(bp_pdg)) & (~np.isnan(bp_dm))
        b0_valid = (~np.isnan(b0_pdg)) & (~np.isnan(b0_dm))
        bp_offset = np.where(np.abs(bp_pdg) == 521, 0, config.N_BP_MODES * 2)
        b0_offset = np.where(np.abs(b0_pdg) == 521, 0, config.N_BP_MODES * 2)
        bp_sign = (bp_pdg > 0).astype(np.int16)
        b0_sign = (b0_pdg > 0).astype(np.int16)
        best_bp_sigprob_iid[bp_valid] = (bp_offset[bp_valid] + bp_dm_i[bp_valid] * 2 + bp_sign[bp_valid]).astype(np.int16)
        best_b0_sigprob_iid[b0_valid] = (b0_offset[b0_valid] + b0_dm_i[b0_valid] * 2 + b0_sign[b0_valid]).astype(np.int16)

        bp_tag_pdg = bp_gen_pdg
        b0_tag_pdg = b0_gen_pdg
        bp_tag_is_gen = np.fromiter(
            (
                int(config.truth_tag_matches_pdg(pdg, tag_pdg, dm_id))
                for pdg, tag_pdg, dm_id in zip(bp_pdg, bp_tag_pdg, bp_dm)
            ),
            dtype=np.int8,
            count=n_events,
        )
        b0_tag_is_gen = np.fromiter(
            (
                int(config.truth_tag_matches_pdg(pdg, tag_pdg, dm_id))
                for pdg, tag_pdg, dm_id in zip(b0_pdg, b0_tag_pdg, b0_dm)
            ),
            dtype=np.int8,
            count=n_events,
        )

        # Packed per-event input_id lists where isSignal == 1 on the deduplicated
        # candidate set (particle_by_input_id). Event i list is:
        #   sig_input_ids_values[sig_input_ids_offsets[i]:sig_input_ids_offsets[i+1]]
        sig_lengths = np.asarray([len(ids) for ids in self._tr_sig_input_ids], dtype=np.int32)
        sig_input_ids_offsets = np.empty(n_events + 1, dtype=np.int32)
        sig_input_ids_offsets[0] = 0
        np.cumsum(sig_lengths, out=sig_input_ids_offsets[1:])
        if sig_input_ids_offsets[-1] > 0:
            sig_input_ids_values = np.concatenate(self._tr_sig_input_ids).astype(np.int16, copy=False)
            sig_btag_index_values = np.concatenate(self._tr_sig_btag_index).astype(np.int16, copy=False)
            sig_delta_p_values = np.concatenate(self._tr_sig_delta_p).astype(np.float32, copy=False)
            sig_sigprob_values = np.concatenate(self._tr_sig_sigprob).astype(np.float32, copy=False)
        else:
            sig_input_ids_values = np.empty(0, dtype=np.int16)
            sig_btag_index_values = np.empty(0, dtype=np.int16)
            sig_delta_p_values = np.empty(0, dtype=np.float32)
            sig_sigprob_values = np.empty(0, dtype=np.float32)

        # Compute per-event FEI calibration weight (same logic as compute_event_weights in train.py)
        bp_calib_map = config.get_fei_calibration_map(521)
        bp_calib_rest = config.get_fei_calibration_rest(521)
        bp_lookup = np.full(config.N_BP_MODES, bp_calib_rest, dtype=np.float32)
        for _dm, _w in bp_calib_map.items():
            if 0 <= _dm < config.N_BP_MODES:
                bp_lookup[_dm] = _w

        b0_calib_map = config.get_fei_calibration_map(511)
        b0_calib_rest = config.get_fei_calibration_rest(511)
        b0_lookup = np.full(config.N_B0_MODES, b0_calib_rest, dtype=np.float32)
        for _dm, _w in b0_calib_map.items():
            if 0 <= _dm < config.N_B0_MODES:
                b0_lookup[_dm] = _w

        use_bp = (bp_is_best == 1)
        tag_is_gen_ev = np.where(use_bp, bp_tag_is_gen.astype(np.int8), b0_tag_is_gen.astype(np.int8))
        best_dp_ev = np.where(use_bp, best_bp_dp, best_b0_dp)
        sigprob_iid_ev = np.where(
            use_bp,
            best_bp_sigprob_iid.astype(np.int32),
            best_b0_sigprob_iid.astype(np.int32),
        )
        bb_mask_ev = (is_cont != 1)
        use_reco_ev = (tag_is_gen_ev == 1) & (best_dp_ev < config.DELTA_P_THRESH) & (sigprob_iid_ev >= 0)

        fei_calib_weight = np.full(n_events, config.FEI_CALIB_CONT, dtype=np.float32)

        bp_threshold = config.N_BP_MODES * 2
        reco_mask_ev = bb_mask_ev & use_reco_ev
        reco_bp_ev = reco_mask_ev & use_bp
        if reco_bp_ev.any():
            _dm = np.clip(best_bp_sigprob_iid[reco_bp_ev].astype(np.int32) // 2, 0, config.N_BP_MODES - 1)
            fei_calib_weight[reco_bp_ev] = bp_lookup[_dm]
        reco_b0_ev = reco_mask_ev & ~use_bp
        if reco_b0_ev.any():
            _dm = np.clip(
                (best_b0_sigprob_iid[reco_b0_ev].astype(np.int32) - bp_threshold) // 2,
                0, config.N_B0_MODES - 1,
            )
            fei_calib_weight[reco_b0_ev] = b0_lookup[_dm]

        gen_mask_ev = bb_mask_ev & ~use_reco_ev
        if gen_mask_ev.any():
            abs_pdg_ev = np.abs(gen_pdg.astype(np.int32))
            gen_dm_ev = np.where(
                use_bp,
                bp_gen_dm_id.astype(np.int32),
                b0_gen_dm_id.astype(np.int32),
            )
            gen_bp_ev = gen_mask_ev & (abs_pdg_ev == 521)
            if gen_bp_ev.any():
                _dm = gen_dm_ev[gen_bp_ev]
                _w = np.full(int(gen_bp_ev.sum()), bp_calib_rest, dtype=np.float32)
                valid = (_dm >= 0) & (_dm < config.N_BP_MODES)
                if valid.any():
                    _w[valid] = bp_lookup[_dm[valid]]
                fei_calib_weight[gen_bp_ev] = _w
            gen_b0_ev = gen_mask_ev & (abs_pdg_ev == 511)
            if gen_b0_ev.any():
                _dm = gen_dm_ev[gen_b0_ev]
                _w = np.full(int(gen_b0_ev.sum()), b0_calib_rest, dtype=np.float32)
                valid = (_dm >= 0) & (_dm < config.N_B0_MODES)
                if valid.any():
                    _w[valid] = b0_lookup[_dm[valid]]
                fei_calib_weight[gen_b0_ev] = _w

        sparse.save_npz(self.training_output.replace('.npz', '_features.npz'), sparse_features)
        np.savez_compressed(self.training_output,
                            is_cont=is_cont,
                            gen_pdg=gen_pdg,
                            bp_gen_decay_mode_id=bp_gen_dm_id,
                            b0_gen_decay_mode_id=b0_gen_dm_id,
                            bp_gen_fei_calib_weight=bp_gen_calib_weight,
                            b0_gen_fei_calib_weight=b0_gen_calib_weight,
                            bp_is_best=bp_is_best,
                            best_sigprob=best_sigprob,
                            best_bp_sigprob_iid=best_bp_sigprob_iid,
                            best_b0_sigprob_iid=best_b0_sigprob_iid,
                            bp_tag_is_gen=bp_tag_is_gen,
                            b0_tag_is_gen=b0_tag_is_gen,
                            fei_calib_weight=fei_calib_weight,
                            best_bp_iid=best_bp_iid,
                            best_bp_dp=best_bp_dp,
                            best_b0_iid=best_b0_iid,
                            best_b0_dp=best_b0_dp,
                            sig_input_ids_values=sig_input_ids_values,
                            sig_input_ids_offsets=sig_input_ids_offsets,
                            sig_btag_index_values=sig_btag_index_values,
                            sig_delta_p_values=sig_delta_p_values,
                            sig_sigprob_values=sig_sigprob_values)

        n_bp_signal = int((best_bp_iid >= 0).sum())
        n_b0_signal = int((best_b0_iid >= 0).sum())
        print(f"\n[TRAINING] Saved {n_events} events to {self.training_output}")
        print(f"[TRAINING] Sparse features: {self.training_output.replace('.npz', '_features.npz')}")
        print(f"[TRAINING] is_target found: B+ sector {n_bp_signal} events, B0 sector {n_b0_signal} events")
        print(f"[TRAINING] Mean best sigProb: {best_sigprob.mean():.4f}")

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
        particle_by_input_id = {}   # input_id -> Particle with highest sigProb
        _sigprob_by_input_id = {}   # input_id -> sigProb for deduplication

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

                # Track best particle per input_id (same deduplication as _build_feature_array)
                if 0 <= input_id < self.n_input_ids:
                    prev_sig = _sigprob_by_input_id.get(input_id, -2)
                    if sig_prob > prev_sig:
                        _sigprob_by_input_id[input_id] = sig_prob
                        particle_by_input_id[input_id] = particle

        if not candidates_data:
            return

        # Get event-level features
        event_features = self._get_event_features()

        # Build feature array
        all_features, max_input_id, n_candidates = self._build_feature_array(candidates_data, event_features)

        # Training mode: save features + MC truth, skip NN inference
        if self.training_mode:
            event_row = {
                'all_features': all_features.copy(),
            }
            self._append_training_row(self._tr_event, event_row)

            bp_truth = self._extract_mc_truth_scalars(best_bp)
            b0_truth = self._extract_mc_truth_scalars(best_b0)
            bp_row = {name: bp_truth[name] for name in self.TR_BEST_FIELDS}
            b0_row = {name: b0_truth[name] for name in self.TR_BEST_FIELDS}
            self._append_training_row(self._tr_bp, bp_row)
            self._append_training_row(self._tr_b0, b0_row)

            bp_threshold = config.N_BP_MODES * 2
            best_bp_iid = -1
            best_bp_dp = np.inf
            best_b0_iid = -1
            best_b0_dp = np.inf

            for iid, particle in particle_by_input_id.items():
                cand_truth = self._extract_mc_truth_scalars(particle)
                is_cont = cand_truth['is_cont']
                tag_pdg = cand_truth['tag_pdg']
                pdg = cand_truth['pdg']
                dp = cand_truth['dp']
                dm_id = cand_truth['dm']
                if np.isnan(is_cont) or np.isnan(tag_pdg) or np.isnan(pdg) or np.isnan(dp) or np.isnan(dm_id):
                    continue
                if is_cont == 1.0 or not config.truth_tag_matches_pdg(pdg, tag_pdg, dm_id):
                    continue

                if int(iid) < bp_threshold:
                    if (best_bp_iid < 0) or (dp < best_bp_dp):
                        best_bp_iid = int(iid)
                        best_bp_dp = dp
                else:
                    if (best_b0_iid < 0) or (dp < best_b0_dp):
                        best_b0_iid = int(iid)
                        best_b0_dp = dp

            sig_rows = []
            for iid, particle in particle_by_input_id.items():
                is_signal = vm.evaluate('isSignal', particle)
                if not np.isnan(is_signal) and int(is_signal) == 1:
                    btag_index = vm.evaluate('mostcommonBTagIndex', particle)
                    delta_p = vm.evaluate('mostcommonBTagDeltaP', particle)
                    sig_prob = vm.evaluate('extraInfo(SignalProbability)', particle)
                    btag_index_i = -1 if np.isnan(btag_index) else int(btag_index)
                    delta_p_f = np.inf if np.isnan(delta_p) else float(delta_p)
                    sig_prob_f = -1.0 if np.isnan(sig_prob) else float(sig_prob)
                    sig_rows.append((int(iid), btag_index_i, delta_p_f, sig_prob_f))
            sig_rows.sort(key=lambda row: row[0])
            self._tr_sig_input_ids.append(np.asarray([row[0] for row in sig_rows], dtype=np.int16))
            self._tr_sig_btag_index.append(np.asarray([row[1] for row in sig_rows], dtype=np.int16))
            self._tr_sig_delta_p.append(np.asarray([row[2] for row in sig_rows], dtype=np.float32))
            self._tr_sig_sigprob.append(np.asarray([row[3] for row in sig_rows], dtype=np.float32))

            event_best_row = {
                'best_bp_iid': best_bp_iid,
                'best_bp_dp': best_bp_dp,
                'best_b0_iid': best_b0_iid,
                'best_b0_dp': best_b0_dp,
            }
            self._append_training_row(self._tr_event_best, event_best_row)
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

        if self.skip_nn_evaluation:
            cat_output, main_output = self._get_placeholder_outputs(features)
            charged_cat = 1.0 if cat_output[1] > cat_output[0] else 0.0
        else:
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
        self._inference_event_count += 1

        # Extract scores
        # Main network outputs (139 classes after softmax):
        #   [0..N_INPUT_IDS-1] = P(true mode is that input_id); class index = input_id
        #   [N_INPUT_IDS+0] = bad_tag
        #   [N_INPUT_IDS+1] = cross_deltaC1
        #   [N_INPUT_IDS+2] = continuum
        bp_threshold = config.N_BP_MODES * 2
        sign = 1.0 if charged_cat else -1.0
        predicted_is_bp = bool(charged_cat)
        predicted_input_ids = [
            iid for iid in particle_by_input_id
            if (iid < bp_threshold) == predicted_is_bp
        ]
        if charged_cat:
            predicted_top_iid = int(np.argmax(main_output[:bp_threshold]))
        else:
            predicted_top_iid = int(np.argmax(main_output[bp_threshold:config.N_INPUT_IDS])) + bp_threshold
        has_predicted_candidate = bool(predicted_input_ids)
        if has_predicted_candidate and predicted_top_iid not in particle_by_input_id:
            self._missing_top_mode_count += 1
        if predicted_input_ids:
            max_mode_prob = max(float(main_output[iid]) for iid in predicted_input_ids)
        else:
            # fallback to the sum of predicted-sector mode outputs + bad_tag
            # transforming to 2 * (sum_mode_prob - 0.5) if sum_mode_prob > 0.5, else 0.0
            # this is to ensure low values for events when the overall predicted-sector confidence is low
            bad_tag_prob = float(main_output[config.N_INPUT_IDS])
            if charged_cat:
                sum_mode_prob = np.sum(main_output[:bp_threshold]) + bad_tag_prob
            else:
                sum_mode_prob = np.sum(main_output[bp_threshold:config.N_INPUT_IDS]) + bad_tag_prob
            max_mode_prob = np.clip(2 * (sum_mode_prob - 0.5), 0.0, None)
            self._empty_predicted_sector_count += 1
        bp_score = sign * max_mode_prob
        is_high_conf = abs(bp_score) > config.HIGH_CONF_BPLUSSCORE_ABS
        if is_high_conf:
            self._high_conf_event_count += 1
        if has_predicted_candidate and predicted_top_iid not in particle_by_input_id and is_high_conf:
            self._missing_top_mode_high_conf_count += 1
        if not predicted_input_ids and is_high_conf:
            self._empty_predicted_sector_high_conf_count += 1

        # Assign modeSelector_eqSigProb per candidate in the predicted sector only.
        # Non-predicted sector candidates are left unset (sigProb-based ranking used downstream).
        predicted_rank_pairs = []
        non_predicted_rank_pairs = []
        for input_id, particle in particle_by_input_id.items():
            is_bp_sector = input_id < bp_threshold
            if bool(charged_cat) == is_bp_sector:
                candidate_score = float(main_output[input_id])
                particle.addExtraInfo(f'{self.AUXILIARY_OUTPUT_PREFIX}_eqSigProb', candidate_score)
                predicted_rank_pairs.append((particle, candidate_score, input_id))
            else:
                non_predicted_rank_pairs.append((particle, float(_sigprob_by_input_id[input_id]), input_id))

        self._assign_rank_extra_info(predicted_rank_pairs, f'{self.AUXILIARY_OUTPUT_PREFIX}_rank')
        self._assign_rank_extra_info(non_predicted_rank_pairs, f'{self.AUXILIARY_OUTPUT_PREFIX}_rank')

        # Store event-level outputs in EventExtraInfo
        event_extra_info = Belle2.PyStoreObj('EventExtraInfo')
        if not event_extra_info.isValid():
            event_extra_info.create()
        event_extra_info.addExtraInfo(self.output_variable, bp_score)
        event_extra_info.addExtraInfo(
            'modeSelector_feiCalibWeight', self._compute_fei_calib_weight(best_bp, best_b0)
        )
        event_extra_info.addExtraInfo(f'{self.AUXILIARY_OUTPUT_PREFIX}_catB0', float(cat_output[0]))
        event_extra_info.addExtraInfo(f'{self.AUXILIARY_OUTPUT_PREFIX}_catBp', float(cat_output[1]))
        event_extra_info.addExtraInfo(f'{self.AUXILIARY_OUTPUT_PREFIX}_catCont', float(cat_output[2]))

        # Debug output (after NN inference so we can save outputs too)
        if self.debug:
            if self.event_count < self.debug_max_events:
                self._print_debug_info(candidates_data, event_features, all_features, max_input_id)

            self.event_count += 1
