#!/usr/bin/env/python3
# -*-coding: utf-8-*-

# Stuck? Ask for help at questions.belle2.org

# This is a mini validation script to check that the proper Mbc and deltaE
# cuts  #are indeed implemented as specified in the Bhadornic skim , the
# output of B2A351-Bhadronic_Skim_Standalon.py.
from basf2 import *
from modularAnalysis import *

# the variables that are printed out are: Mbc, deltaE, invariant mass of
# momentum of D meson, and invariant mass of D meson and  pion.
inputMdst('default', 'BHadronic_mixed.udst.root')
from variables import variables
variablesToHistogram(filename='validation1.root', decayString='B+:all', variables=[('Mbc', 100, 5.2, 5.3), ('deltaE', 100, -5, 5), (
    'daughter(0,InvM)', 100, 0, 1), ('daughter(0,p)', 100, 0, 4), ('daughter(1,InvM)', 100, 1.6, 2), ('daughter(1,p)', 100, 0, 4)])
process(analysis_main)
print(statistics)
