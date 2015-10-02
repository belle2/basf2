#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Test that for_each also works with parallel processing
from basf2 import *
from ROOT import Belle2

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
modptr = subeventpath.add_module(TestModule())
modptr.set_property_flags(ModulePropFlags.PARALLELPROCESSINGCERTIFIED)
# read: for each  $objName   in $arrayName   run over $path
path.for_each('MCParticle', 'MCParticles', subeventpath)

path.add_module('PrintCollections')

print(path)
set_nprocesses(2)
process(path)
