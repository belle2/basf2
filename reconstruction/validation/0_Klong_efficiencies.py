#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>KlongValidationData.root</output>
  <description>Generates particle gun Klong Events for Kl efficiency validation</description>
  <contact>jo-frederik.krohn@desy.de</contact>
  <interval>nightly</interval>
</header>
"""

from basf2 import *
from simulation import *
from reconstruction import *
from ROOT import Belle2

set_random_seed(56424)


main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10000])

main.add_module(eventinfosetter)


generator = register_module('ParticleGun')
generator.param('momentumParams', [0.05, 5.0])
generator.param('pdgCodes', [130, 321, 311, 2212, 2112, 211, 13, 11])
main.add_module(generator)

add_simulation(main, ['EKLM', 'BKLM'])
add_reconstruction(main, ['EKLM', 'BKLM'])


# run a module to generate histograms to test pid performance
validation = register_module('KlongValidation')
main.add_module(validation)

main.add_module(register_module('ProgressBar'))

process(main)
print(statistics)
