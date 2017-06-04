#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
from basf2 import *
from ROOT import Belle2

reset_database()
use_database_chain()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
# use_local_database('localdb/database.txt')

main = create_path()
main.add_module('EventInfoSetter')
main.add_module('Cosmics')
main.add_module('Gearbox')
main.add_module('Geometry', excludedComponents=['MagneticField'])
main.add_module('FullSim')
main.add_module('PXDDigitizer')
main.add_module('SVDDigitizer')
main.add_module('PXDClusterizer')
main.add_module('SVDClusterizer')
main.add_module('CDCDigitizer')
main.add_module('RootOutput')
main.add_module('Progress')
process(main)
print(statistics)
