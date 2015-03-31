#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

from basf2 import Module
from ROOT import Belle2


class TestModule(Module):

    """Test ECLCluster data"""

    def event(self):
        """reimplementation of Module::event().

        prints some ECLClusters
        """

        ll = Belle2.PyStoreArray('ECLClusters')
        for cluster in ll:
            print "index: %d" % (cluster.getArrayIndex())
            cluster.getError3x3().Print()
            cluster.getError4x4().Print()
            # cluster.getError7x7().Print()
            # this uses the ecl 7x7 matrix directly, so should be the same
            p = Belle2.Particle(cluster)
            p.getMomentumVertexErrorMatrix().Print()


input = register_module('RootInput')
eventinfo = register_module('EventInfoPrinter')

input.param('inputFileName', Belle2.FileSystem.findFile('analysis/tests/mdst_r10142.root'))
input.logging.log_level = LogLevel.WARNING

main = create_path()

main.add_module(input)
main.add_module(eventinfo)

main.add_module(TestModule())

# Process events
process(main, max_event=5)
