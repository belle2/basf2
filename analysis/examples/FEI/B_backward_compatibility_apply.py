#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2017

from basf2 import *
from modularAnalysis import *

# Use backward compatibility layer in order to use an old FEI training with a newer basf2 version.
# Which functions you have to call exactly depends on you basf2 version.
from fei import backward_compatibility_layer
backward_compatibility_layer.pid_renaming_oktober_2017()


# In case you have problems with the conditions database you can use the localdb of the FEI directly
# use_local_database('/home/belle2/tkeck/feiv4/Belle2_2017_MC7_Track14_2/localdb/database.txt',
#                     '/home/belle2/tkeck/feiv4/Belle2_2017_MC7_Track14_2/localdb/', True, LogLevel.WARNING)

path = create_path()
inputMdstList('MC7', [], path)

import fei
particles = fei.get_default_channels()
# You can turn on and off individual parts of the reconstruction without retraining!
# particles = fei.get_default_channels(hadronic=True, semileptonic=True, chargedB=True, neutralB=True)

configuration = fei.config.FeiConfiguration(prefix='FEIv4_2017_MC7_Track14_2', training=False, monitor=False)
feistate = fei.get_path(particles, configuration)
path.add_path(feistate.path)


process(path)
print(statistics)
