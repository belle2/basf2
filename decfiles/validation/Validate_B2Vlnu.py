#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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

variables.addAlias('helicity1', 'daughter(0, cosTheta)')
variables.addAlias('helicity2', 'daughter(1, cosTheta)')
variables.addAlias('planarAngle', 'cosTheta')
variables.addAlias('q2', 'E')

path = basf2.Path()
path.add_module('EventInfoSetter', evtNumList=[10000])
add_evtgen_generator(path, 'signal', basf2.find_file('decfiles/dec/Bu_Dst0enu.dec'))
findMCDecay('B+:sig', 'B+ => [anti-D*0 -> anti-D0 pi0] e+ nu_e', path=path)
create_validation_histograms(
    path, '../Validate_B2Vlnu.root', 'B+:sig',
    [
        ('helicity1', 50, -1.2, 1.2, '', 'P. Urquijo <phillip.urquijo@unimelb.edu.au>',
         r'B2Vlnu helicity angle of the $\bar{D^{*0} \to \bar{D^0} \pi^0$ in $B^+ \to \bar{D^{*0}} e^+ \nu_e$ (truth values)',
         'should follow the reference', 'cos#theta_{helicity}(V1)'),
        ('helicity2', 50, -1.2, 1.2, '', 'P. Urquijo <phillip.urquijo@unimelb.edu.au>',
         r'B2Vlnu helicity angle of the $e^+ \nu_e$ in $B^+ \to \bar{D^{*0}} e^+ \nu_e$ (truth values)',
         'should follow the reference', 'cos#theta_{helicity}(V1)'),
        ('planarAngle', 50, -3.2, 3.2, '', 'P. Urquijo <phillip.urquijo@unimelb.edu.au>',
         r'B2Vlnu planar angle of $B^+ \to \bar{D^{*0}} e^+ \nu_e$ (truth values)',
         'should follow the reference', '#chi (planar angle)'),
        ('q2', 50, -1, 12, '', 'P. Urquijo <phillip.urquijo@unimelb.edu.au>',
         r'B2Vlnu $q^2$ of $B^+ \to \bar{D^{*0}} e^+ \nu_e$ (truth values)',
         'should follow the reference', '#q^2'),
    ],
    description='B2Vlnu Validation to check generator level distributions',
)
basf2.process(path)
