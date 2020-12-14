#!/usr/bin/env/python3
# -*-coding: utf-8-*-

from skim.taupair import TauLFV
import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdE, stdK, stdMu, stdPi, stdPr
from stdPhotons import stdPhotons
from skim.standardlists.lightmesons import loadStdAllRho0, loadStdAllKstar0, loadStdAllPhi, loadStdAllF_0
from validation_tools.metadata import create_validation_histograms

"""
<header>
  <input>../TauLFV.udst.root</input>
  <output>TauLFV_Validation.root</output>
  <contact>kenji@hepl.phys.nagoya-u.ac.jp</contact>
</header>
"""

taulfvskim = b2.Path()

ma.inputMdst('default', '../TauLFV.udst.root', path=taulfvskim)

stdE('all', path=taulfvskim)
stdMu('all', path=taulfvskim)
stdPi('all', path=taulfvskim)
stdK('all', path=taulfvskim)
stdPr('all', path=taulfvskim)
stdPhotons('all', path=taulfvskim)
loadStdAllRho0(path=taulfvskim)
loadStdAllKstar0(path=taulfvskim)
loadStdAllPhi(path=taulfvskim)
loadStdAllF_0(path=taulfvskim)

tauList = TauLFV().get_skim_list_names()
ma.copyLists('tau+:LFV', tauList, path=taulfvskim)

# add contact information to histogram
contact = "kenji@hepl.phys.nagoya-u.ac.jp"

# the variables that are printed out are: M, deltaE
create_validation_histograms(
    rootfile='TauLFV_Validation.root',
    particlelist='tau+:LFV',
    variables_1d=[
        ('M', 100, 1.00, 2.00, '', contact, '', ''),
        ('deltaE', 120, -1.6, 0.6, '', contact, '', '')],
    variables_2d=[('M', 50, 1.00, 2.00, 'deltaE', 60, -1.6, 0.6, '', contact, '', '')],
    path=taulfvskim)

# process
b2.process(taulfvskim)
print(b2.statistics)
