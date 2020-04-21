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
from skim import dark
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

variables.addAlias('mu_p_cms', 'daughter(0, useCMSFrame(p))')
variables.addAlias('gamma_p_cms', 'daughter(1, useCMSFrame(p))')
variables.addAlias('mu_theta_lab', 'formula(daughter(0, theta)*180/3.1415927)')
variables.addAlias('gamma_theta_lab', 'formula(daughter(1, theta)*180/3.1415927)')
variables.addAlias('mu_gamma_delta_theta_cms', 'abs(formula('
                   + 'daughter(0, useCMSFrame(theta))*180/3.1415927 +'
                   + 'daughter(1, useCMSFrame(theta))*180/3.1415927 - 180))')

variablesHist2 = [
                   ('mup_p_cms', 100, 4, 6),
                   ('mum_p_cms', 100, 4, 6),
                   ('mup_theta_lab', 90, 0, 180),
                   ('mum_theta_lab', 90, 0, 180),
                   ('mup_mum_delta_theta_cms', 100, 0, 2)
                 ]

variablesHist1 = [
                   ('mu_p_cms', 100, 4, 6),
                   ('gamma_p_cms', 100, 1, 6),
                   ('mu_theta_lab', 90, 0, 180),
                   ('gamma_theta_lab', 90, 0, 180),
                   ('mu_gamma_delta_theta_cms', 100, 0, 2)
                 ]

# Output the variables to a ntuple
variablesToHistogram(
    'vpho:TwoTrackLeptonsForLuminosity2',
    variablesHist2,
    filename='../TwoTrackMuonsForLuminosity_Validation.root',
    path=TwoTrackLeptonsForLuminosity_path)
variablesToHistogram(
    'vpho:TwoTrackLeptonsForLuminosity1',
    variablesHist1,
    filename='../TwoTrackMuonsForLuminosity_Validation.root',
    path=TwoTrackLeptonsForLuminosity_path)

# Process the events
basf2.process(TwoTrackLeptonsForLuminosity_path)

# print out the summary
print(basf2.statistics)
