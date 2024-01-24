#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Generate alignment payload for VXD

"""

import basf2 as b2

import alignment.misalignment as misalignment

deformations = []
deformations.append(misalignment.Bowing(0.0003))
# deformations.append(misalignment.Curl(0.001, 0.)) # Curl can have two parameters
# deformations.append(misalignment.Twist(0.001))

main = b2.create_path()
main.add_module('EventInfoSetter')
main.add_module('Gearbox')
main.add_module('Geometry', components=['PXD', 'SVD'])

main.add_module(misalignment.CreateMisalignmentModule(global_deformations=deformations))

main.add_module('Progress')

b2.process(main)
print(b2.statistics)
