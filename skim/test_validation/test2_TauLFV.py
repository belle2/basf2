#!/usr/bin/env/python3
# -*-coding: utf-8-*-

from basf2 import *
from modularAnalysis import *

"""
<header>
  <input>../TauLFV.udst.root</input>
  <output>../TauLFV_Validation.root</output>
  <contact>kenji@hepl.phys.nagoya-u.ac.jp</contact>
</header>
"""

inputMdst('MC9', '../TauLFV.udst.root')

from skim.taupair import *
tauList = TauLFVList(0)
copyLists('tau+:LFV', tauList)

# the variables that are printed out are: Mbc, deltaE
from variables import variables
variablesToHistogram(
    filename='../TauLFV_Validation.root',
    decayString='tau+:LFV',
    variables=[('Mbc', 100, 1.50, 2.00), ('deltaE', 120, -1.1, 1.1)],
    variables_2d=[('Mbc', 50, 1.50, 2.00, 'deltaE', 60, -1.1, 1.1)]
)
process(analysis_main)
print(statistics)
