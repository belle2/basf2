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

This module provides tools for improving FEI B meson signal probability
by leveraging information from all candidates in an event.

Main components:
- modeSelector(): Main function to add ModeSelector to a path
- addDstarVeto(): Add D* veto reconstruction
- ModeSelectorModule: The basf2 Python module for NN evaluation

Example usage:
    import modeSelector

    # Add D* veto reconstruction (optional, improves performance)
    modeSelector.addDstarVeto(['B+:fei', 'B0:fei'], path=my_path)

    # Add ModeSelector NN evaluation
    modeSelector.modeSelector(
        bp_list='B+:fei',
        b0_list='B0:fei',
        path=my_path
    )

    # Access the score in ntuples
    variables = ['extraInfo(BplusScore)']
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
    cat_model_path=None,
    main_model_path=None,
    payload_cat_model='modeSelector_cat_model_v0',
    payload_main_model='modeSelector_main_model_v0',
    output_variable='BplusScore',
    addDstarVetoReco=False,
    training_mode=False,
    training_output='modeSelector_training.npz',
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

    Parameters
    ----------
    bp_list : str
        B+ meson particle list name to process.
        Example: 'B+:feiHadronic'
    b0_list : str
        B0 meson particle list name to process.
        Example: 'B0:feiHadronic'
    cat_model_path : str, optional
        Path to the category network ONNX model file.
        If None, loads from conditions database.
    main_model_path : str, optional
        Path to the main network ONNX model file.
        If None, loads from conditions database.
    payload_cat_model : str, optional
        Conditions DB payload name for the category model.
        Used only when cat_model_path is None.
    payload_main_model : str, optional
        Conditions DB payload name for the main model.
        Used only when main_model_path is None.
    output_variable : str
        Name of the ExtraInfo variable for the output score.
        Default: 'BplusScore'
    addDstarVetoReco : bool
        Whether to add D* veto reconstruction before the NN.
        Default: False (assumes already added or not needed)
    path : basf2.Path
        The basf2 path to add the module to.

    Returns
    -------
    None

    Notes
    -----
    The ModeSelector neural network outputs:

    Candidate-level (ExtraInfo on best B+ and best B0):
    - modeSelector_eqSigProb: Probability assigned to a candidate's input_id
      in the predicted sector

    Event-level (EventExtraInfo):
    - {output_variable}: signed main score
    - modeSelector_catB0: Category network B0 probability
    - modeSelector_catBp: Category network B+ probability
    - modeSelector_catCont: Category network continuum probability

    Candidate-level ranking (ExtraInfo on deduplicated representatives):
    - modeSelector_rank: predicted sector ranked by modeSelector_eqSigProb,
      non-predicted sector ranked by sigProb

    For best performance, run addDstarVeto() before modeSelector() to
    provide D* veto features to the network.

    Example
    -------
    >>> import basf2 as b2
    >>> import modularAnalysis as ma
    >>> import modeSelector
    >>>
    >>> path = b2.create_path()
    >>> ma.inputMdstList('default', path=path)
    >>>
    >>> # Assume FEI B lists are available
    >>> # Add D* veto reconstruction
    >>> modeSelector.addDstarVeto(['B+:fei', 'B0:fei'], path=path)
    >>>
    >>> # Add ModeSelector
    >>> modeSelector.modeSelector(
    ...     bp_list='B+:fei',
    ...     b0_list='B0:fei',
    ...     path=path
    ... )
    >>>
    >>> # Write to ntuple
    >>> ma.variablesToNtuple(
    ...     'B+:fei',
    ...     ['extraInfo(BplusScore)', 'extraInfo(SignalProbability)'],
    ...     filename='output.root',
    ...     path=path
    ... )
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

    # Optionally add D* veto reconstruction
    if addDstarVetoReco:
        addDstarVeto(particle_lists, path=path)

    # Add the ModeSelector module
    module = ModeSelectorModule(
        particle_lists=particle_lists,
        cat_model_path=cat_model_path,
        main_model_path=main_model_path,
        payload_cat_model=payload_cat_model,
        payload_main_model=payload_main_model,
        output_variable=output_variable,
        training_mode=training_mode,
        training_output=training_output,
        debug=debug,
        debug_max_events=debug_max_events,
    )
    path.add_module(module)

    b2.B2INFO(f"ModeSelector: Added to path for particle lists: {particle_lists}")
    b2.B2INFO(f"ModeSelector: Output variable: extraInfo({output_variable})")
