#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import shutil
from basf2 import *

from basf2 import Module
from ROOT import Belle2


class TestModule(Module):
    """Test to read relations."""

    def __init__(self):
        """constructor."""

        super(TestModule, self).__init__()

    def event(self):
        """reimplementation of Module::event().

        access all relations from/to MCParticles,
        any invalid indices should be caught.
        """

        mcparticles = Belle2.PyStoreArray('MCParticles')
        # this will generate an index internally, checking consistency
        # (will die with a FATAL if something goes wrong)
        from_relations = mcparticles[0].getRelationsFrom("ALL")
        to_relations = mcparticles[0].getRelationsTo("ALL")


eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10])

gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
pgun = register_module('ParticleGun')
g4sim = register_module('FullSim')
# make the simulation less noisy
g4sim.logging.log_level = LogLevel.ERROR

main = create_path()

main.add_module(eventinfosetter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(pgun)
main.add_module(g4sim)
main.add_module(TestModule())

process(main)
