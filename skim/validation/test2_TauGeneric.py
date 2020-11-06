#!/usr/bin/env/python3
# -*-coding: utf-8-*-

import ROOT
import basf2 as b2
import modularAnalysis as ma
from variables import variables as vm
from stdCharged import stdPi
from stdPhotons import stdPhotons
from skim.taupair import TauGeneric
from validation_tools.metadata import create_validation_histograms

"""
<header>
  <input>../TauGeneric.udst.root</input>
  <output>TauGeneric_Validation.root</output>
  <contact>kenji@hepl.phys.nagoya-u.ac.jp</contact>
</header>
"""

taugenericskim = b2.Path()

ma.inputMdst('default', '../TauGeneric.udst.root', path=taugenericskim)

stdPi('all', path=taugenericskim)
stdPhotons('all', path=taugenericskim)

# set variables
TauGeneric().additional_setup(path=taugenericskim)

vm.addAlias('Theta_miss', 'formula(missingMomentumOfEvent_theta*180/3.14159)')

# add contact information to histogram
contact = "kenji@hepl.phys.nagoya-u.ac.jp"

# the variables that are printed out are:
create_validation_histograms(
    rootfile='TauGeneric_Validation.root',
    particlelist='',
    variables_1d=[
        ('nGoodTracks', 7, 1, 8, '', contact, '', ''),
        ('visibleEnergyOfEventCMS', 40, 0, 12, '', contact, '', ''),
        ('E_ECLtrk', 70, 0, 7, '', contact, '', ''),
        ('maxPt', 30, 0, 6, '', contact, '', ''),
        ('invMS1', 60, 0, 3, '', contact, '', '', '', ''),
        ('invMS2', 60, 0, 3, '', contact, '', ''),
        ('Theta_miss', 30, 0, 180, '', contact, '', '')],
    variables_2d=[('invMS1', 30, 0, 3, 'invMS2', 30, 0, 3, '', contact, '', '')],
    path=taugenericskim)

# process
b2.process(taugenericskim)
print(b2.statistics)
