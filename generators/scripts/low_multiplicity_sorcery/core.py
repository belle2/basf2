##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
from .utils import add_low_multiplicity_generator, EventRangePathSplitter, ExtraInfoPathSplitter
from .constants import EventCodes_taus

from basf2 import Path, AfterConditionPath


def add_generators(path, event_codes):
    """
    Add generators for a list of event codes.

    Parameters:
        path (basf2.Path): path where generators should be added
        event_codes (Iterable): tuple or list of EventCode.
    """

    s1 = path.add_module(EventRangePathSplitter(event_codes))

    for e in event_codes:
        generator_path = Path()
        add_low_multiplicity_generator(generator_path, e)
        s1.if_value(f'={e.value}', generator_path, AfterConditionPath.CONTINUE)

    if len(tuple(set(event_codes) & set(EventCodes_taus))):
        s2 = path.add_module(ExtraInfoPathSplitter(EventCodes_taus))
        tau_decay_path = Path()
        tau_decay_path.add_module('EvtGenDecay')
        s2.if_true(tau_decay_path, AfterConditionPath.CONTINUE)
