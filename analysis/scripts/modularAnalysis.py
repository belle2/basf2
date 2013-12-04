#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

main = create_path()


def inputMdst(filename, path=main):
    roinput = register_module('RootInput')
    roinput.param('inputFileName', filename)
    path.add_module(roinput)
    gearbox = register_module('Gearbox')
    path.add_module(gearbox)
    progress = register_module('Progress')
    path.add_module(progress)


def inputMdstList(filelist, path=main):
    roinput = register_module('RootInput')
    roinput.param('inputFileNames', filelist)
    path.add_module(roinput)
    gearbox = register_module('Gearbox')
    path.add_module(gearbox)
    progress = register_module('Progress')
    path.add_module(progress)


def outputMdst(filename, path=main):
    rooutput = register_module('RootOutput')
    rooutput.param('outputFileName', filename)
    rooutput.set_name('RootOutput_' + filename)
    path.add_module(rooutput)


def loadMCParticles(path=main):
    ploader = register_module('ParticleLoader')
    ploader.param('UseMCParticles', True)
    path.add_module(ploader)


def loadReconstructedParticles(path=main):
    ploader = register_module('ParticleLoader')
    ploader.param('UseMCParticles', False)
    path.add_module(ploader)


def selectParticle(
    list_name,
    PDGcode,
    criteria,
    persistent=False,
    path=main,
    ):

    pselect = register_module('ParticleSelector')
    pselect.set_name('ParticleSelector_' + list_name)
    pselect.param('PDG', PDGcode)
    pselect.param('ListName', list_name)
    pselect.param('Select', criteria)
    pselect.param('persistent', persistent)
    path.add_module(pselect)


def applyCuts(list_name, criteria, path=main):
    pselect = register_module('ParticleSelector')
    pselect.set_name('ParticleSelector_applyCuts_' + list_name)
    pselect.param('ListName', list_name)
    pselect.param('Select', criteria)
    path.add_module(pselect)


def makeParticle(
    list_name,
    PDGcode,
    list_of_lists,
    mL,
    mH,
    persistent=False,
    path=main,
    ):

    pmake = register_module('ParticleCombiner')
    pmake.set_name('ParticleCombiner_' + list_name)
    pmake.param('PDG', PDGcode)
    pmake.param('ListName', list_name)
    pmake.param('InputListNames', list_of_lists)
    pmake.param('MassCutLow', mL)
    pmake.param('MassCutHigh', mH)
    pmake.param('persistent', persistent)
    path.add_module(pmake)


def fitVertex(
    list_name,
    confidenceLevel,
    decay_string='',
    fitter='kfitter',
    fit_type='vertex',
    with_constraint='',
    path=main,
    ):

    pvfit = register_module('ParticleVertexFitter')
    pvfit.set_name('ParticleVertexFitter_' + list_name)
    pvfit.param('ListName', list_name)
    pvfit.param('ConfidenceLevel', confidenceLevel)
    pvfit.param('VertexFitter', fitter)
    pvfit.param('fitType', fit_type)
    pvfit.param('withConstraint', with_constraint)
    pvfit.param('decayString', decay_string)
    path.add_module(pvfit)


def printList(list_name, full, path=main):
    prlist = register_module('ParticlePrinter')
    prlist.set_name('ParticlePrinter_' + list_name)
    prlist.param('ListName', list_name)
    prlist.param('FullPrint', full)
    path.add_module(prlist)


def ntupleFile(file_name, path=main):
    ntmaker = register_module('NtupleMaker')
    ntmaker.set_name('NtupleMaker_ntupleFile_' + file_name)
    ntmaker.param('strFileName', file_name)
    path.add_module(ntmaker)


def ntupleTree(
    tree_name,
    list_name,
    tools,
    path=main,
    ):

    ntmaker = register_module('NtupleMaker')
    ntmaker.set_name('NtupleMaker_ntupleTree_' + list_name)
    ntmaker.param('strTreeName', tree_name)
    ntmaker.param('strListName', list_name)
    ntmaker.param('strTools', tools)
    path.add_module(ntmaker)


def findMCDecay(
    list_name,
    decay,
    persistent=False,
    path=main,
    ):

    decayfinder = register_module('MCDecayFinder')
    decayfinder.set_name('MCDecayFinder_' + list_name)
    decayfinder.param('strListName', list_name)
    decayfinder.param('strDecayString', decay)
    decayfinder.param('persistent', persistent)
    path.add_module(decayfinder)


def summaryOfLists(particleLists, path=main):
    particleStats = register_module('ParticleStats')
    particleStats.param('strParticleLists', particleLists)
    path.add_module(particleStats)


def matchMCTruth(list_name, path=main):
    mcMatch = register_module('MCMatching')
    mcMatch.set_name('MCMatching_' + list_name)
    mcMatch.param('ListName', list_name)
    path.add_module(mcMatch)


