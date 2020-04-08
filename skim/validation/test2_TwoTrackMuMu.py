#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../TwoTrackMuMu.udst.root</input>
  <output>../TwoTrackMuMu_Validation.root</output>
  <contact>zhouxy@buaa.edu.cn</contact>
</header>
"""

import basf2
from modularAnalysis import inputMdst
from skim import dark
from variables import variables
from modularAnalysis import variablesToHistogram


TwoTrackEEMuMu_path = basf2.Path()

# load input ROOT file
inputMdst('default', '../TwoTrackMuMu.udst.root', path=TwoTrackEEMuMu_path)

variables.addAlias('mup_p_cms', 'daughter(0, useCMSFrame(p))')
variables.addAlias('mum_p_cms', 'daughter(1, useCMSFrame(p))')
variables.addAlias('mup_theta_lab', 'formula(daughter(0, theta)*180/3.1415927)')
variables.addAlias('mum_theta_lab', 'formula(daughter(1, theta)*180/3.1415927)')
variables.addAlias('delta_theta_cms', 'abs(formula('
                   + 'daughter(0, useCMSFrame(theta))*180/3.1415927 +'
                   + 'daughter(1, useCMSFrame(theta))*180/3.1415927 - 180))')

variablesHist = [
                  ('mup_p_cms', 100, 1, 6),
                  ('mum_p_cms', 100, 1, 6),
                  ('mup_theta_lab', 90, 0, 180),
                  ('mum_theta_lab', 90, 0, 180),
                  ('delta_theta_cms', 100, 0, 10)
                ]

# Output the variables to a ntuple
variablesToHistogram('vpho:TwoTrackEEMuMu', variablesHist, filename='../TwoTrackMuMu_Validation.root', path=TwoTrackEEMuMu_path)

# Process the events
basf2.process(TwoTrackEEMuMu_path)

# print out the summary
print(basf2.statistics)
