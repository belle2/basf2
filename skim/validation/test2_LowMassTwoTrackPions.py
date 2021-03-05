#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../LowMassTwoTrackPions.udst.root</input>
  <output>../LowMassTwoTrackPions_Validation.root</output>
  <contact>zhouxy@buaa.edu.cn</contact>
</header>
"""

import basf2
from modularAnalysis import inputMdst
from variables import variables
from modularAnalysis import copyLists, variablesToHistogram
from skim.lowMulti import LowMassTwoTrackPions

LowMassTwoTrackPions_path = basf2.Path()

# load input ROOT file
inputMdst('default', '../LowMassTwoTrackPions.udst.root', path=LowMassTwoTrackPions_path)

variables.addAlias('pip_p_cms', 'daughter(0, useCMSFrame(p))')
variables.addAlias('pim_p_cms', 'daughter(1, useCMSFrame(p))')
variables.addAlias('gamma_E_cms', 'daughter(2, useCMSFrame(E))')
variables.addAlias('pip_theta_lab', 'formula(daughter(0, theta)*180/3.1415927)')
variables.addAlias('pim_theta_lab', 'formula(daughter(1, theta)*180/3.1415927)')
variables.addAlias('gamma_theta_lab', 'formula(daughter(2, theta)*180/3.1415927)')
variables.addAlias('Mpipi', 'daughterInvM(0,1)')

variablesHist = [
                  ('pip_p_cms', 60, 0, 6),
                  ('pim_p_cms', 60, 0, 6),
                  ('gamma_E_cms', 60, 0, 6),
                  ('pip_theta_lab', 90, 0, 180),
                  ('pim_theta_lab', 90, 0, 180),
                  ('gamma_theta_lab', 90, 0, 180),
                  ('Mpipi', 80, 0., 4.),
                  ('M', 60, 6., 12.)
                ]

# Merge all skim lists into a single list
copyLists('vpho:LowMassTwoTrackPions', LowMassTwoTrackPions().get_skim_list_names(), path=path)

# Output the variables to histograms
variablesToHistogram(
    'vpho:LowMassTwoTrack',
    variablesHist,
    filename='../LowMassTwoTrackPions_Validation.root',
    path=LowMassTwoTrackPions_path)

# Process the events
basf2.process(LowMassTwoTrackPions_path)

# print out the summary
print(basf2.statistics)
