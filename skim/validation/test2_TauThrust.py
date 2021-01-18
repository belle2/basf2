#!/usr/bin/env/python3
# -*-coding: utf-8-*-

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdPi
from stdPhotons import stdPhotons
from skim.taupair import TauThrust
from validation_tools.metadata import create_validation_histograms

"""
<header>
  <input>../TauThrust.udst.root</input>
  <output>TauThrust_Validation.root</output>
  <contact>kenji@hepl.phys.nagoya-u.ac.jp</contact>
</header>
"""

tauthrustskim = b2.Path()

ma.inputMdst('default', '../TauThrust.udst.root', path=tauthrustskim)

stdPi('all', path=tauthrustskim)
stdPhotons('all', path=tauthrustskim)

# set variables
TauThrust().additional_setup(path=tauthrustskim)

# add contact information to histogram
contact = "kenji@hepl.phys.nagoya-u.ac.jp"

# the variables that are printed out are:
create_validation_histograms(
    rootfile='TauGeneric_Validation.root',
    particlelist='',
    variables_1d=[
        ('nGoodTracksThrust', 7, 1, 8, '', contact, '', ''),
        ('visibleEnergyOfEventCMS', 40, 0, 12, '', contact, '', ''),
        ('thrust', 50, 0.75, 1, '', contact, '', '')],
    path=tauthrustskim)

# process
b2.process(tauthrustskim)
print(b2.statistics)
