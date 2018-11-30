#!/usr/bin/env/python3
# -*-coding: utf-8-*-

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *
from stdPi0s import *
from stdV0s import *
from skim.standardlists.lightmesons import *

"""
<header>
  <input>../TauLFV.udst.root</input>
  <output>../TauLFV_Validation.root</output>
  <contact>kenji@hepl.phys.nagoya-u.ac.jp</contact>
</header>
"""

inputMdst('MC9', '../TauLFV.udst.root')

stdPi('loose')
stdK('loose')
stdPr('loose')
stdE('loose')
stdMu('loose')
stdPhotons('loose')
stdPi0s('loose')
loadStdSkimPi0()
stdKshorts()
loadStdLightMesons()

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
