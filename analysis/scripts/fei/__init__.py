#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Full Event Interpretation framework for Belle II
Thomas Keck, Christian Pulvermacher 2013-2017

Detailed usage examples can be found in analysis/examples/FEI/
"""

# @cond internal
# only public interfaces, to make package doc nicer. Also the only things imported by 'from fei import *'
__all__ = ['Particle', 'FeiState', 'get_path', 'get_default_channels', 'get_unittest_channels', 'do_trainings',
           'MVAConfiguration', 'PreCutConfiguration', 'PostCutConfiguration', 'DecayChannel', 'Resource']
# @endcond

from fei.config import *
from fei.default_channels import *
from fei.fei import get_path, FeiState, do_trainings
