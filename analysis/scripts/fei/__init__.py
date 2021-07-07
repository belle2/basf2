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

# @cond internal
# only public interfaces, to make package doc nicer. Also the only things imported by 'from fei import *'
__all__ = ['FeiState', 'get_path', 'get_default_channels', 'get_unittest_channels', 'do_trainings',
           'Particle', 'MVAConfiguration', 'PreCutConfiguration', 'PostCutConfiguration', 'FeiConfiguration', 'DecayChannel']
# @endcond

from fei.config import Particle, MVAConfiguration, PreCutConfiguration, PostCutConfiguration, FeiConfiguration, DecayChannel
from fei.default_channels import get_default_channels, get_unittest_channels
from fei.core import get_path, FeiState, do_trainings
