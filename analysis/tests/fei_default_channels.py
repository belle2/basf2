#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Christian Pulvermacher 2015

from fei import *
from basf2 import *
from modularAnalysis import *

# test wether we can actually get the default channels
particles_hadronic = get_default_channels(semileptonic=False)
particles_noklong = get_default_channels(KLong=False)
particles_withextracut = get_default_channels(B_extra_cut='Mbc > 5.27')

particles_all = get_default_channels()

assert len(particles_all) > len(particles_hadronic)
assert len(particles_all) > len(particles_noklong)
assert len(particles_all) == len(particles_withextracut)

path = create_path()
path.add_module('RootInput')
feistate = fullEventInterpretation(None, path, particles_all, databasePrefix='FEI_TEST')

# show constructed path
print(feistate.path)

# no actual basf2 execution
