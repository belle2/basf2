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
  <output>../Validate_B2VV.root</output>
</header>
"""

import basf2
from generators import add_evtgen_generator
from modularAnalysis import findMCDecay
from variables import variables
from validation_tools.metadata import create_validation_histograms

basf2.set_random_seed('B+ => [rho0 -> pi+ pi-] [rho+ -> pi0 pi+]')

variables.addAlias('cosThetaRhoZ', 'cosHelicityAngle(0, 0)')
variables.addAlias('cosThetaRhoP', 'cosHelicityAngle(1, 1)')
variables.addAlias('planarAngle', 'cosAcoplanarityAngle(0, 0)')

path = basf2.Path()
path.add_module('EventInfoSetter', evtNumList=[10000])
add_evtgen_generator(path, 'signal', basf2.find_file('decfiles/dec/Bu_rho0rho+.dec'))
findMCDecay('B+:sig', 'B+ -> [rho0 -> pi+ pi-] [rho+ -> pi0 pi+]', path=path)
create_validation_histograms(
    path, '../Validate_B2VV.root', 'B+:sig',
    [
        ('cosThetaRhoZ', 50, -1.2, 1.2, '', 'P. Urquijo <phillip.urquijo@unimelb.edu.au>',
         r'B2VV helicity angle of the $\rho^0 \to \pi^+ \pi^-$ in $B^+ \to \rho^0 \rho^+$ (truth values)',
         'should follow the reference', 'cos#theta_{helicity}(V1)'),
        ('cosThetaRhoP', 50, -1.2, 1.2, '', 'P. Urquijo <phillip.urquijo@unimelb.edu.au>',
         r'B2VV helicity angle of the $\rho^+ \to \pi^0 \pi^+$ in $B^+ \to \rho^0 \rho^+$ (truth values)',
         'should follow the reference', 'cos#theta_{helicity}(V1)'),
        ('planarAngle', 50, -3.2, 3.2, '', 'P. Urquijo <phillip.urquijo@unimelb.edu.au>',
         r'B2VV planar angle of the $B^+ \to \rho^0 \rho^+$ (truth values)',
         'should follow the reference', '#chi (planar angle)'),
    ],
    description='B2VV Validation to check generator level distributions',
)
basf2.process(path)
