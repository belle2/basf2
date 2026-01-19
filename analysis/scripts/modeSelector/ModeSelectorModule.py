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
        has_inputs_path=None,
        output_variable='BplusScore',
        store_event_info=True,
        payload_cat_model='ModeSelector_cat_model',
        payload_main_model='ModeSelector_main_model',
        payload_has_inputs='ModeSelector_has_inputs',
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
        #: Path to has_inputs file
        self.has_inputs_path = has_inputs_path
        #: Output variable name
        self.output_variable = output_variable
        #: Store event-level info
        self.store_event_info = store_event_info
        #: Payload name for category model
        self.payload_cat_model = payload_cat_model
        #: Payload name for main model
        self.payload_main_model = payload_main_model
        #: Payload name for has_inputs
        self.payload_has_inputs = payload_has_inputs
        #: Debug mode
        self.debug = debug
        #: Max events to debug
        self.debug_max_events = debug_max_events
        #: Event counter for debug
        self.event_count = 0
        #: Store debug features for comparison
        self.debug_features = []

        # Feature configuration
        #: Number of decay mode indices (dmID * 2 + is_charged)
        self.n_input_ids = 136
        #: Number of feature blocks (sigProb, chiProb, etc.)
        self.n_feature_blocks = 11  # Mbc is excluded
        #: Event-level feature names
        self.event_features = [
            'sphericity', 'thrust', 'thrustAxisCosTheta', 'aplanarity', 'foxWolframR2',
            'harmonicMomentThrust0', 'harmonicMomentThrust1', 'harmonicMomentThrust2'
        ]

    def initialize(self):
        """Called at the beginning of processing."""
        import ast

        import onnxruntime as ort

        # Load has_inputs (feature indices) from file or database
        if self.has_inputs_path:
            with open(self.has_inputs_path, 'r') as f:
                self.has_inputs = ast.literal_eval(f.read())
        else:
            try:
                db_accessor = Belle2.DBAccessorBase(
                    Belle2.DBStoreEntry.c_RawFile, self.payload_has_inputs, True
                )
                with open(db_accessor.getFilename(), 'r') as f:
                    self.has_inputs = ast.literal_eval(f.read())
            except Exception as e:
                b2.B2WARNING(f"ModeSelector: Could not load has_inputs from database: {e}")
                b2.B2WARNING("ModeSelector: Using all features (this may not match training!)")
                self.has_inputs = None

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

        # Build feature index mapping
        # The training uses a specific set of feature indices (has_inputs)
        # We need to compute features for all positions and select the right ones
        self._build_feature_indices()

    def _build_feature_indices(self):
        """
        Build the feature index mapping.

        The training used sparse matrices with specific non-zero columns.
        We need to match that structure.
        """
        # Feature blocks (in order):
        # 0: sigProb (1.0 + val * 100)
        # 1: chiProb (2.0 + val)
        # 2: Bdaughter_sigProb (1.0 + val * 10)
        # 3: Bdaughter2_sigProb (1.0 + val * 10)
        # 4: Bdaughter_chiProb (2.0 + val)
        # 5: Dst0_deltaMassDiff (1.0 + val * 20 + 0.5)
        # 6: Dstp_deltaMassDiff (1.0 + val * 20 + 0.5)
        # 7: Dst0_chiProb (2.0 + val)
        # 8: Dstp_chiProb (2.0 + val)
        # 9: deltaE (1.0 + val * 5 + 0.75)
        # 10: Mbc - EXCLUDED from training
        # 11: cosTBTO (1.0 + val)

        #: Feature block names and transformations
        # Note: Mbc is included to match training structure (excluded via has_inputs)
        self.feature_blocks = [
            ('sigProb', lambda x: 1.0 + x * 100),
            ('chiProb', lambda x: 2.0 + x),
            ('Bdaughter_sigProb', lambda x: 1.0 + x * 10),
            ('Bdaughter2_sigProb', lambda x: 1.0 + x * 10),
            ('Bdaughter_chiProb', lambda x: 2.0 + x),
            ('Dst0_deltaMassDiff', lambda x: 1.0 + x * 20 + 0.5),
            ('Dstp_deltaMassDiff', lambda x: 1.0 + x * 20 + 0.5),
            ('Dst0_chiProb', lambda x: 2.0 + x),
            ('Dstp_chiProb', lambda x: 2.0 + x),
            ('deltaE', lambda x: 1.0 + x * 5 + 0.75),
            ('Mbc', lambda x: 1.0 + (x - 5.23) * 20),  # Included for index compatibility
            ('cosTBTO', lambda x: 1.0 + x),
        ]

        # Variable names to extract from particles
        #: Variable mapping for particle features
        self.particle_vars = {
            'sigProb': 'extraInfo(SignalProbability)',
            'chiProb': 'chiProb',
            'Bdaughter_sigProb': 'daughter(0, extraInfo(SignalProbability))',
            'Bdaughter2_sigProb': 'daughter(1, extraInfo(SignalProbability))',
            'Bdaughter_chiProb': 'daughter(0, chiProb)',
            'Dst0_deltaMassDiff': 'extraInfo(Dst0_deltaMassDiff)',
            'Dstp_deltaMassDiff': 'extraInfo(Dstp_deltaMassDiff)',
            'Dst0_chiProb': 'extraInfo(Dst0_chiProb)',
            'Dstp_chiProb': 'extraInfo(Dstp_chiProb)',
            'deltaE': 'deltaE',
            'Mbc': 'Mbc',  # Included for index compatibility (excluded via has_inputs)
            'cosTBTO': 'cosTBTO',  # From buildContinuumSuppression
        }

        # D* deltaMassDiff cut
        #: Cut range for D* delta mass difference
        self.deltaM_cut = (-0.05, 0.05)

    def _get_input_id(self, particle):
        """
        Compute input_id from decay mode ID and B charge.

        input_id = dmID * 2 + is_charged
        """
        dm_id = int(vm.evaluate('extraInfo(decayModeID)', particle))
        pdg = int(vm.evaluate('PDG', particle))
        is_charged = 1 if abs(pdg) == 521 else 0
        return dm_id * 2 + is_charged

    def _extract_particle_features(self, particle):
        """
        Extract features from a single particle.

        Returns:
            tuple: (input_id, feature_dict)
        """
        input_id = self._get_input_id(particle)

        features = {}
        for name, var_name in self.particle_vars.items():
            val = vm.evaluate(var_name, particle)
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
        # We exclude Mbc (block 10), so we have 11 blocks
        n_blocks = len(self.feature_blocks)
        feature_matrix = np.zeros((n_blocks, self.n_input_ids), dtype=np.float32)

        # Fill in per-candidate features
        for input_id, features in candidates_data:
            if input_id < 0 or input_id >= self.n_input_ids:
                continue

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

        # Add ncandidates / 10
        n_candidates = len(candidates_data)
        event_feat_values.append(n_candidates / 10.0)

        # Find best candidate (highest sigProb) and compute input_ids
        if candidates_data:
            sig_probs = []
            for input_id, features in candidates_data:
                sp = features.get('sigProb')
                sig_probs.append(sp if sp is not None else -1)

            # Best candidate overall
            best_idx = np.argmax(sig_probs)
            max_input_id = candidates_data[best_idx][0]

            # Best neutral (B0) and charged (B+) candidates
            neutral_ids = [(i, inp_id, sp) for i, (inp_id, features) in enumerate(candidates_data)
                           for sp in [features.get('sigProb', -1)]
                           if inp_id % 2 == 0]  # Even input_id = neutral
            charged_ids = [(i, inp_id, sp) for i, (inp_id, features) in enumerate(candidates_data)
                           for sp in [features.get('sigProb', -1)]
                           if inp_id % 2 == 1]  # Odd input_id = charged

            # Second best from other B type
            best_pdg_is_charged = max_input_id % 2 == 1
            if best_pdg_is_charged and neutral_ids:
                scnd_max_input_id = max(neutral_ids, key=lambda x: x[2])[1]
            elif not best_pdg_is_charged and charged_ids:
                scnd_max_input_id = max(charged_ids, key=lambda x: x[2])[1]
            else:
                scnd_max_input_id = -10
        else:
            max_input_id = 0
            scnd_max_input_id = -10

        event_feat_values.append(max_input_id / 50.0)
        event_feat_values.append(scnd_max_input_id / 50.0)

        # Concatenate all features
        all_features = np.concatenate([flat_features, np.array(event_feat_values, dtype=np.float32)])

        return all_features, max_input_id

    def _get_event_features(self):
        """Extract event-level features from EventShapeContainer."""
        event_features = {}

        # Event shape variables are stored in EventShapeContainer
        event_shape = Belle2.PyStoreObj('EventShapeContainer')
        if event_shape.isValid():
            obj = event_shape.obj()
            # Map feature names to EventShapeContainer methods
            if 'sphericity' in self.event_features:
                event_features['sphericity'] = obj.getSphericityEigenvalue(0)
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

    def _print_debug_info(self, candidates_data, event_features, all_features, max_input_id):
        """Print debug information for comparing with offline preprocessing."""
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
        event_feat_names = self.event_features + ['ncandidates/10', 'max_input_id/50', 'scnd_max_input_id/50']
        for i, name in enumerate(event_feat_names):
            idx = event_feat_start + i
            if idx < len(all_features):
                print(f"  {name}: {all_features[idx]}")

        print(f"\n--- Total feature array shape: {len(all_features)} ---")

        # Store for later comparison
        self.debug_features.append({
            'event_id_str': event_id_str,
            'all_features': all_features.copy(),
            'max_input_id': max_input_id,
            'n_candidates': len(candidates_data),
            'candidates_data': candidates_data,
        })

    def terminate(self):
        """Called at the end of processing."""
        if self.debug and self.debug_features:
            # Save debug features to file
            debug_file = 'modeSelector_debug_features.npz'
            feature_arrays = np.array([d['all_features'] for d in self.debug_features])
            np.savez(debug_file,
                     features=feature_arrays,
                     n_candidates=[d['n_candidates'] for d in self.debug_features],
                     max_input_ids=[d['max_input_id'] for d in self.debug_features])
            print(f"\n[DEBUG] Saved {len(self.debug_features)} events to {debug_file}")

    def event(self):
        """Called for each event."""
        # Collect all candidates from all particle lists
        all_candidates = []
        candidates_data = []

        for list_name in self.particle_lists:
            plist = Belle2.PyStoreObj(list_name)
            if not plist.isValid():
                continue

            for i in range(plist.getListSize()):
                particle = plist.obj().getParticle(i)
                all_candidates.append(particle)

                input_id, features = self._extract_particle_features(particle)
                candidates_data.append((input_id, features))

        if not all_candidates:
            return

        # Get event-level features
        event_features = self._get_event_features()

        # Build feature array
        all_features, max_input_id = self._build_feature_array(candidates_data, event_features)

        # Debug output
        if self.debug and self.event_count < self.debug_max_events:
            self._print_debug_info(candidates_data, event_features, all_features, max_input_id)
            self.event_count += 1

        # Select features based on has_inputs (indices used during training)
        if self.has_inputs is not None:
            features = all_features[self.has_inputs]
        else:
            features = all_features

        # Verify feature size matches model expectation
        if len(features) != self.cat_input_size:
            b2.B2WARNING(f"Feature size mismatch: got {len(features)}, expected {self.cat_input_size}")
            # Pad or truncate as needed
            if len(features) < self.cat_input_size:
                features = np.pad(features, (0, self.cat_input_size - len(features)))
            else:
                features = features[:self.cat_input_size]

        # Run category network
        cat_input = features.reshape(1, -1).astype(np.float32)
        cat_output = self.cat_session.run(None, {self.cat_input_name: cat_input})[0][0]

        # Determine predicted category (0=B0, 1=B+, 2=continuum)
        charged_cat = 1.0 if cat_output[1] > cat_output[0] else 0.0

        # Build main network input (features + cat_output + charged_cat)
        main_features = np.concatenate([features, cat_output, [charged_cat]])

        if len(main_features) != self.main_input_size:
            b2.B2WARNING(f"Main feature size mismatch: got {len(main_features)}, expected {self.main_input_size}")
            if len(main_features) < self.main_input_size:
                main_features = np.pad(main_features, (0, self.main_input_size - len(main_features)))
            else:
                main_features = main_features[:self.main_input_size]

        main_input = main_features.reshape(1, -1).astype(np.float32)
        main_output = self.main_session.run(None, {self.main_input_name: main_input})[0][0]

        # Extract scores
        # Main network outputs: signal probability for different categories
        # Index 1 = is_target for neutral, Index 5 = is_target for charged
        if charged_cat > 0.5:
            bplus_score = main_output[5] if len(main_output) > 5 else main_output[1]
        else:
            bplus_score = main_output[1]

        # Store results on the best candidate
        best_candidate = None
        best_sig_prob = -1
        for particle in all_candidates:
            sig_prob = vm.evaluate('extraInfo(SignalProbability)', particle)
            if sig_prob > best_sig_prob:
                best_sig_prob = sig_prob
                best_candidate = particle

        if best_candidate is not None:
            best_candidate.addExtraInfo(self.output_variable, float(bplus_score))
            best_candidate.addExtraInfo(f'{self.output_variable}_catB0', float(cat_output[0]))
            best_candidate.addExtraInfo(f'{self.output_variable}_catBp', float(cat_output[1]))
            best_candidate.addExtraInfo(f'{self.output_variable}_catCont', float(cat_output[2]))

        # Optionally store event-level info
        if self.store_event_info:
            event_extra_info = Belle2.PyStoreObj('EventExtraInfo')
            if not event_extra_info.isValid():
                event_extra_info.create()
            event_extra_info.addExtraInfo(self.output_variable, float(bplus_score))
