#!/usr/bin/env python
# -*- coding: utf-8 -*-

# test saving of invariant mass histograms -- output of a) (PreCutHistMaker) and b) (ParticleCombiner -> MCMatching) should be identical

import os
from basf2 import *
from ROOT import Belle2
from modularAnalysis import *


main = create_path()

rootinput = register_module('RootInput')
rootinput.param('inputFileName', Belle2.FileSystem.findFile('analysis/tests/mdst_r10142.root'))


main.add_module(rootinput)
main.add_module(register_module('EventInfoPrinter'))


fillParticleList('K-', 'Kid > 0.1', path=main)
fillParticleList('pi+', 'piid > 0.1', path=main)

fillParticleList('gamma', '', path=main)
reconstructDecay('pi0 -> gamma gamma', '0.11 < M < 0.15', 0, path=main)
matchMCTruth('pi0', path=main)

combinedlist = 'D0'
daughters = ['K-', 'pi+', 'pi0']

#a) save signal invariant mass using PreCutHistMaker
histmaker = register_module('PreCutHistMaker')
histmaker.param('decayString', 'D0 -> K- pi+ pi0')
histmaker.param('fileName', 'test_D0signalHist.root')
hist_params = (100, 0, 6.2)
histmaker.param('histParams', hist_params)
histmaker.param('variable', 'M')
main.add_module(histmaker)


#b) make all  combinations and save only signal

#essentially no cuts here, which makes it slow
reconstructDecay('D0 -> K- pi+ pi0', '', 0, path=main)
matchMCTruth(combinedlist, path=main)

ntupler = register_module('VariablesToNtuple')
ntupler.param('fileName', 'test_D0ntuple.root')
ntupler.param('variables', ['M', 'isSignal'])
ntupler.param('particleList', combinedlist)
main.add_module(ntupler)

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

checkHistograms("allD0", "")
checkHistograms("signalD0", "isSignal > 0.5")


print "Test passed, cleaning up."
#cleanup
os.remove('test_D0ntuple.root')
os.remove('test_D0signalHist.root')
