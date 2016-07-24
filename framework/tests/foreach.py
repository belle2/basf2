#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2

set_random_seed("something important")

path = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [5, 1])
eventinfosetter.param('runList', [0, 1])
eventinfosetter.param('expList', [0, 1])
path.add_module(eventinfosetter)

pgun = register_module('ParticleGun')
pgun.param('nTracks', 3)
path.add_module(pgun)


class TestModule(Module):

    """print some debug info"""

    def event(self):
        """reimplementation of Module::event()."""

        part = Belle2.PyStoreObj('MCParticle')
        B2INFO("MCPart: " + str(part.obj().getIndex()))

    def beginRun(self):
        """reimplementation of Module::beginRun()."""
        B2INFO("TestModule: beginRun()")


subeventpath = create_path()
subeventpath.add_module('EventInfoPrinter')
testmod = subeventpath.add_module(TestModule())
# read: for each  $objName   in $arrayName   run over $path
path.for_each('MCParticle', 'MCParticles', subeventpath)

path.add_module('PrintCollections')

print(path)
process(path)

# initialize/terminate once
assert statistics.get(pgun).calls(statistics.INIT) == 1
assert statistics.get(testmod).calls(statistics.INIT) == 1
assert statistics.get(pgun).calls(statistics.TERM) == 1
assert statistics.get(testmod).calls(statistics.TERM) == 1
# 2 runs
assert statistics.get(pgun).calls(statistics.BEGIN_RUN) == 2
assert statistics.get(testmod).calls(statistics.BEGIN_RUN) == 2
assert statistics.get(pgun).calls(statistics.END_RUN) == 2
assert statistics.get(testmod).calls(statistics.END_RUN) == 2
# 6 events, a 3 particles
assert statistics.get(pgun).calls(statistics.EVENT) == 6
assert statistics.get(testmod).calls(statistics.EVENT) == 3*6

# print(statistics)
