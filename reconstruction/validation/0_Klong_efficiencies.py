#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>K_long_full validation_sample.root</output>
  <description>Generates particle gun Klong Events for Kl efficiency validation.\t
   Output is not very meaningful but sufficient to check for regression.</description>
  <contact>jo-frederik.krohn@desy.de</contact>
  <interval>nightly</interval>
</header>
"""

from basf2 import *
from simulation import *
from reconstruction import *
from ROOT import Belle2
import glob
from generators import add_evtgen_generator
set_random_seed('#fa1afe1')


main = create_path()


noEvents = 1000

bkg = glob.glob('/sw/belle2/bkg/*.root')
# bkg = '~/bkg/*.root'

main.add_module("EventInfoSetter", expList=1, runList=1, evtNumList=noEvents)

generator = register_module('ParticleGun')
generator.param('momentumParams', [0.05, 5.0])
generator.param('pdgCodes', [130, 321, 311, 2212, 2112, 211, 13, 11])
main.add_module(generator)

# add_evtgen_generator(main, finalstate='mixed')

# bkginput = register_module('BGOverlayInput')
# bkginput.param('inputFileNames', bkg)
# main.add_module(bkginput)

add_simulation(main, bkgfiles=bkg)
# add_simulation(main)

add_reconstruction(main)

# run a module to generate histograms to test pid performance
validation = register_module('KlongValidation')
validation.param('outPath', 'K_long_full validation_sample.root'.format(noEvents))
validation.param("KlId_cut", 0.1)
main.add_module(validation)

main.add_module(register_module('ProgressBar'))

process(main)
print(statistics)
