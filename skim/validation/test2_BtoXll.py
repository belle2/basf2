#!/usr/bin/env/python3
# -*-coding: utf-8-*-

"""
<header>
  <input>../BtoXll.udst.root</input>
  <output>../BtoXll_Validation.root</output>
  <contact>soumen.halder@tifr.res.in</contact>
</header>
"""
__author__ = "Soumen Halder, Trevor Shillington"

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdK
from stdCharged import stdMu

# the variables that are printed out are: Mbc and deltaE

my_path = b2.Path()

my_email = 'Trevor Shillington <trshillington@hep.physics.mcgill.ca>'

histogram_filename = 'BtoXll_Validation.root'

fileList = ['../BtoXll.udst.root']
ma.inputMdstList('default', fileList, path=my_path)

stdK(listtype='good', path=my_path)
stdMu(listtype='good', path=my_path)
ma.reconstructDecay("B+:signal -> K+:good mu+:good mu-:good", "Mbc > 5.2 and deltaE < 0.5 and deltaE > -0.5", path=my_path)
ma.matchMCTruth('B+:signal', path=my_path)

create_validation_histograms(
    rootfile=histogram_filename,
    particlelist='B+:signal',
    variables_1d=[
        ('deltaE', 100, -0.5, 0.5, 'Signal B deltaE', my_email,
         'deltaE of the Signal B', '', 'deltaE [GeV]', 'Candidates'),
        ('Mbc', 100, 5.2, 5.3, 'Signal B Mbc', my_email,
         'Mbc of the signal B', '', 'Mbc [GeV/c^2]', 'Candidates')],
    variables_2d=[],
    path=my_path)

b2.process(my_path)
print(b2.statistics)
