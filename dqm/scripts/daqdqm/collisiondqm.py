#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

from daqdqm.commondqm import add_common_dqm


def add_collision_dqm(path, components=None, dqm_environment="expressreco"):
    """
    This function adds DQMs for collisions

    @param components: A list of the detector components which are available in this
                       run of basf2
    @param dqm_environment: The environment the DQM modules are running in
                            "expressreco" (default) if running on the ExpressReco system
                            "hlt" if running on the HLT online reconstructon nodes
                            If running on the hlt, you may want to output less or other DQM plots
                            due to the limited bandwith of the HLT nodes.
    """

    add_common_dqm(path, components, dqm_environment)
