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

import basf2
from flavorTagger import utils
from .core.flavorTagger import flavorTagger
filepath = basf2.find_file('data/analysis/output_vars.yaml')
flavor_tagging = utils.read_yaml(filepath)["output_vars"]

# only public interfaces, to make package doc nicer. Also the only things imported by 'from flavorTagger import *'
__all__ = ['utils', 'flavorTagger', 'flavor_tagging']
