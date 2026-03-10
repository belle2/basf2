#!/usr/bin/env python3
##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Calibration-aware eventRandom sampling for ModeSelector MC workflows.
"""

import basf2 as b2
from modeSelector import config
from ROOT import Belle2
from variables import variables as vm

DEFAULT_WEIGHT_VARIABLE = 'modeSelectorCalibWeight'
DEFAULT_INPUT_ID_VARIABLE = 'modeSelectorCalibInputId'
NO_SELECTED_INPUT_ID = -1


def get_input_id(pdg, dm_id):
    """
    Compute the ModeSelector input_id from PDG and decay mode ID.
    """
    is_particle = 1 if pdg > 0 else 0
    offset = 0 if abs(pdg) == 521 else config.N_BP_MODES * 2
    return offset + dm_id * 2 + is_particle


def get_fei_calib(dm_id, pdg):
    """
    Get the FEI calibration factor for a decay mode.
    """
    if abs(pdg) in (511, 521):
        calib_map = config.get_fei_calibration_map(pdg)
        calib_rest = config.get_fei_calibration_rest(pdg)
        return calib_map.get(dm_id, calib_rest)
    return 1.0


def compute_candidate_calibration(pdg, dm_id, tag_pdg, is_cont):
    """
    Compute the calibration factor for a selected calibration candidate.
    """
    if is_cont:
        return 1.0
    if pdg is None or dm_id is None or tag_pdg is None:
        return 1.0
    if not config.truth_tag_matches_pdg(pdg, tag_pdg, dm_id):
        return 1.0
    return get_fei_calib(dm_id, pdg)


def compute_event_sample_probability(event_calib, is_cont, base_fraction, cont_fraction):
    """
    Compute the event keep probability from an event-level calibration factor.
    """
    if is_cont:
        return min(base_fraction * cont_fraction, 1.0)
    return min(base_fraction * event_calib, 1.0)


def is_event_sample_probability_clipped(event_calib, is_cont, base_fraction, cont_fraction):
    """
    Whether the requested keep probability is clipped at one.
    """
    if is_cont:
        return False
    return base_fraction * event_calib > 1.0


def build_candidate_record(pdg, dm_id, tag_pdg, is_cont, event_random,
                           is_signal, delta_p, sigprob):
    """
    Build a normalized candidate record for generated-decay selection.
    """
    if pdg is None or dm_id is None:
        input_id = None
    else:
        input_id = get_input_id(pdg, dm_id)

    return {
        'pdg': pdg,
        'dm_id': dm_id,
        'tag_pdg': tag_pdg,
        'is_cont': is_cont,
        'event_random': event_random,
        'is_signal': is_signal,
        'delta_p': delta_p,
        'sigprob': sigprob,
        'input_id': input_id,
    }


def is_truth_compatible_candidate(candidate, delta_p_thresh=config.DELTA_P_THRESH):
    """
    Whether a candidate is eligible for generated-decay calibration lookup.
    """
    if candidate['is_cont']:
        return False
    if candidate['pdg'] is None or candidate['tag_pdg'] is None or candidate['dm_id'] is None:
        return False
    if candidate['delta_p'] is None or candidate['sigprob'] is None or candidate['input_id'] is None:
        return False
    if not config.truth_tag_matches_pdg(
        candidate['pdg'],
        candidate['tag_pdg'],
        candidate['dm_id'],
    ):
        return False
    return candidate['is_signal'] or candidate['delta_p'] < delta_p_thresh


def choose_generated_decay_candidate(candidates, delta_p_thresh=config.DELTA_P_THRESH):
    """
    Choose the training-style truth proxy candidate for calibration.

    Returns
    -------
    tuple
        (chosen_candidate or None, used_is_signal)
    """
    eligible = [
        candidate for candidate in candidates
        if is_truth_compatible_candidate(candidate, delta_p_thresh=delta_p_thresh)
    ]
    if not eligible:
        return None, False

    signal_candidates = [candidate for candidate in eligible if candidate['is_signal']]
    pool = signal_candidates if signal_candidates else eligible

    def _rank_key(candidate):
        return (
            float(candidate['delta_p']),
            -float(candidate['sigprob']),
            -int(candidate['input_id']),
        )

    return min(pool, key=_rank_key), bool(signal_candidates)


def compute_event_calibration_metadata(candidates, delta_p_thresh=config.DELTA_P_THRESH):
    """
    Compute the event-level calibration metadata from FEI candidates.
    """
    is_cont = any(candidate['is_cont'] for candidate in candidates)
    chosen_candidate, used_is_signal = choose_generated_decay_candidate(
        candidates,
        delta_p_thresh=delta_p_thresh,
    )

    if chosen_candidate is None:
        return {
            'event_calib': 1.0,
            'selected_input_id': NO_SELECTED_INPUT_ID,
            'used_is_signal': False,
            'used_delta_p_fallback': False,
            'no_truth_match_bb': not is_cont,
            'is_cont': is_cont,
        }

    return {
        'event_calib': compute_candidate_calibration(
            chosen_candidate['pdg'],
            chosen_candidate['dm_id'],
            chosen_candidate['tag_pdg'],
            chosen_candidate['is_cont'],
        ),
        'selected_input_id': chosen_candidate['input_id'],
        'used_is_signal': used_is_signal,
        'used_delta_p_fallback': not used_is_signal,
        'no_truth_match_bb': False,
        'is_cont': is_cont,
    }


class CalibratedEventRandomFilterModule(b2.Module):
    """
    Filter MC events using online FEI calibration-aware eventRandom sampling.
    """

    def __init__(self, bp_list, b0_list, base_fraction=0.3, cont_fraction=0.25,
                 random_variable='eventRandom', apply_cut=True,
                 weight_variable=DEFAULT_WEIGHT_VARIABLE,
                 input_id_variable=DEFAULT_INPUT_ID_VARIABLE,
                 delta_p_thresh=config.DELTA_P_THRESH):
        super().__init__()
        self.bp_list = bp_list
        self.b0_list = b0_list
        self.base_fraction = base_fraction
        self.cont_fraction = cont_fraction
        self.random_variable = random_variable
        self.apply_cut = apply_cut
        self.weight_variable = weight_variable
        self.input_id_variable = input_id_variable
        self.delta_p_thresh = delta_p_thresh
        self._validated = False
        self._summary = {
            'processed': 0,
            'kept': 0,
            'kept_cont': 0,
            'kept_bb': 0,
            'empty_events': 0,
            'used_is_signal': 0,
            'used_delta_p_fallback': 0,
            'no_truth_match_bb': 0,
            'clipped_probability': 0,
        }
        self._warned_probability_clip = False

    def initialize(self):
        self._validate_configuration()

    def _validate_configuration(self):
        if self._validated:
            return

        if not isinstance(self.bp_list, str) or not self.bp_list.startswith('B+:'):
            b2.B2FATAL(f"ModeSelector calibrated sampling: invalid bp_list '{self.bp_list}'")
        if not isinstance(self.b0_list, str) or not self.b0_list.startswith('B0:'):
            b2.B2FATAL(f"ModeSelector calibrated sampling: invalid b0_list '{self.b0_list}'")
        if not 0.0 <= self.base_fraction <= 1.0:
            b2.B2FATAL(f"ModeSelector calibrated sampling: base_fraction must be in [0, 1], got {self.base_fraction}")
        if not 0.0 <= self.cont_fraction <= 1.0:
            b2.B2FATAL(f"ModeSelector calibrated sampling: cont_fraction must be in [0, 1], got {self.cont_fraction}")
        if not isinstance(self.random_variable, str) or not self.random_variable:
            b2.B2FATAL("ModeSelector calibrated sampling: random_variable must be a non-empty string")
        if not isinstance(self.apply_cut, bool):
            b2.B2FATAL(f"ModeSelector calibrated sampling: apply_cut must be bool, got {self.apply_cut}")
        if not isinstance(self.weight_variable, str) or not self.weight_variable:
            b2.B2FATAL("ModeSelector calibrated sampling: weight_variable must be a non-empty string")
        if not isinstance(self.input_id_variable, str) or not self.input_id_variable:
            b2.B2FATAL("ModeSelector calibrated sampling: input_id_variable must be a non-empty string")
        if self.delta_p_thresh < 0.0:
            b2.B2FATAL(
                "ModeSelector calibrated sampling: "
                f"delta_p_thresh must be >= 0, got {self.delta_p_thresh}"
            )
        self._validated = True

    def _extract_candidate_info(self, particle):
        """
        Extract the variables needed for calibration selection.
        """
        pdg = vm.evaluate('PDG', particle)
        dm_id = vm.evaluate('extraInfo(decayModeID)', particle)
        tag_pdg = vm.evaluate('mostcommonBTagPDG', particle)
        is_cont = vm.evaluate('isContinuumEvent', particle)
        event_random = vm.evaluate(self.random_variable, particle)
        is_signal = vm.evaluate('isSignal', particle)
        delta_p = vm.evaluate('mostcommonBTagDeltaP', particle)
        sigprob = vm.evaluate('extraInfo(SignalProbability)', particle)

        return build_candidate_record(
            pdg=None if pdg != pdg else int(pdg),
            dm_id=None if dm_id != dm_id else int(dm_id),
            tag_pdg=None if tag_pdg != tag_pdg else int(tag_pdg),
            is_cont=False if is_cont != is_cont else bool(int(is_cont)),
            event_random=None if event_random != event_random else float(event_random),
            is_signal=False if is_signal != is_signal else bool(int(is_signal)),
            delta_p=None if delta_p != delta_p else float(delta_p),
            sigprob=None if sigprob != sigprob else float(sigprob),
        )

    def _collect_candidates(self):
        """
        Collect candidate records from both FEI lists.
        """
        candidates = []
        for list_name in [self.bp_list, self.b0_list]:
            plist = Belle2.PyStoreObj(list_name)
            if not plist.isValid():
                continue

            for i in range(plist.getListSize()):
                particle = plist.obj().getParticle(i)
                candidates.append(self._extract_candidate_info(particle))

        return candidates

    def _pick_reference_candidate(self, candidates):
        """
        Pick a candidate used to read event-level quantities like eventRandom.
        """
        for candidate in candidates:
            if candidate['event_random'] is not None:
                return candidate
        return None

    def _store_event_extra_info(self, key, value):
        """
        Store a value in EventExtraInfo, overwriting any previous value.
        """
        event_extra_info = Belle2.PyStoreObj('EventExtraInfo')
        if not event_extra_info.isValid():
            event_extra_info.create()
        if event_extra_info.hasExtraInfo(key):
            event_extra_info.setExtraInfo(key, value)
        else:
            event_extra_info.addExtraInfo(key, value)

    def event(self):
        candidates = self._collect_candidates()
        self._summary['processed'] += 1
        if not candidates:
            self._summary['empty_events'] += 1
            self._store_event_extra_info(self.weight_variable, 1.0)
            self._store_event_extra_info(self.input_id_variable, float(NO_SELECTED_INPUT_ID))
            self.return_value(1 if not self.apply_cut else 0)
            return

        metadata = compute_event_calibration_metadata(
            candidates,
            delta_p_thresh=self.delta_p_thresh,
        )
        self._store_event_extra_info(self.weight_variable, metadata['event_calib'])
        self._store_event_extra_info(
            self.input_id_variable,
            float(metadata['selected_input_id']),
        )

        if metadata['used_is_signal']:
            self._summary['used_is_signal'] += 1
        elif metadata['used_delta_p_fallback']:
            self._summary['used_delta_p_fallback'] += 1
        elif metadata['no_truth_match_bb']:
            self._summary['no_truth_match_bb'] += 1

        keep_prob = compute_event_sample_probability(
            metadata['event_calib'],
            metadata['is_cont'],
            self.base_fraction,
            self.cont_fraction,
        )
        if is_event_sample_probability_clipped(
            metadata['event_calib'],
            metadata['is_cont'],
            self.base_fraction,
            self.cont_fraction,
        ):
            self._summary['clipped_probability'] += 1
            if not self._warned_probability_clip:
                b2.B2WARNING(
                    "ModeSelector calibrated sampling: keep probability is clipped at 1.0 "
                    f"(base_fraction={self.base_fraction}, event_calib={metadata['event_calib']:.6f}, "
                    "fewer events than the requested calibration scaling can be kept)"
                )
                self._warned_probability_clip = True
        keep_event = True
        if self.apply_cut:
            reference_candidate = self._pick_reference_candidate(candidates)
            if reference_candidate is None:
                b2.B2FATAL(
                    "ModeSelector calibrated sampling: "
                    f"{self.random_variable} is not available on the FEI candidates"
                )
            keep_event = reference_candidate['event_random'] < keep_prob

        if keep_event:
            self._summary['kept'] += 1
            if metadata['is_cont']:
                self._summary['kept_cont'] += 1
            else:
                self._summary['kept_bb'] += 1

        self.return_value(int(keep_event))

    def terminate(self):
        processed = self._summary['processed']
        kept = self._summary['kept']
        if processed == 0:
            b2.B2INFO("ModeSelector calibrated sampling: no events processed")
            return

        frac = 100.0 * kept / processed
        cut_mode = 'applied' if self.apply_cut else 'disabled'
        b2.B2INFO(
            "ModeSelector calibrated sampling: kept "
            f"{kept}/{processed} event(s) ({frac:.2f}%), "
            f"cut={cut_mode}, "
            f"BB={self._summary['kept_bb']}, continuum={self._summary['kept_cont']}, "
            f"empty={self._summary['empty_events']}, "
            f"isSignal={self._summary['used_is_signal']}, "
            f"deltaP_fallback={self._summary['used_delta_p_fallback']}, "
            f"bb_no_truth={self._summary['no_truth_match_bb']}, "
            f"clipped={self._summary['clipped_probability']}"
        )


def addCalibratedEventRandomSampling(bp_list, b0_list,
                                     base_fraction=0.3, cont_fraction=0.25,
                                     random_variable='eventRandom',
                                     apply_cut=True,
                                     weight_variable=DEFAULT_WEIGHT_VARIABLE,
                                     input_id_variable=DEFAULT_INPUT_ID_VARIABLE,
                                     delta_p_thresh=config.DELTA_P_THRESH,
                                     path=None):
    """
    Add calibration-aware MC event sampling to a basf2 path.
    """
    if path is None:
        b2.B2FATAL("ModeSelector calibrated sampling: path is required")

    module = CalibratedEventRandomFilterModule(
        bp_list=bp_list,
        b0_list=b0_list,
        base_fraction=base_fraction,
        cont_fraction=cont_fraction,
        random_variable=random_variable,
        apply_cut=apply_cut,
        weight_variable=weight_variable,
        input_id_variable=input_id_variable,
        delta_p_thresh=delta_p_thresh,
    )
    path.add_module(module)
    module.if_false(b2.create_path(), b2.AfterConditionPath.END)

    b2.B2INFO(
        "ModeSelector calibrated sampling: added "
        f"(base_fraction={base_fraction}, cont_fraction={cont_fraction}, "
        f"random_variable={random_variable}, apply_cut={apply_cut}, "
        f"weight_variable={weight_variable}, input_id_variable={input_id_variable}, "
        f"delta_p_thresh={delta_p_thresh})"
    )
