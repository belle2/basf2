#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <contact>P. Urquijo phillip.urquijo@unimelb.edu.au</contact>
  <output>../Validate_B2Vlnu.root</output>
</header>
"""

import basf2
from generators import add_evtgen_generator
from modularAnalysis import findMCDecay
from variables import variables
from validation_tools.metadata import create_validation_histograms

basf2.set_random_seed('B+ => [anti-D*0 -> anti-D0 pi0] e+ nu_e')

variables.addAlias('q2', 'formula(daughterInvM(1, 2)**2)')

path = basf2.Path()
path.add_module('EventInfoSetter', evtNumList=[10000])
add_evtgen_generator(path, 'signal', basf2.find_file('decfiles/dec/Bu_Dst0enu.dec'))
findMCDecay('B+:sig', 'B+ -> [anti-D*0 -> anti-D0 pi0] e+ nu_e', path=path)
create_validation_histograms(
    path, '../Validate_B2Vlnu.root', 'B+:sig',
    [
        ('q2', 50, -1, 12, '', 'P. Urquijo <phillip.urquijo@unimelb.edu.au>',
         r'B2Vlnu $q^2$ of the $e^+\nu_{e}$ system in $B^+ \to \bar{D^{*0}} e^+ \nu_e$ (truth values)',
         'should follow the reference', '#q^2'),
    ],
    description='B2Vlnu Validation to check generator level distributions',
)
basf2.process(path)
