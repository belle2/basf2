#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../TwoTrackPions.udst.root</input>
  <output>../TwoTrackPions_Validation.root</output>
  <contact>zhouxy@buaa.edu.cn</contact>
</header>
"""

import basf2
from modularAnalysis import inputMdst
from skim import dark
from variables import variables
from modularAnalysis import variablesToHistogram

TwoTrackPions_path = basf2.Path()

# load input ROOT file
inputMdst('default', '../TwoTrackPions.udst.root', path=TwoTrackPions_path)

variables.addAlias('pip_p_cms', 'daughter(0, useCMSFrame(p))')
variables.addAlias('pim_p_cms', 'daughter(1, useCMSFrame(p))')
variables.addAlias('pip_theta_lab', 'formula(daughter(0, theta)*180/3.1415927)')
variables.addAlias('pim_theta_lab', 'formula(daughter(1, theta)*180/3.1415927)')
variables.addAlias('Mpipi', 'M')

variablesHist = [
                  ('pip_p_cms', 60, 0, 6),
                  ('pim_p_cms', 60, 0, 6),
                  ('pip_theta_lab', 90, 0, 180),
                  ('pim_theta_lab', 90, 0, 180),
                  ('Mpipi', 80, 0., 2.)
                ]

# Output the variables to a ntuple
variablesToHistogram('vpho:TwoTrackPions', variablesHist, filename='../TwoTrackPions_Validation.root', path=TwoTrackPions_path)

# Process the events
basf2.process(TwoTrackPions_path)

# print out the summary
print(basf2.statistics)
