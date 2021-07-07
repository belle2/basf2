#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Study of alignment limits.

import basf2
from ROOT.Belle2 import KLMDisplacementGenerator

# Create main path
main = basf2.create_path()
basf2.set_log_level(basf2.LogLevel.INFO)

# EventInfoSetter
main.add_module('EventInfoSetter')

# Gearbox
main.add_module('Gearbox')

# Process the main path
basf2.process(main)

displacementGenerator = KLMDisplacementGenerator()
displacementGenerator.studyAlignmentLimits('EKLMAlignmentLimits.root')
