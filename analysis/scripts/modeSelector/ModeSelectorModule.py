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
to compute BplusScore, an event-level score based on all candidates.

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

    This module processes FEI B meson candidates and computes a score using
    information from all candidates in the event.

    Args:
        particle_lists (list): List of B meson particle list names
        cat_model_path (str): Path to the basf2 MVA weightfile for the category network,
            as produced by convert_to_onnx.py. Pass None to load from the conditions
            database via payload_cat_model.
        main_model_path (str): Path to the basf2 MVA weightfile for the main network,
            as produced by convert_to_onnx.py. Pass None to load from the conditions
            database via payload_main_model.
        payload_cat_model (str): Conditions DB payload name for the category model. None
            (default) uses the name derived from the contract version this release
            implements, which is the normal case.
        payload_main_model (str): Same for the main model.
        output_variable (str): Name of ExtraInfo variable for output score
        training_mode (bool): Expose features and MC truth for training instead of running
            the networks.
        skip_nn_evaluation (bool): Fill placeholder outputs instead of running the networks
            (debugging and timing only).
        store_fei_calib_weight (bool): Store modeSelector_feiCalibWeight (MC only).
        debug (bool): Print the feature vector and network outputs for the first events.
        debug_max_events (int): Number of events printed when debug is True.

    See modeSelector.modeSelector() for the full description of each parameter.
    """
    #: Prefix used for auxiliary ExtraInfo output variables
    AUXILIARY_OUTPUT_PREFIX = 'modeSelector'

    def __init__(
        self,
        particle_lists,
        payload_cat_model=None,
        payload_main_model=None,
        output_variable='BplusScore',
        cat_model_path=None,
        main_model_path=None,
        training_mode=False,
        skip_nn_evaluation=False,
        store_fei_calib_weight=False,
        debug=False,
        debug_max_events=10,
    ):
        """Initialise module parameters and training data buffers."""
        super().__init__()
        #: Input particle lists
        self.particle_lists = particle_lists if isinstance(particle_lists, list) else [particle_lists]
        #: Path to category model
        self.cat_model_path = cat_model_path
        #: Path to main model
        self.main_model_path = main_model_path
        #: Output variable name
        self.output_variable = output_variable
        # None means: use the name derived from the contract version this release implements
        #: True if the category payload name was given explicitly instead of derived
        self.payload_cat_model_given = payload_cat_model is not None
        #: True if the main payload name was given explicitly instead of derived
        self.payload_main_model_given = payload_main_model is not None
        #: Payload name for category model
        self.payload_cat_model = payload_cat_model if payload_cat_model is not None else config.DEFAULT_CAT_PAYLOAD
        #: Payload name for main model
        self.payload_main_model = payload_main_model if payload_main_model is not None else config.DEFAULT_MAIN_PAYLOAD
        #: Contract version the loaded models were built for. Set in initialize() from the
        #: weightfiles. Everything that differs between supported contract versions (feature
        #: construction, expected output classes, output interpretation) branches on this, so a
        #: model for an older supported contract runs with that contract's behaviour.
        self.contract_version = config.MODEL_CONTRACT_VERSION
        #: Training mode (save features + MC truth, skip NN inference)
        self.training_mode = training_mode
        #: Skip NN inference and fill deterministic placeholder outputs
        self.skip_nn_evaluation = skip_nn_evaluation
        #: Store modeSelector_feiCalibWeight in EventExtraInfo (MC only)
        self.store_fei_calib_weight = store_fei_calib_weight
        #: Flag to emit the gen-calib-weight missing warning at most once
        self._gen_calib_weight_missing_warned = False
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
        #: Number of candidates checked for preselection compliance
        self._presel_total_candidates = 0
        #: Number of candidates failing at least one preselection cut
        self._presel_violation_count = 0
        #: Number of inference events where unsupported experiment ids were replaced
        self._unsupported_experiment_event_count = 0
        #: Counts of unsupported raw experiment ids seen during inference
        self._unsupported_experiment_counts = {}

        # Feature configuration (from modeSelector.config)
        #: Number of input_id slots (B+ sector + B0 sector, each split by particle/antiparticle)
        self.n_input_ids = config.N_INPUT_IDS
        #: Event-level feature names
        self.event_features = config.EVENT_FEATURES

    def _load_weightfile(self, path, label):
        """Load a basf2 MVA weightfile, raises error for unsupported extensions."""
        if not path.endswith('.root'):
            b2.B2FATAL(
                "ModeSelector: " + label + " model path '" + path + "' is not a basf2 MVA "
                "weightfile. Pass the file produced by convert_to_onnx.py "
                "(must have a .root extension), not a raw ONNX file."
            )
        return Belle2.MVA.Weightfile.loadFromFile(path)

    def _feature_selection_from_weightfile(self, variables, label):
        """Recover the raw feature indices a model was trained on from its weightfile.

        Returns None for legacy weightfiles that only carry placeholder variable names,
        in which case the caller falls back to config.HAS_INPUTS.
        """
        prefix = config.FEATURE_VAR_PREFIX
        indices = []
        for name in variables:
            name = str(name)
            if not name.startswith(prefix):
                continue
            try:
                indices.append(int(name[len(prefix):]))
            except ValueError:
                b2.B2FATAL(
                    "ModeSelector: " + label + " model weightfile has a malformed feature "
                    "name '" + name + "'. Re-export the model with convert_to_onnx.py."
                )
        return indices if indices else None

    def _check_contract_is_self_consistent(self):
        """Check that SUPPORTED_CONTRACT_VERSIONS is consistent with the current contract version.

        A mismatch is a developer error in the release, not a problem with the payload.
        The same rule is applied by convert_to_onnx.py before exporting.
        """
        error = config.contract_consistency_error()
        if error:
            b2.B2FATAL("ModeSelector: " + error)

    def _check_contract(self, weightfile, identifier, label):
        """Check a model's contract version and return it together with the training name.

        The contract version is an extra element of the weightfile; the identifier holds the
        training name. The contract version must be one this release supports: the current
        one, or an older one whose code path the release still provides. A newer version, or
        an older one that is no longer supported, would run without error and give wrong
        results, so it is fatal. Weightfiles written before the contract version was recorded
        are treated as contract version 1 and are subject to the same support rule.
        """
        training = str(identifier)
        if not weightfile.hasContractVersion():
            b2.B2WARNING(
                "ModeSelector: " + label + " model weightfile carries no contract version, "
                "so it is treated as contract version 1. Re-export it with convert_to_onnx.py."
            )
            # Still subject to the support rule: without it, a release that dropped contract 1
            # would run such a model with the wrong code path.
            self._require_supported_contract(1, label)
            return 1, training

        # getContractVersion() also returns the default if the stored value is not an
        # integer, which hasContractVersion() above has already ruled out as 'not stored'.
        version = weightfile.getContractVersion(-1)
        if version < 0:
            b2.B2FATAL(
                "ModeSelector: " + label + " model weightfile has a malformed contract "
                "version. Re-export it with convert_to_onnx.py."
            )

        self._require_supported_contract(version, label)
        b2.B2INFO(
            "ModeSelector: " + label + " model training '" + training
            + "' (contract v" + str(version) + ")"
        )
        return version, training

    def _require_supported_contract(self, version, label):
        """Stop unless this release can run models built for the given contract version."""
        if version in config.SUPPORTED_CONTRACT_VERSIONS:
            return
        current = config.MODEL_CONTRACT_VERSION
        if version > current:
            b2.B2FATAL(
                "ModeSelector: " + label + " model was built for contract version " + str(version)
                + ", newer than contract version " + str(current) + " implemented by this release. "
                "Use a newer release for this payload."
            )
        b2.B2FATAL(
            "ModeSelector: " + label + " model was built for contract version " + str(version)
            + ", which this release no longer supports (supported: "
            + ", ".join(str(v) for v in sorted(config.SUPPORTED_CONTRACT_VERSIONS))
            + "). Use an older release for this payload."
        )

    def _select_contract_version(self, cat_version, main_version):
        """Return the contract version to run with, and warn when it is older than the current one.

        Both networks are evaluated by one code path, so they must share a contract version.
        """
        if cat_version != main_version:
            b2.B2FATAL(
                "ModeSelector: the category model was built for contract version " + str(cat_version)
                + " but the main model for contract version " + str(main_version)
                + ". Both must come from the same contract."
            )
        current = config.MODEL_CONTRACT_VERSION
        if cat_version < current:
            b2.B2WARNING(
                "ModeSelector: the models were built for contract version " + str(cat_version)
                + ", older than contract version " + str(current) + " implemented by this release. "
                "The module falls back to the behaviour of contract version " + str(cat_version)
                + ", which this release still supports. See the ModeSelector performance "
                "recommendations for what changed in contract version " + str(current)
                + " and whether using its models is worthwhile."
            )
        return cat_version

    def _check_same_training(self, cat_training, main_training):
        """Check that the category and main models come from the same training.

        The main network takes the category network outputs as inputs, so a pair from
        different trainings runs without error but gives wrong results. Such a pair shares
        the contract version, so only the training name in the weightfile
        identifier, which convert_to_onnx.py writes identically into both weightfiles, can
        tell them apart.
        """
        if cat_training != main_training:
            b2.B2FATAL(
                "ModeSelector: the category model comes from training '" + cat_training
                + "' but the main model from training '" + main_training + "'. The main network "
                "takes the category outputs as inputs, so both must come from the same training. "
                "Check that the two payloads were exported and uploaded together."
            )

    def _check_output_classes(self, options, expected, label):
        """Check the model's output size against what the module's interpretation assumes.

        The outputs are read positionally -- for the main network the class index is the
        input_id and the last three entries are the background classes -- so a model with a
        different number of outputs would be misread rather than rejected.
        """
        n_classes = int(options.m_nClasses)
        if n_classes != expected:
            b2.B2FATAL(
                "ModeSelector: " + label + " model produces " + str(n_classes) + " output "
                "classes but this software interprets " + str(expected) + ". The payload does "
                "not match this release."
            )

    def _warn_if_newer_contract_available(self):
        """Warn when the configured globaltags also serve payloads for a newer contract version.

        Payload names are derived from the contract version, so a single performance
        globaltag can hold models for several releases at once. Finding a newer one means a
        newer release would use a different, possibly improved model. Looking up a payload
        that does not exist only queries the metadata; the file of a newer payload is
        fetched only if it is actually present.
        """
        # Contract versions are bumped rarely, and a globaltag may drop payloads for versions
        # no supported release uses, so look a few versions ahead instead of stopping at the
        # first gap.
        probe_range = 10
        current = config.MODEL_CONTRACT_VERSION
        newest = None
        for version in range(current + 1, current + 1 + probe_range):
            name = config.payload_names(version)[0]
            accessor = Belle2.DBAccessorBase(Belle2.DBStoreEntry.c_RawFile, name, False)
            if accessor.getFilename():
                newest = (version, name)
        if newest is not None:
            b2.B2WARNING(
                "ModeSelector: the configured globaltags also provide payloads for contract version "
                + str(newest[0]) + " ('" + newest[1] + "'), which newer releases use. This release "
                "implements contract version " + str(current) + ". See the ModeSelector performance "
                "recommendations for what changed and whether moving to a newer release is worthwhile."
            )

    def initialize(self):
        """Called at the beginning of processing."""
        # Build feature index mapping (needed for both inference and training)
        self._build_feature_indices()

        if self.training_mode:
            b2.B2INFO("ModeSelector: Running in TRAINING mode (saving features, no NN inference)")
            #: Indices of non-zero features kept after sparsity filtering (None in training mode)
            self.has_inputs = None

            bp_calib_map = config.get_fei_calibration_map(521)
            #: FEI calibration fallback factor for the B+ sector
            self._bp_calib_rest = config.get_fei_calibration_rest(521)
            #: Per-decay-mode FEI calibration lookup for the B+ sector
            self._bp_calib_lookup = np.full(config.N_BP_MODES, self._bp_calib_rest, dtype=np.float32)
            for _dm, _w in bp_calib_map.items():
                if 0 <= _dm < config.N_BP_MODES:
                    self._bp_calib_lookup[_dm] = _w

            b0_calib_map = config.get_fei_calibration_map(511)
            #: FEI calibration fallback factor for the B0 sector
            self._b0_calib_rest = config.get_fei_calibration_rest(511)
            #: Per-decay-mode FEI calibration lookup for the B0 sector
            self._b0_calib_lookup = np.full(config.N_B0_MODES, self._b0_calib_rest, dtype=np.float32)
            for _dm, _w in b0_calib_map.items():
                if 0 <= _dm < config.N_B0_MODES:
                    self._b0_calib_lookup[_dm] = _w
            return

        if self.skip_nn_evaluation:
            self.has_inputs = list(config.HAS_INPUTS)
            #: Category network input size (number of selected features)
            self.cat_input_size = len(self.has_inputs)
            #: Main network input size (cat features + cat output + charged flag)
            self.main_input_size = self.cat_input_size + 4
            b2.B2INFO("ModeSelector: Running with NN evaluation disabled")
            b2.B2INFO(
                f"ModeSelector: Using placeholder outputs with {self.cat_input_size} selected features"
            )
            return

        import basf2_mva

        # A non-default payload name is the wrong way to pick a training: the names are
        # deliberately version-free so that the globaltag decides which models are served.
        # Only warn for names that are actually used, i.e. not overridden by a local file.
        # An explicitly given payload name bypasses the name derived from the contract version.
        # Local weightfiles take precedence, so names they override are not reported.
        explicit = []
        if not self.cat_model_path and self.payload_cat_model_given:
            explicit.append(self.payload_cat_model + " (default " + config.DEFAULT_CAT_PAYLOAD + ")")
        if not self.main_model_path and self.payload_main_model_given:
            explicit.append(self.payload_main_model + " (default " + config.DEFAULT_MAIN_PAYLOAD + ")")
        if explicit:
            b2.B2WARNING(
                "ModeSelector: not using the default payloads, explicitly requested: " + ", ".join(explicit)
                + ". Normally the payload names are left unset, so they are derived from the contract"
                " version this release implements, and the training is selected by prepending the"
                " performance globaltag that serves it."
            )

        # Load models through the basf2 MVA Expert framework.
        # Single-threaded ONNX execution is enforced by the framework in mva/methods/src/ONNX.cc.
        Belle2.MVA.AbstractInterface.initSupportedInterfaces()
        supported = Belle2.MVA.AbstractInterface.getSupportedInterfaces()

        if self.cat_model_path:
            cat_wf = self._load_weightfile(self.cat_model_path, 'category')
        else:
            db_accessor = Belle2.DBAccessorBase(
                Belle2.DBStoreEntry.c_RawFile, self.payload_cat_model, True
            )
            cat_filename = db_accessor.getFilename()
            if not cat_filename:
                b2.B2FATAL(
                    "ModeSelector: category model payload '"
                    + self.payload_cat_model
                    + "' not found in the conditions database. This release implements "
                    "contract version " + str(config.MODEL_CONTRACT_VERSION) + ", so the "
                    "configured performance globaltag must provide payloads for it."
                )
            cat_wf = Belle2.MVA.Weightfile.loadFromFile(cat_filename)

        if self.main_model_path:
            main_wf = self._load_weightfile(self.main_model_path, 'main')
        else:
            db_accessor = Belle2.DBAccessorBase(
                Belle2.DBStoreEntry.c_RawFile, self.payload_main_model, True
            )
            main_filename = db_accessor.getFilename()
            if not main_filename:
                b2.B2FATAL(
                    "ModeSelector: main model payload '"
                    + self.payload_main_model
                    + "' not found in the conditions database. This release implements "
                    "contract version " + str(config.MODEL_CONTRACT_VERSION) + ", so the "
                    "configured performance globaltag must provide payloads for it."
                )
            main_wf = Belle2.MVA.Weightfile.loadFromFile(main_filename)

        #: Category network expert
        self.cat_expert = supported["ONNX"].getExpert()
        self.cat_expert.load(cat_wf)
        #: Main network expert
        self.main_expert = supported["ONNX"].getExpert()
        self.main_expert.load(main_wf)

        cat_opts = basf2_mva.GeneralOptions()
        cat_wf.getOptions(cat_opts)
        main_opts = basf2_mva.GeneralOptions()
        main_wf.getOptions(main_opts)

        self._check_contract_is_self_consistent()
        cat_version, cat_training = self._check_contract(cat_wf, cat_opts.m_identifier, 'category')
        main_version, main_training = self._check_contract(main_wf, main_opts.m_identifier, 'main')
        self._check_same_training(cat_training, main_training)
        self.contract_version = self._select_contract_version(cat_version, main_version)
        self._check_output_classes(cat_opts, config.NUM_CAT_LABELS, 'category')
        self._check_output_classes(main_opts, config.N_INPUT_IDS + 3, 'main')
        if not self.cat_model_path:
            self._warn_if_newer_contract_available()

        # The payload records which raw feature indices its model was trained on, so a
        # retraining can change the selection without a software release.
        cat_selection = self._feature_selection_from_weightfile(cat_opts.m_variables, 'category')
        main_selection = self._feature_selection_from_weightfile(main_opts.m_variables, 'main')
        if cat_selection is None or main_selection is None:
            b2.B2WARNING(
                "ModeSelector: weightfile does not record its feature selection, falling back "
                "to config.HAS_INPUTS. This only works if the payload was trained against the "
                "same selection. Re-export the model with convert_to_onnx.py."
            )
            self.has_inputs = list(config.HAS_INPUTS)
            selection_source = 'config.HAS_INPUTS'
        elif cat_selection != main_selection:
            b2.B2FATAL(
                "ModeSelector: the category and main models were trained on different feature "
                "selections, so they come from different trainings and must not be combined."
            )
        else:
            self.has_inputs = cat_selection
            selection_source = 'the weightfile'

        # Input sizes derived from the weightfile variable list
        self.cat_input_size = len(cat_opts.m_variables)
        self.main_input_size = len(main_opts.m_variables)

        # Pre-allocate datasets; m_input elements are overwritten per event
        #: SingleDataset for category network inference
        self.cat_dataset = Belle2.MVA.SingleDataset(cat_opts, [0.0] * self.cat_input_size, 1.0)
        #: SingleDataset for main network inference
        self.main_dataset = Belle2.MVA.SingleDataset(main_opts, [0.0] * self.main_input_size, 1.0)

        b2.B2INFO(f"ModeSelector: Loaded category model (input size: {self.cat_input_size})")
        b2.B2INFO(f"ModeSelector: Loaded main model (input size: {self.main_input_size})")
        if self.has_inputs:
            b2.B2INFO(f"ModeSelector: Using {len(self.has_inputs)} selected features from {selection_source}")

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

        Parameters:
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
        experiment = self._get_inference_experiment_feature_value()
        event_feat_values.append(experiment / 10.0)

        # Concatenate all features
        all_features = np.concatenate([flat_features, np.array(event_feat_values, dtype=np.float32)])

        return all_features, max_input_id, n_candidates

    def _get_inference_experiment_feature_value(self):
        """Return the raw experiment id."""
        event_meta = Belle2.PyStoreObj('EventMetaData')
        experiment = int(event_meta.getExperiment()) if event_meta.isValid() else 0

        if self.training_mode or experiment in config.ALLOWED_EXPERIMENTS:
            return experiment

        self._unsupported_experiment_event_count += 1
        self._unsupported_experiment_counts[experiment] = (
            self._unsupported_experiment_counts.get(experiment, 0) + 1
        )
        return config.DEFAULT_EXPERIMENT

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

    def _compute_fei_calib_weight(self, best_bp, best_b0):
        """
        Compute event-level FEI calibration weight from the best candidates.

        Based on the reconstructed candidate, checking for truth-compatible tag PDG
        and DeltaP < DELTA_P_THRESH. Returns FEI_CALIB_CONT for continuum events.
        Returns NaN when reco conditions are not met (non-continuum events where
        tag PDG does not match or DeltaP is above threshold).
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

    def _compute_training_event_scalars(
        self, bp_truth, b0_truth, best_bp_iid, best_bp_dp, best_b0_iid, best_b0_dp
    ):
        """
        Compute the per-event training scalar fields written to EventExtraInfo.

        best_bp_iid/best_bp_dp/best_b0_iid/best_b0_dp come from the truth-tag-matched
        particle_by_input_id scan already performed in event(); everything else is
        derived from the best-B+/best-B0 MC truth scalars for this event.
        """
        bp_pdg = bp_truth['pdg']
        b0_pdg = b0_truth['pdg']
        bp_dm = bp_truth['dm']
        b0_dm = b0_truth['dm']
        bp_sig = -1.0 if np.isnan(bp_truth['sigprob']) else float(bp_truth['sigprob'])
        b0_sig = -1.0 if np.isnan(b0_truth['sigprob']) else float(b0_truth['sigprob'])
        bp_is_cont = bp_truth['is_cont']
        b0_is_cont = b0_truth['is_cont']
        bp_gen_pdg = bp_truth['tag_pdg']
        b0_gen_pdg = b0_truth['tag_pdg']
        bp_gen_dm_id = -1 if np.isnan(bp_truth['gen_dm_id']) else int(bp_truth['gen_dm_id'])
        b0_gen_dm_id = -1 if np.isnan(b0_truth['gen_dm_id']) else int(b0_truth['gen_dm_id'])
        bp_gen_calib_weight = 1.0 if np.isnan(bp_truth['gen_calib_weight']) else float(bp_truth['gen_calib_weight'])
        b0_gen_calib_weight = 1.0 if np.isnan(b0_truth['gen_calib_weight']) else float(b0_truth['gen_calib_weight'])

        bp_is_best = 1 if bp_sig >= b0_sig else 0
        best_sigprob = max(bp_sig, b0_sig)

        is_cont_f = bp_is_cont if bp_is_best else b0_is_cont
        is_cont = 1 if is_cont_f == 1.0 else 0

        gen_pdg_f = bp_gen_pdg if bp_is_best else b0_gen_pdg
        gen_pdg = -1 if is_cont == 1 else (-1 if np.isnan(gen_pdg_f) else int(gen_pdg_f))

        bp_dm_i = -1 if np.isnan(bp_dm) else int(bp_dm)
        b0_dm_i = -1 if np.isnan(b0_dm) else int(b0_dm)

        best_bp_sigprob_iid = -1
        if not np.isnan(bp_pdg) and not np.isnan(bp_dm):
            offset = 0 if abs(int(bp_pdg)) == 521 else config.N_BP_MODES * 2
            sign = 1 if bp_pdg > 0 else 0
            best_bp_sigprob_iid = offset + bp_dm_i * 2 + sign

        best_b0_sigprob_iid = -1
        if not np.isnan(b0_pdg) and not np.isnan(b0_dm):
            offset = 0 if abs(int(b0_pdg)) == 521 else config.N_BP_MODES * 2
            sign = 1 if b0_pdg > 0 else 0
            best_b0_sigprob_iid = offset + b0_dm_i * 2 + sign

        bp_tag_is_gen = int(config.truth_tag_matches_pdg(bp_pdg, bp_gen_pdg, bp_dm))
        b0_tag_is_gen = int(config.truth_tag_matches_pdg(b0_pdg, b0_gen_pdg, b0_dm))

        use_bp = bp_is_best == 1
        tag_is_gen_ev = bp_tag_is_gen if use_bp else b0_tag_is_gen
        best_dp_ev = best_bp_dp if use_bp else best_b0_dp
        sigprob_iid_ev = best_bp_sigprob_iid if use_bp else best_b0_sigprob_iid
        bb_mask_ev = is_cont != 1
        use_reco_ev = (tag_is_gen_ev == 1) and (best_dp_ev < config.DELTA_P_THRESH) and (sigprob_iid_ev >= 0)

        fei_calib_weight = config.FEI_CALIB_CONT
        bp_threshold = config.N_BP_MODES * 2

        if bb_mask_ev and use_reco_ev:
            if use_bp:
                dm = min(max(best_bp_sigprob_iid // 2, 0), config.N_BP_MODES - 1)
                fei_calib_weight = float(self._bp_calib_lookup[dm])
            else:
                dm = min(max((best_b0_sigprob_iid - bp_threshold) // 2, 0), config.N_B0_MODES - 1)
                fei_calib_weight = float(self._b0_calib_lookup[dm])
        elif bb_mask_ev:
            abs_pdg_ev = abs(gen_pdg)
            gen_dm_ev = bp_gen_dm_id if use_bp else b0_gen_dm_id
            if abs_pdg_ev == 521:
                fei_calib_weight = (
                    float(self._bp_calib_lookup[gen_dm_ev])
                    if 0 <= gen_dm_ev < config.N_BP_MODES else self._bp_calib_rest
                )
            elif abs_pdg_ev == 511:
                fei_calib_weight = (
                    float(self._b0_calib_lookup[gen_dm_ev])
                    if 0 <= gen_dm_ev < config.N_B0_MODES else self._b0_calib_rest
                )

        return {
            'is_cont': is_cont,
            'gen_pdg': gen_pdg,
            'bp_gen_decay_mode_id': bp_gen_dm_id,
            'b0_gen_decay_mode_id': b0_gen_dm_id,
            'bp_gen_fei_calib_weight': bp_gen_calib_weight,
            'b0_gen_fei_calib_weight': b0_gen_calib_weight,
            'bp_is_best': bp_is_best,
            'best_sigprob': best_sigprob,
            'best_bp_sigprob_iid': best_bp_sigprob_iid,
            'best_b0_sigprob_iid': best_b0_sigprob_iid,
            'bp_tag_is_gen': bp_tag_is_gen,
            'b0_tag_is_gen': b0_tag_is_gen,
            'fei_calib_weight': fei_calib_weight,
            'best_bp_iid': best_bp_iid,
            'best_bp_dp': best_bp_dp,
            'best_b0_iid': best_b0_iid,
            'best_b0_dp': best_b0_dp,
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

        b2.B2DEBUG(10, f"{'='*60}")
        b2.B2DEBUG(10, f"DEBUG Event {self.event_count} ({event_id_str})")
        b2.B2DEBUG(10, f"{'='*60}")
        b2.B2DEBUG(10, f"Number of candidates: {len(candidates_data)}")
        b2.B2DEBUG(10, f"Max input_id (best candidate): {max_input_id}")

        b2.B2DEBUG(10, "--- Candidates ---")
        for i, (input_id, features) in enumerate(candidates_data):
            b2.B2DEBUG(10, f"  Candidate {i}: input_id={input_id}")
            for key, val in features.items():
                b2.B2DEBUG(10, f"    {key}: {val}")

        b2.B2DEBUG(10, "--- Event Features ---")
        for key, val in event_features.items():
            b2.B2DEBUG(10, f"  {key}: {val}")

        b2.B2DEBUG(10, "--- Feature Array (non-zero, first 5 blocks) ---")
        n_blocks = len(self.feature_blocks)
        for block_idx in range(min(5, n_blocks)):
            block_name = self.feature_blocks[block_idx][0]
            start = block_idx * self.n_input_ids
            end = start + self.n_input_ids
            block_data = all_features[start:end]
            non_zero = [(j, v) for j, v in enumerate(block_data) if v != 0]
            if non_zero:
                b2.B2DEBUG(10, f"  {block_name}: {non_zero[:10]}...")

        # Log last few features (event-level)
        n_candidate_features = n_blocks * self.n_input_ids
        event_feat_start = n_candidate_features
        b2.B2DEBUG(10, f"--- Event-level features (indices {event_feat_start}+) ---")
        event_feat_names = self.event_features + ['ncandidates/10', 'max_input_id/50', 'scnd_max_input_id/50', '__experiment__/10']
        for i, name in enumerate(event_feat_names):
            idx = event_feat_start + i
            if idx < len(all_features):
                b2.B2DEBUG(10, f"  {name}: {all_features[idx]}")

        b2.B2DEBUG(10, f"--- Total feature array shape: {len(all_features)} ---")

    def terminate(self):
        """Called at the end of processing."""
        if self._presel_violation_count > 0:
            frac = self._presel_violation_count / max(self._presel_total_candidates, 1)
            _sep = "=" * 70
            b2.B2WARNING(
                f"\n{_sep}\n"
                f"ModeSelector: {self._presel_violation_count}/{self._presel_total_candidates} "
                f"candidates ({100.0 * frac:.2f}%) did not pass the required preselections "
                f"(Mbc > {config.PRESELECTION_MBC_MIN}, "
                f"{config.PRESELECTION_DELTAE_MIN} < deltaE < {config.PRESELECTION_DELTAE_MAX}, "
                f"cosTBTO < {config.PRESELECTION_COSTBTO_MAX}). "
                f"Apply these cuts before running ModeSelector to match the training setup.\n{_sep}"
            )

        if self._unsupported_experiment_event_count > 0:
            unsupported_summary = ", ".join(
                f"{exp} ({count} event(s))"
                for exp, count in sorted(self._unsupported_experiment_counts.items())
            )
            b2.B2WARNING(
                "ModeSelector: unsupported EventMetaData experiment ids encountered in "
                f"{self._unsupported_experiment_event_count} event(s); replaced with default "
                f"experiment {config.DEFAULT_EXPERIMENT}. Observed unsupported values: "
                f"{unsupported_summary}."
            )

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
                f"({100.0 * missing_frac:.3f}%), high-confidence (sigProb>{config.HIGH_CONF_SIGPROB_MIN}) "
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
                f"({100.0 * fallback_frac:.3f}%), high-confidence (sigProb>{config.HIGH_CONF_SIGPROB_MIN}) "
                f"{self._empty_predicted_sector_high_conf_count}/{self._high_conf_event_count} event(s) "
                f"({100.0 * high_conf_fallback_frac:.3f}%); used score fallback."
            )

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

                # Check preselection compliance (negligible overhead; values already extracted)
                self._presel_total_candidates += 1
                _mbc = features.get('Mbc')
                _de = features.get('deltaE')
                _cos = features.get('cosTBTO')
                if (
                    (_mbc is None or _mbc <= config.PRESELECTION_MBC_MIN)
                    or (_de is None or not (config.PRESELECTION_DELTAE_MIN < _de < config.PRESELECTION_DELTAE_MAX))
                    or (_cos is None or _cos >= config.PRESELECTION_COSTBTO_MAX)
                ):
                    self._presel_violation_count += 1

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

        # Training mode: expose features + MC truth via EventExtraInfo/ExtraInfo for
        # variablesToNtuple to dump in the steering script, skip NN inference.
        if self.training_mode:
            event_extra_info = Belle2.PyStoreObj('EventExtraInfo')
            if not event_extra_info.isValid():
                event_extra_info.create()

            for i, value in enumerate(all_features):
                event_extra_info.addExtraInfo(f'modeSelector_feat_{i:04d}', float(value))

            bp_truth = self._extract_mc_truth_scalars(best_bp)
            b0_truth = self._extract_mc_truth_scalars(best_b0)

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

            # Mark the deduplicated best-per-input_id signal candidates so the
            # per-candidate ntuple dump in the steering script can pick them out.
            for iid, particle in particle_by_input_id.items():
                is_signal = vm.evaluate('isSignal', particle)
                if not np.isnan(is_signal) and int(is_signal) == 1:
                    particle.addExtraInfo('modeSelector_trainSigInputId', int(iid))

            training_scalars = self._compute_training_event_scalars(
                bp_truth, b0_truth, best_bp_iid, best_bp_dp, best_b0_iid, best_b0_dp
            )
            for name, value in training_scalars.items():
                event_extra_info.addExtraInfo(f'modeSelector_tr_{name}', float(value))
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
            for i, v in enumerate(features.tolist()):
                self.cat_dataset.m_input[i] = v
            cat_output = list(self.cat_expert.applyMulticlass(self.cat_dataset)[0])

            # Determine predicted category (0=B0, 1=B+, 2=continuum)
            charged_cat = 1.0 if cat_output[1] > cat_output[0] else 0.0

            # Build main network input (features + cat_output + charged_cat)
            main_vals = features.tolist() + cat_output + [charged_cat]

            if len(main_vals) != self.main_input_size:
                b2.B2FATAL(
                    f"ModeSelector: main model input size mismatch: "
                    f"got {len(main_vals)} features, model expects {self.main_input_size}. "
                    f"The ONNX model and current config must match."
                )

            for i, v in enumerate(main_vals):
                self.main_dataset.m_input[i] = v
            main_output = list(self.main_expert.applyMulticlass(self.main_dataset)[0])
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
        best_sigprob = max(best_bp_sig, best_b0_sig)
        is_high_conf = best_sigprob > config.HIGH_CONF_SIGPROB_MIN
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
        if self.store_fei_calib_weight:
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
