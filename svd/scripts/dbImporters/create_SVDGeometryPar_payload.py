#!/usr/bin/env python3
# -*- coding: utf-8 -*-

############################################################################
#
# this script is used to create SVDGeometryPar payload that contains:
# 1. SVD Geometry (Phase3 by default)
# 2. characteristic of the sensors, for simulation (noise, ADUequivalent, Cint,...)
#
# by default the SVD.xml file is used (and all included xml from there)
#
############################################################################

from basf2 import *

use_database_chain()
db = "localDB_SVDGeometryPar"
use_local_database(db + "/database.txt", db)

main = create_path()
main.add_module('EventInfoSetter')

# Phase2 Geometry
# main.add_module("Gearbox", fileName='/geometry/Beast2_phase2.xml')

# Phase3 Geometry
main.add_module("Gearbox")

main.add_module('Geometry', components=['SVD'], createPayloads=True)

# NOTE: uncomment display to see what you get - but it does not work together with creating the payloads above
# main.add_module('Display', fullGeometry=True)

process(main)
print(statistics(statistics.BEGIN_RUN))
