#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import ROOT
from ROOT import Belle2


class MomentumSetup(Module):

    def __init__(self):
        """ init """
        super(MomentumSetup, self).__init__()

    def event(self):
        """ Setup momentum before fitting """

        recoTracks = Belle2.PyStoreArray('RecoTracks')

        for track in recoTracks:

            definedMagMomentum = 10.0
            momentum = track.getMomentumSeed()
            magnitudeMomentum = momentum.Mag()
            newMomentumX = (momentum.Px() * definedMagMomentum) / magnitudeMomentum
            newMomentumY = (momentum.Py() * definedMagMomentum) / magnitudeMomentum
            newMomentumZ = (momentum.Pz() * definedMagMomentum) / magnitudeMomentum
            newMomentum = ROOT.TVector3(newMomentumX, newMomentumY, newMomentumZ)
            track.setPositionAndMomentum(track.getPositionSeed(), newMomentum)


class MomentumPrint(Module):

    def __init__(self):
        """ init """
        super(MomentumPrint, self).__init__()

    def event(self):
        """ Setup momentum before fitting """

        recoTracks = Belle2.PyStoreArray('RecoTracks')

        for track in recoTracks:

            momentum = track.getMomentumSeed().Mag()
            print('Momentum of RecoTrack is', momentum)


main = create_path()
# main.add_module('SeqRootInput')
main.add_module('RootInput')
main.add_module('Gearbox')

main.add_module('Geometry', components=['PXD', 'SVD'], excludedComponents=['MagneticField']
                # , useDB=False
                )

MomentumSetup = MomentumSetup()
main.add_module(MomentumSetup)

# MomentumPrint = MomentumPrint()
# main.add_module(MomentumPrint)

main.add_module('SetupGenfitExtrapolation', noiseBetheBloch=False, noiseCoulomb=False, noiseBrems=False)
main.add_module('HistoManager', histoFileName=Belle2.Environment.Instance().getOutputFileOverride())
main.add_module('MillepedeCollector',
                minPValue=-1.0,
                components=['VXDAlignment'],
                tracks=['RecoTracks'],
                particles=[],
                vertices=[],
                primaryVertices=[],
                calibrateVertex=False,
                useGblTree=True)

main.add_module('ProgressBar')
# main.add_module('Progress')
print_path(main)

process(main)
print(statistics)
