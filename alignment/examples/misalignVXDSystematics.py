#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Generate alignment payload for VXD

"""

import os
import sys
from basf2 import *
import ROOT
from ROOT import Belle2

import alignment.misalignment as misalignment

deformations = []
deformations.append(misalignment.Bowing(0.0003))
# deformations.append(misalignment.Curl(0.001, 0.)) # Curl can have two parameters
# deformations.append(misalignment.Twist(0.001))

main = create_path()
main.add_module('EventInfoSetter')
main.add_module('Gearbox')
main.add_module('Geometry', components=['PXD', 'SVD'])

main.add_module(misalignment.CreateMisalignmentModule(global_deformations=deformations))

main.add_module('Progress')

process(main)
print(statistics)
