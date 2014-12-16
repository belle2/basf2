#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

from basf2 import Module
from ROOT import Belle2


class TestModule(Module):
    """Test to check if PIDLikelihood version <= 2 data can be read by newer versions of the software."""

    def event(self):
        """reimplementation of Module::event().

        prints some PIDlikelihood data
        """
        partList = [Belle2.Const.electron, Belle2.Const.muon, Belle2.Const.pion, Belle2.Const.kaon, Belle2.Const.proton]
        detList = [Belle2.Const.PIDDetectorSet(d) for d in (Belle2.Const.SVD, Belle2.Const.CDC, Belle2.Const.TOP, Belle2.Const.ARICH)]

        ll = Belle2.PyStoreArray('PIDLikelihoods')
        for l in ll:
            print "index: %d" % (l.getArrayIndex())
            for det in detList:
                logls = [str(l.getLogL(part, det)) for part in partList]
                print str(l.isAvailable(det)) + "\t" + " ".join(logls)


input = register_module('RootInput')
eventinfo = register_module('EventInfoPrinter')

input.param('inputFileName', Belle2.FileSystem.findFile('analysis/tests/mdst_r10142.root'))
input.logging.log_level = LogLevel.WARNING

main = create_path()

main.add_module(input)
main.add_module(eventinfo)

main.add_module(TestModule())

# Process events
process(main)
