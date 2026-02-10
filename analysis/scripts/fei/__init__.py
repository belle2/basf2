#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Full Event Interpretation framework for Belle II

Detailed usage examples can be found in analysis/examples/FEI/
"""

# only public interfaces, to make package doc nicer. Also the only things imported by 'from fei import *'
__all__ = [
    'FeiState',
    'get_path',
    'get_stages_from_particles',
    'get_default_channels',
    'get_ccbarLambdaC_channels',
    'get_unittest_channels',
    'do_trainings',
    'get_mode_names',
    'Particle',
    'MVAConfiguration',
    'PreCutConfiguration',
    'PostCutConfiguration',
    'FeiConfiguration',
    'DecayChannel',
    'save_summary'
]

from fei.config import Particle, MVAConfiguration, PreCutConfiguration, PostCutConfiguration, FeiConfiguration, DecayChannel
from fei.default_channels import get_default_channels, get_ccbarLambdaC_channels, get_unittest_channels, get_mode_names
from fei.core import get_path, FeiState, do_trainings, get_stages_from_particles, save_summary
