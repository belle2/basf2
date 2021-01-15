#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../TwoTrackMuonsForLuminosity.udst.root</input>
  <output>../TwoTrackMuonsForLuminosity_Validation.root</output>
  <contact>zhouxy@buaa.edu.cn</contact>
</header>
"""

import basf2
from modularAnalysis import inputMdst
from variables import variables
from modularAnalysis import variablesToHistogram


TwoTrackLeptonsForLuminosity_path = basf2.Path()

# load input ROOT file
inputMdst('default', '../TwoTrackMuonsForLuminosity.udst.root', path=TwoTrackLeptonsForLuminosity_path)

variables.addAlias('mup_p_cms', 'daughter(0, useCMSFrame(p))')
variables.addAlias('mum_p_cms', 'daughter(1, useCMSFrame(p))')
variables.addAlias('mup_theta_lab', 'formula(daughter(0, theta)*180/3.1415927)')
variables.addAlias('mum_theta_lab', 'formula(daughter(1, theta)*180/3.1415927)')
variables.addAlias('mup_mum_delta_theta_cms', 'abs(formula('
                   + 'daughter(0, useCMSFrame(theta))*180/3.1415927 +'
                   + 'daughter(1, useCMSFrame(theta))*180/3.1415927 - 180))')
variables.addAlias('dau1_charge', 'daughter(1, charge)')

variablesHist = [
                  ('mup_p_cms', 100, 4, 6),
                  ('mum_p_cms', 100, 4, 6),
                  ('mup_theta_lab', 90, 0, 180),
                  ('mum_theta_lab', 90, 0, 180),
                  ('mup_mum_delta_theta_cms', 100, 0, 2),
                  ('dau1_charge', 6, -3, 3)
                ]

# Output the variables to histograms
variablesToHistogram(
    'vpho:TwoTrackLeptonsForLuminosity',
    variablesHist,
    filename='../TwoTrackMuonsForLuminosity_Validation.root',
    path=TwoTrackLeptonsForLuminosity_path)

# Process the events
basf2.process(TwoTrackLeptonsForLuminosity_path)

# print out the summary
print(basf2.statistics)
