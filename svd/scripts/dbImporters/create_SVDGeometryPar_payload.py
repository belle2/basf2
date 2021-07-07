#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

############################################################################
#
# this script is used to create SVDGeometryPar payload that contains:
# 1. SVD Geometry (Phase3 by default)
# 2. characteristic of the sensors, for simulation (noise, ADUequivalent, Cint,...)
#
# by default the SVD.xml file is used (and all included xml from there)
#
############################################################################

import basf2 as b2


main = b2.create_path()
main.add_module('EventInfoSetter')

# Phase2 Geometry
# main.add_module("Gearbox", fileName='/geometry/Beast2_phase2.xml')

# Phase3 Geometry
main.add_module("Gearbox")

main.add_module('Geometry', components=['SVD'], createPayloads=True)

# NOTE: uncomment display to see what you get - but it does not work together with creating the payloads above
# main.add_module('Display', fullGeometry=True)

b2.process(main)
print(b2.statistics(b2.statistics.BEGIN_RUN))
