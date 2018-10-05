#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import basf2 as b2
from ROOT import Belle2

b2.set_random_seed("something important")

path = b2.Path()
path.add_module('EventInfoSetter', evtNumList=[5, 1], runList=[0, 1], expList=[0, 1])
pgun = path.add_module('ParticleGun', nTracks=3)


class TestModule(b2.Module):

    """print some debug info"""

    def event(self):
        """reimplementation of Module::event()."""

        part = Belle2.PyStoreObj('MCParticle')
        b2.B2INFO("MCPart: " + str(part.obj().getIndex()))

    def beginRun(self):
        """reimplementation of Module::beginRun()."""
        b2.B2INFO("TestModule: beginRun()")


for use_pp in [False, True]:
    if os.fork() == 0:
        subeventpath = b2.Path()
        testmod = TestModule()
        if use_pp:
            testmod.set_property_flags(b2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED)
        else:
            subeventpath.add_module('EventInfoPrinter')
        subeventpath.add_module(testmod)
        # read: for each  $objName   in $arrayName   run over $path
        path.for_each('MCParticle', 'MCParticles', subeventpath)
        path.add_module('PrintCollections', printForEvent=0)
        if use_pp:
            b2.set_nprocesses(2)
            b2.logging.log_level = b2.LogLevel.WARNING  # suppress output
            b2.process(path)
        else:
            print(path)
            b2.process(path)
        # print(statistics)
        # initialize/terminate once
        assert b2.statistics.get(pgun).calls(b2.statistics.INIT) == 1
        assert b2.statistics.get(testmod).calls(b2.statistics.INIT) == 1
        if not use_pp:
            # for pp, term b2.statistics are wrong, begin/end run are complicated
            assert b2.statistics.get(pgun).calls(b2.statistics.TERM) == 1
            assert b2.statistics.get(testmod).calls(b2.statistics.TERM) == 1
            # 2 runs
            assert b2.statistics.get(pgun).calls(b2.statistics.BEGIN_RUN) == 2
            assert b2.statistics.get(testmod).calls(b2.statistics.BEGIN_RUN) == 2
            assert b2.statistics.get(pgun).calls(b2.statistics.END_RUN) == 2
            assert b2.statistics.get(testmod).calls(b2.statistics.END_RUN) == 2
        # 6 events, a 3 particles
        assert b2.statistics.get(pgun).calls(b2.statistics.EVENT) == 6
        assert b2.statistics.get(testmod).calls(b2.statistics.EVENT) == 3 * 6

        sys.exit(0)
    retbytes = os.wait()[1]
    assert retbytes == 0
