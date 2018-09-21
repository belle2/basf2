#!/usr/bin/env/python3
# -*-coding: utf-8-*-

from basf2 import *
from modularAnalysis import *

inputMdst('default', 'TauLFV.udst.root')

copyLists('tau+:LFV', ['tau+:LFV_lgamma', 'tau+:LFV_lll', 'tau+:LFV_lP0',
                       'tau+:LFV_lS0', 'tau+:LFV_lV0', 'tau+:LFV_lhh', 'tau+:LFV_bnv'])

# the variables that are printed out are: Mbc, deltaE
from variables import variables
variablesToHistogram(
    filename='TauLFV_Validation.root',
    decayString='tau+:LFV',
    variables=[('Mbc', 100, 1.50, 2.00), ('deltaE', 120, -1.1, 1.1)]
)
process(analysis_main)
print(statistics)
