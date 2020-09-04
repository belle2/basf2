#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
  <input>../TCPV.udst.root</input>
  <output>TCPV_Validation.root</output>
  <contact>reem.rasheed@iphc.cnrs.fr</contact>
</header>
"""
import basf2 as b2
import modularAnalysis as ma
from stdV0s import stdKshorts
from stdPhotons import stdPhotons
from stdCharged import stdPi

from ROOT import gROOT
import sysconfig

gROOT.ProcessLine(".include " + sysconfig.get_path("include"))
# the variables that are printed out are: Mbc, deltaE, invariant mass of
# momentum of D meson, and invariant mass of D meson and  pion.

tcpvskimpath = b2.Path()


fileList = ['../TCPV.udst.root']
ma.inputMdstList('default', fileList, path=tcpvskimpath)

Kres = 'K_10'
stdKshorts(path=tcpvskimpath)
stdPhotons('loose', path=tcpvskimpath)
stdPi('all', path=tcpvskimpath)
ma.applyCuts('gamma:loose', '1.4 < E < 4', path=tcpvskimpath)

ma.reconstructDecay(Kres + ":all -> K_S0:merged pi+:all pi-:all ", "", path=tcpvskimpath)

ma.reconstructDecay("B0:signal -> " + Kres + ":all gamma:loose", "Mbc > 5.2 and deltaE < 0.5 and deltaE > -0.5", path=tcpvskimpath)

ma.matchMCTruth('B0:signal', path=tcpvskimpath)

variableshisto = [('deltaE', 100, -0.5, 0.5), ('Mbc', 100, 5.2, 5.3)]
ma.variablesToHistogram('B0:signal', variableshisto, filename='TCPV_Validation.root', path=tcpvskimpath)


b2.process(tcpvskimpath)
print(b2.statistics)
