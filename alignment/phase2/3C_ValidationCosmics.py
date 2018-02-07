#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# *****************************************************************************

# title           : 3_ValidationCosmics.py
# description     : Validation of cosmic tracks in phase II
# author          : Jakub Kandra (jakub.kandra@karlov.mff.cuni.cz)
# date            : 8. 2. 2018

# *****************************************************************************

from basf2 import *
from modularAnalysis import *
from CosmicAnalysis import CosmicAnalysis

inname = "reconstruction.root"

if len(sys.argv) == 3:
    inname = (sys.argv)[1]
    outname = (sys.argv)[2]

main = create_path()

main.add_module('RootInput', inputFileName=inname)

main.add_module('Gearbox')

main.add_module('Geometry')

CosmicAnalysis = CosmicAnalysis()
main.add_module(CosmicAnalysis)

progress = register_module('ProgressBar')
main.add_module(progress)

process(main)

# Print call statistics
print(statistics)
