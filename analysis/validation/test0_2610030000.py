#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor: [ccbar Dstar2D0Pip D02KmPip]

"""
<header>
  <output>../2610030000.dst.root</output>
  <contact>Jake Bennett; jvbennett@cmu.edu</contact>
</header>
"""

from basf2 import *
from modularAnalysis import setupEventInfo
from generators import add_inclusive_continuum_generator
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output
from ROOT import Belle2
import glob

set_random_seed(150922)

# background (collision) files
bg = glob.glob('./BG/[A-Z]*.root')

# create path
main = create_path()

# generate continuum events
decayTable = Belle2.FileSystem.findFile('/decfiles/dec/2610030000.dec')
setupEventInfo(1000, main)
add_inclusive_continuum_generator(main, "ccbar", ["D0"], decayTable)

# detector simulation
# add_simulation(main, bkgfiles=bg)
add_simulation(main)

# reconstruction
add_reconstruction(main)


# Finally add mdst output
output_filename = "../2610030000.dst.root"
add_mdst_output(main, filename=output_filename)

# process events and print call statistics
process(main)
print(statistics)
