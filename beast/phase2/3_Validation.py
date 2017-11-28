#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# *****************************************************************************

# title           : 3_Reconstruction.py
# description     : Reconstruction De^+ e^- -> Y(4S)
# author          : Jakub Kandra (jakub.kandra@karlov.mff.cuni.cz)
# date            : 20. 6. 2017

# *****************************************************************************

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from modularAnalysis import *
from VXDHits import VXDHits

inname = "reconstruction.root"
outname = "reconstruction.root"

if len(sys.argv) == 3:
    inname = (sys.argv)[1]
    outname = (sys.argv)[2]

main = create_path()

main.add_module('RootInput', inputFileName=inname)

main.add_module('Gearbox')

main.add_module('Geometry')

# detector reconstruction
components = [
    'MagneticField',
    'BeamPipe',
    'PXD',
    'SVD',
    'CDC',
    'TOP',
    'ARICH',
    'BKLM',
    'EKLM',
    'ECL'
]

# reconstruction
# add_reconstruction(main, components, pruneTracks = False)
# add_reconstruction(main, pruneTracks = False)

VXDHits = VXDHits()
main.add_module(VXDHits)

# output
main.add_module('RootOutput', outputFileName=outname)

progress = register_module('ProgressBar')
main.add_module(progress)

process(main)

# Print call statistics
print(statistics)
