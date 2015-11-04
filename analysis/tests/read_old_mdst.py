#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2

set_random_seed("something important")
ievent = 0


class TestModule(Module):

    """Test to check if we can read old mdst files with newer versions of the software."""

    def event(self):
        """reimplementation of Module::event(). """
        global ievent

        if ievent > 50:
            return

        print("PIDLikelihoods:")
        partList = [Belle2.Const.electron, Belle2.Const.muon, Belle2.Const.pion, Belle2.Const.kaon, Belle2.Const.proton]
        detList = [
            Belle2.Const.PIDDetectorSet(d) for d in (
                Belle2.Const.SVD,
                Belle2.Const.CDC,
                Belle2.Const.TOP,
                Belle2.Const.ARICH)]

        ll = Belle2.PyStoreArray('PIDLikelihoods')
        for l in ll:
            print("index: %d" % (l.getArrayIndex()))
            for det in detList:
                logls = ["{:.12g}".format(l.getLogL(part, det)) for part in partList]
                print(str(l.isAvailable(det)) + "\t" + " ".join(logls))

        print("TrackFitResults:")
        trackfitresults = Belle2.PyStoreArray('TrackFitResults')
        for t in trackfitresults:
            print(t.getInfo())
            print("{:.12g}".format(t.getCotTheta()))

        if ievent < 10:  # limit output
            print("MCParticles:")
            mcparticles = Belle2.PyStoreArray('MCParticles')
            for m in mcparticles:
                print(m.getInfo())

        if ievent < 10:  # limit output
            print("ECLClusters:")
            clusters = Belle2.PyStoreArray('ECLClusters')
            for cluster in clusters:
                cluster.getError3x3().Print()

        ievent += 1


input = register_module('RootInput')
eventinfo = register_module('EventInfoPrinter')

input.param('inputFileName', Belle2.FileSystem.findFile('analysis/tests/mdst_r10142.root'))
input.logging.log_level = LogLevel.WARNING

main = create_path()

main.add_module(input)
main.add_module(eventinfo)

main.add_module(TestModule())

process(main)
