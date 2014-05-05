#!/usr/bin/env python
# -*- coding: utf-8 -*-

# test saving of invariant mass histograms

import os
from basf2 import *
from ROOT import Belle2
from modularAnalysis import *


main = create_path()

rootinput = register_module('RootInput')
rootinput.param('inputFileName', Belle2.FileSystem.findFile('analysis/tests/mdst_r10142.root'))


main.add_module(rootinput)
main.add_module(register_module('EventInfoPrinter'))
main.add_module(register_module('ParticleLoader'))


selectParticle('K-', -321, [], path=main)
selectParticle('pi+', 211, [], path=main)

# not necessary?
selectParticle('g', 22, [], path=main)
makeParticle('pi0', 111, ['g', 'g'], 0.110, 0.150, path=main)
matchMCTruth('pi0', path=main)
#selectParticle('pi0', 111, [], path=main)

#makeParticle('D0', 421, ['K-', 'pi+', 'pi0'], 1.700, 2.000, path=main)
#matchMCTruth('D0', path=main)

combinedlist = 'D0'
#daughters = ['K-', 'pi+', 'pi0']
daughters = ['K-', 'pi+', 'pi0']
#a) save signal invariant mass using PreCutHistMaker
histmaker = register_module('PreCutHistMaker')
histmaker.param('PDG', 421)
histmaker.param('fileName', 'test_D0signalHist.root')
hist_params = (100, 0, 6.2)
histmaker.param('histParams', hist_params)
histmaker.param('inputListNames', daughters)
main.add_module(histmaker)


#b) make all  combinations and save only signal

#essentially no cuts here, which makes it slow
makeParticle(combinedlist, 421, daughters, 0.0, 100.0, path=main)
matchMCTruth(combinedlist, path=main)

ntupler = register_module('VariablesToNtuple')
ntupler.param('fileName', 'test_D0ntuple.root')
ntupler.param('variables', ['M', 'isSignal'])
ntupler.param('particleList', combinedlist)
main.add_module(ntupler)


class TestModule(Module):
    """print some debug info"""

    def initialize(self):
        """reimplementation of Module::initialize()."""

    def event(self):
        """reimplementation of Module::event()."""

        particles = Belle2.PyStoreArray('Particles')
        for p in particles:
            mc = p.getRelated("MCParticles")
            mcidx = ''
            mcpdg = ''
            more = ''
            if mc is not None:
                mcidx = mc.getArrayIndex()
                mcpdg = mc.getPDG()
                status = int(Belle2.analysis.particleMCMatchStatus(p))
                more = 'status=' + str(status) + ' '
                mom = mc.getMother()
                if mom is not None:
                    more = more + 'mom: ' + str(mom.getArrayIndex())
                daughters = p.getDaughterIndices()
                if daughters:
                    more = more + ' daughter IDs:'
                for d in daughters:
                    more = more + ' ' + str(d)
            print repr(p.getArrayIndex()).rjust(6), repr(p.getPDGCode()).rjust(6), repr(mcidx).rjust(6), repr(mcpdg).rjust(6), more


#main.add_module(TestModule())
#main.add_module(register_module('Interactive'))

process(main)

print statistics

#compare PreCutHistMaker output to that saved directly

from ROOT import TFile
from ROOT import TNtuple
from ROOT import TH1F


def checkHistograms(name, selection):
    """
    compare histogram 'name' with distribution of M with condition 'selection'
    """
    histfile = TFile('test_D0signalHist.root')
    outputHist = histfile.Get(name)
    hmSignals = outputHist.GetEntries()
    print "entries in hist " + name + ": " + str(hmSignals)

    ntuplefile = TFile('test_D0ntuple.root')
    ntuple = ntuplefile.Get('ntuple')

    trueSignals = ntuple.GetEntries(selection)
    print "from ntuple (" + selection + "): " + str(trueSignals)

    if trueSignals == 0:
        B2FATAL("No events found. Reconstruction broken? Too few events in sample?")

    if trueSignals != hmSignals:
        B2FATAL("Mismatch in number of entries! (ParticleCombiner + MCMatching: " + str(trueSignals) + ", PreCutHistMaker: " + str(hmSignals))

    hist_from_ntuple = TH1F("ntuplehist", "ntuplehist", hist_params[0], hist_params[1], hist_params[2])
    ntuple.Project("ntuplehist", "M", selection)

#compare histograms by subtracting them (slightly ugly, not sure if there's a better way)
    hist_from_ntuple.Add(outputHist, -1.0)
    maximum = hist_from_ntuple.GetMaximum()
    minimum = hist_from_ntuple.GetMinimum()
    if maximum != 0.0 or minimum != 0.0:
        B2FATAL("Histograms differ!")

checkHistograms("signal", "isSignal > 0.5")
checkHistograms("all", "")


print "Test passed, cleaning up."
#cleanup
os.remove('test_D0ntuple.root')
os.remove('test_D0signalHist.root')
