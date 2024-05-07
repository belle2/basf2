#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# *************  Flavor Tagging   ************
# * This script is needed to train           *
# * and to test the flavor tagger.           *
# ********************************************

import os
from fTagger import utils
from .core.flavor_tagger import flavor_tagger
filepath = os.path.join(os.path.dirname(__file__), 'statics/output_vars.yaml')
flavor_tagging = utils.read_yaml(filepath)["output_vars"]

# only public interfaces, to make package doc nicer. Also the only things imported by 'from fei import *'
__all__ = ['utils', 'flavor_tagger', 'flavor_tagging']
# @endcond
