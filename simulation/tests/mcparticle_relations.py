#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
from ROOT import Belle2

b2.logging.log_level = b2.LogLevel.ERROR


class TestModule(b2.Module):

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
        from_relations = mcparticles[0].getRelationsFrom("ALL")  # noqa
        to_relations = mcparticles[0].getRelationsTo("ALL")  # noqa


eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10])

gearbox = b2.register_module('Gearbox')
geometry = b2.register_module('Geometry')
pgun = b2.register_module('ParticleGun')
g4sim = b2.register_module('FullSim')
# make the simulation less noisy

main = b2.create_path()

main.add_module(eventinfosetter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(pgun)
main.add_module(g4sim)
main.add_module(TestModule())

b2.process(main)
