#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
  <input>../TDCP.udst.root</input>
  <output>../TCPV_Validation.root</output>
  <contact>reem.rasheed@iphc.cnrs.fr</contact>
</header>
"""
import basf2
import modularAnalysis as ma
from stdV0s import stdKshorts
from stdPhotons import *
from stdCharged import *

from variables import variables
from ROOT import gROOT, TFile, TTree
import sysconfig

gROOT.ProcessLine(".include " + sysconfig.get_path("include"))
# the variables that are printed out are: Mbc, deltaE, invariant mass of
# momentum of D meson, and invariant mass of D meson and  pion.

tcpvskimpath = Path()


fileList = ['TCPV.udst.root']
inputMdstList('default', fileList, path=tcpvskimpath)

Kres = 'K_10'
stdKshorts(path=tcpvskimpath)
stdPhotons('loose', path=tcpvskimpath)
stdPi('all', path=tcpvskimpath)
applyCuts('gamma:loose', '1.4 < E < 4', path=tcpvskimpath)

reconstructDecay(Kres + ":all -> K_S0:all pi+:all pi-:all ", "", path=tcpvskimpath)

reconstructDecay("B0:signal -> " + Kres + ":all gamma:loose", "Mbc > 5.2 and deltaE < 0.5 and deltaE > -0.5", path=tcpvskimpath)

ma.matchMCTruth('B0:signal', path=tcpvskimpath)

variableshisto = [('deltaE', 100, -0.5, 0.5), ('Mbc', 100, 5.2, 5.3)]
variablesToHistogram('B0:signal', variableshisto, filename='TCPV_Validation.root', path=tcpvskimpath)


process(tcpvskimpath)
print(statistics)
