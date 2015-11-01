#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Christian Pulvermacher 2015

from fei import *
from basf2 import *
from modularAnalysis import *

# test wether we can actually get the default channels
particles_charged = get_default_channels(neutralB=False)
particles_neutral = get_default_channels(chargedB=False)
particles_hadronic = get_default_channels(semileptonicB=False)
particles_withextracut = get_default_channels(BlevelExtraCut='Mbc > 5.27')

particles_all = get_default_channels()
path = create_path()
path.add_module('RootInput')
feistate = fullEventInterpretation(None, path, particles_all)

assert len(particles_all) > len(particles_charged)
assert len(particles_all) > len(particles_neutral)
assert len(particles_all) > len(particles_hadronic)
assert len(particles_all) == len(particles_withextracut)


# show constructed path
print(feistate.path)

# no actual basf2 execution
