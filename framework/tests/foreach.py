#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2

set_random_seed("something important")

path = create_path()
path.add_module('EventInfoSetter', evtNumList=[5, 1], runList=[0, 1], expList=[0, 1])
pgun = path.add_module('ParticleGun', nTracks=3)


class TestModule(Module):

    """print some debug info"""

    def event(self):
        """reimplementation of Module::event()."""

        part = Belle2.PyStoreObj('MCParticle')
        B2INFO("MCPart: " + str(part.obj().getIndex()))

    def beginRun(self):
        """reimplementation of Module::beginRun()."""
        B2INFO("TestModule: beginRun()")


for use_pp in [False, True]:
    if os.fork() == 0:
        subeventpath = create_path()
        testmod = TestModule()
        if use_pp:
            testmod.set_property_flags(ModulePropFlags.PARALLELPROCESSINGCERTIFIED)
        else:
            subeventpath.add_module('EventInfoPrinter')
        subeventpath.add_module(testmod)
        # read: for each  $objName   in $arrayName   run over $path
        path.for_each('MCParticle', 'MCParticles', subeventpath)
        path.add_module('PrintCollections', printForEvent=0)
        if use_pp:
            set_nprocesses(2)
            logging.log_level = LogLevel.WARNING  # suppress output
            process(path)
        else:
            print(path)
            process(path)
        # print(statistics)
        # initialize/terminate once
        assert statistics.get(pgun).calls(statistics.INIT) == 1
        assert statistics.get(testmod).calls(statistics.INIT) == 1
        if not use_pp:
            # for pp, term statistics are wrong, begin/end run are complicated
            assert statistics.get(pgun).calls(statistics.TERM) == 1
            assert statistics.get(testmod).calls(statistics.TERM) == 1
            # 2 runs
            assert statistics.get(pgun).calls(statistics.BEGIN_RUN) == 2
            assert statistics.get(testmod).calls(statistics.BEGIN_RUN) == 2
            assert statistics.get(pgun).calls(statistics.END_RUN) == 2
            assert statistics.get(testmod).calls(statistics.END_RUN) == 2
        # 6 events, a 3 particles
        assert statistics.get(pgun).calls(statistics.EVENT) == 6
        assert statistics.get(testmod).calls(statistics.EVENT) == 3 * 6

        sys.exit(0)
    retbytes = os.wait()[1]
    assert retbytes == 0
