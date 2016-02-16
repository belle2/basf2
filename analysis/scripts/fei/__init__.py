#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Full Event Interpretation framework for Belle II
Thomas Keck, Christian Pulvermacher 2013-2016

Detailed usage examples can be found in analysis/examples/FEI/
"""

# @cond internal
# only public interfaces, to make package doc nicer. Also the only things imported by 'from fei import *'
__all__ = ['Particle', 'FeiState', 'fullEventInterpretation', 'get_default_channels', 'get_unittest_channels',
           'MVAConfiguration', 'PreCutConfiguration', 'PostCutConfiguration', 'DecayChannel', 'Resource']
# @endcond

from fei.config import *
from fei.dag import Resource
from fei.steering import fullEventInterpretation, FeiState
from fei.default_channels import *
