#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# *****************************************************************************

# title           : generatePhase2VXDMisalignment.py
# description     : Generation VXD misalignment in phase 2 geometry
# author          : Jakub Kandra (jakub.kandra@karlov.mff.cuni.cz)
# date            : 8. 2. 2018

# *****************************************************************************


import os
import sys
from basf2 import *
from ROOT import Belle2

from alignment.utils import AlignmentGeneratorConfig

alignment = AlignmentGeneratorConfig()

"""
-------------------------
Misalignment for Phase II
-------------------------
- Used sensors: 1.1.0, 2.1.0, 3.1.0, 4.1.0, 5.1.0, 6.1.0
- alignment.set_u('0.0.0', 0.01) -> set U coordinate 100 um
- alignment.gen_v_gaus('0.0.0', 0.01) -> set V from normal distribution dispersion 100 um
- alignment.gen_w_uniform('0.0.0', 0.01) -> set W uniform ditribution from interval +/- 100 um
- alignment.set_alpha('0.0.0', 0.001) -> set ALPHA coordinate 1 mrad
- alignment.gen_beta_gaus('0.0.0', 0.001) -> set BETA from normal distribution dispersion 1 mrad
- alignment.gen_gamma_uniform('0.0.0', 0.001) -> set GAMMA uniform ditribution from interval +/- 1 mrad
- alignment.set_all('0.0.0', 0.0) -> generate "empty" alignment with zeros
"""

alignment.set_all('0.1.0', 0.0)

main = create_path()
main.add_module('EventInfoSetter')
main.add_module('Gearbox')
main.add_module('Geometry', components=['PXD', 'SVD'])
main.add_module('AlignmentGenerator', payloadIov=[0, 0, -1, -1], payloadName="", data=alignment.get_data(), createPayload=True)
main.add_module('Progress')

process(main)
print(statistics)
