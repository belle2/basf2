#!/usr/bin/env python3
##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
ModeSelector - Event-level B meson classification using neural networks.

The FEI assigns a signal probability (sigProb) to each B candidate independently.
ModeSelector uses the full set of FEI B candidates with sigProb > 0.001 simultaneously
to classify the event and produce a refined score (BplusScore).

Main components:

- modeSelector(): main function to add ModeSelector to a basf2 path
- addDstarVeto(): D* veto reconstruction (called automatically by modeSelector)

See ``analysis/doc/ModeSelector.rst`` for usage instructions and
``analysis/scripts/modeSelector/README.md`` for implementation details.
"""

from modeSelector import config
from modeSelector.dstarVeto import addDstarVeto
from modeSelector.generatedDecayWeights import (GeneratedDecayWeightModule,
                                                addGeneratedDecayWeights)
from modeSelector.ModeSelectorModule import ModeSelectorModule

__all__ = [
    'config',
    'addGeneratedDecayWeights',
    'modeSelector',
    'addDstarVeto',
    'add_dstar_veto_aliases',
    'GeneratedDecayWeightModule',
    'get_exact_dmid_from_signature',
    'ModeSelectorModule',
]


def modeSelector(
    bp_list,
    b0_list,
    payload_cat_model=config.DEFAULT_CAT_PAYLOAD,
    payload_main_model=config.DEFAULT_MAIN_PAYLOAD,
    output_variable='BplusScore',
    cat_model_path=None,
    main_model_path=None,
    addDstarVetoReco=True,
    training_mode=False,
    skip_nn_evaluation=False,
    store_fei_calib_weight=False,
    debug=False,
    debug_max_events=10,
    path=None
):
    """
    Add ModeSelector neural network evaluation to a basf2 path.

    This function applies a two-stage neural network to FEI B meson candidates
    to compute an improved signal probability score. The network considers
    information from all B candidates in the event.

    The main output score is stored in EventExtraInfo using output_variable.
    Auxiliary category and per-candidate mode outputs are stored with the
    fixed modeSelector_* names.

    Parameters:
        bp_list (str): B+ meson particle list name to process. Example: 'B+:feiHadronic'
        b0_list (str): B0 meson particle list name to process. Example: 'B0:feiHadronic'
        payload_cat_model (str): Conditions DB payload name for the category model.
            Used only when cat_model_path is None.
        payload_main_model (str): Conditions DB payload name for the main model.
            Used only when main_model_path is None.
        output_variable (str): Name of the ExtraInfo variable for the output score.
            Default: 'BplusScore'
        cat_model_path (str): Path to the basf2 MVA weightfile for the category network,
            as produced by convert_to_onnx.py (a .root file, not a raw .onnx file).
            If None, loads from conditions database.
        main_model_path (str): Path to the basf2 MVA weightfile for the main network,
            as produced by convert_to_onnx.py (a .root file, not a raw .onnx file).
            If None, loads from conditions database.
        skip_nn_evaluation (bool): If True, skip loading and evaluating the neural networks
            and fill deterministic placeholder outputs instead. Intended for debugging or
            timing studies and emits a warning at module initialization.
        training_mode (bool): If True, skip NN inference and instead expose per-event
            features and MC truth as EventExtraInfo/ExtraInfo (``modeSelector_feat_XXXX``,
            ``modeSelector_tr_*``, ``modeSelector_trainSigInputId``) for a
            variablesToNtuple call in the steering script to dump. See
            ``analysis/examples/modeSelector/produceTrainingInputs.py``. Default: False.
        addDstarVetoReco (bool): Whether to add D* veto reconstruction before the NN.
            Default: True
        debug (bool): If True, print the feature vector and network outputs for the first
            few events. Intended for comparing against a reference implementation.
            Default: False
        debug_max_events (int): Number of events to print when debug is True. Default: 10
        store_fei_calib_weight (bool): If True, compute and store modeSelector_feiCalibWeight
            in EventExtraInfo using the reco path (truth-compatible tag PDG and
            DeltaP < DELTA_P_THRESH). Returns NaN when reco conditions are not met.
            Requires mostcommonBTagPDG and mostcommonBTagDeltaP to be defined.
            Meaningful only on MC. Default: False.
        path (basf2.Path): The basf2 path to add the module to.

    Notes:
        Candidate-level ``ExtraInfo`` (predicted sector only):

        - ``modeSelector_eqSigProb``: main-network probability for the candidate's
          specific decay mode.
        - ``modeSelector_rank``: sector-local rank. Predicted sector ranked by
          ``modeSelector_eqSigProb``; non-predicted sector ranked by ``sigProb``.

        Event-level ``EventExtraInfo``:

        - ``BplusScore`` (or the name given by ``output_variable``): signed score,
          positive for B+ prediction, negative for B0.
        - ``modeSelector_catB0``, ``modeSelector_catBp``, ``modeSelector_catCont``:
          category network probabilities.

        The D* veto reconstruction is added automatically (``addDstarVetoReco=True``).
        Pass ``addDstarVetoReco=False`` to skip it if already added separately.
    """
    import basf2 as b2

    if path is None:
        b2.B2FATAL("Path is required for modeSelector")

    if not isinstance(bp_list, str) or not bp_list:
        b2.B2FATAL("ModeSelector: bp_list must be a non-empty string")
    if not isinstance(b0_list, str) or not b0_list:
        b2.B2FATAL("ModeSelector: b0_list must be a non-empty string")
    if not bp_list.startswith('B+:'):
        b2.B2FATAL(f"ModeSelector: bp_list must start with 'B+:'; got '{bp_list}'")
    if not b0_list.startswith('B0:'):
        b2.B2FATAL(f"ModeSelector: b0_list must start with 'B0:'; got '{b0_list}'")
    particle_lists = [bp_list, b0_list]

    if addDstarVetoReco:
        # Add D* veto reconstruction (pi0 list created internally)
        addDstarVeto(particle_lists, path=path)

    # Add the ModeSelector module
    module = ModeSelectorModule(
        particle_lists=particle_lists,
        payload_cat_model=payload_cat_model,
        payload_main_model=payload_main_model,
        output_variable=output_variable,
        cat_model_path=cat_model_path,
        main_model_path=main_model_path,
        training_mode=training_mode,
        skip_nn_evaluation=skip_nn_evaluation,
        store_fei_calib_weight=store_fei_calib_weight,
        debug=debug,
        debug_max_events=debug_max_events,
    )
    path.add_module(module)

    b2.B2INFO(f"ModeSelector: Added to path for particle lists: {particle_lists}")
    b2.B2INFO(f"ModeSelector: Output variable: extraInfo({output_variable})")
