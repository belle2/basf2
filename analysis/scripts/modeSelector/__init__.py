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
        particleLists=['B+:fei', 'B0:fei'],
        path=my_path
    )

    # Access the score in ntuples
    variables = ['extraInfo(BplusScore)']
"""

from modeSelector import config
from modeSelector.dstarVeto import add_dstar_veto_aliases, addDstarVeto
from modeSelector.ModeSelectorModule import ModeSelectorModule

__all__ = [
    'config',
    'modeSelector',
    'addDstarVeto',
    'add_dstar_veto_aliases',
    'ModeSelectorModule',
]


def modeSelector(
    particleLists,
    cat_model_path=None,
    main_model_path=None,
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

    The output score (BplusScore) is stored as ExtraInfo on the best B candidate
    and optionally in EventExtraInfo.

    Parameters
    ----------
    particleLists : str or list
        Name(s) of B meson particle list(s) to process.
        Example: 'B+:feiHadronic' or ['B+:fei', 'B0:fei']
    cat_model_path : str, optional
        Path to the category network ONNX model file.
        If None, loads from conditions database.
    main_model_path : str, optional
        Path to the main network ONNX model file.
        If None, loads from conditions database.
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
    - {output_variable}_eqSigProb: Signal probability (0.5 + Bp_score/2 for B+, 0.5 - Bp_score/2 for B0)

    Event-level (EventExtraInfo):
    - {output_variable}: Bp_score = main_output[5] - main_output[1]
    - {output_variable}_catB0: Category network B0 probability
    - {output_variable}_catBp: Category network B+ probability
    - {output_variable}_catCont: Category network continuum probability

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
    ...     particleLists=['B+:fei', 'B0:fei'],
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

    if isinstance(particleLists, str):
        particleLists = [particleLists]

    # Optionally add D* veto reconstruction
    if addDstarVetoReco:
        addDstarVeto(particleLists, path=path)

    # Add the ModeSelector module
    module = ModeSelectorModule(
        particle_lists=particleLists,
        cat_model_path=cat_model_path,
        main_model_path=main_model_path,
        output_variable=output_variable,
        training_mode=training_mode,
        training_output=training_output,
        debug=debug,
        debug_max_events=debug_max_events,
    )
    path.add_module(module)

    b2.B2INFO(f"ModeSelector: Added to path for particle lists: {particleLists}")
    b2.B2INFO(f"ModeSelector: Output variable: extraInfo({output_variable})")
