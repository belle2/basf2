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


def outputMdst(filename, path=main):
    rooutput = register_module('RootOutput')
    rooutput.param('outputFileName', filename)
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
    pselect.param('PDG', PDGcode)
    pselect.param('ListName', list_name)
    pselect.param('Select', criteria)
    pselect.param('persistent', persistent)
    path.add_module(pselect)


def applyCuts(list_name, criteria, path=main):
    pselect = register_module('ParticleSelector')
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
    pmake.param('PDG', PDGcode)
    pmake.param('ListName', list_name)
    pmake.param('InputListNames', list_of_lists)
    pmake.param('MassCutLow', mL)
    pmake.param('MassCutHigh', mH)
    pmake.param('persistent', persistent)
    path.add_module(pmake)


def fitVertex(list_name, confidenceLevel, path=main):
    pvfit = register_module('ParticleVertexFitter')
    pvfit.param('ListName', list_name)
    pvfit.param('ConfidenceLevel', confidenceLevel)
    path.add_module(pvfit)


def printList(list_name, full, path=main):
    prlist = register_module('ParticlePrinter')
    prlist.param('ListName', list_name)
    prlist.param('FullPrint', full)
    path.add_module(prlist)


def ntupleFile(file_name, path=main):
    ntmaker = register_module('NtupleMaker')
    ntmaker.param('strFileName', file_name)
    path.add_module(ntmaker)


def ntupleTree(
    tree_name,
    list_name,
    tools,
    path=main,
    ):

    ntmaker = register_module('NtupleMaker')
    ntmaker.param('strTreeName', tree_name)
    ntmaker.param('strListName', list_name)
    ntmaker.param('strTools', tools)
    path.add_module(ntmaker)


