#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *


def Events(
    path,
    numlist,
    runlist,
    explist,
    ):

    evtmetagen = register_module('EvtMetaGen')
    evtmetagen.param({'EvtNumList': numlist, 'RunList': runlist, 'ExpList'
                     : explist})
    main.add_module(evtmetagen)
    progress = register_module('Progress')
    path.add_module(progress)


def Input(path, filename):
    roinput = register_module('RootInput')
    roinput.param('inputFileName', filename)
    path.add_module(roinput)


def Output(path, filename):
    rooutput = register_module('RootOutput')
    rooutput.param('outputFileName', filename)
    path.add_module(rooutput)


def loadMCParticles(path):
    ploader = register_module('ParticleLoader')
    ploader.param('UseMCParticles', True)
    path.add_module(ploader)


def loadReconstructedParticles(path):
    ploader = register_module('ParticleLoader')
    ploader.param('UseMCParticles', False)
    path.add_module(ploader)


def selectParticle(
    path,
    list_name,
    PDGcode,
    criteria,
    ):

    pselect = register_module('ParticleSelector')
    pselect.param('PDG', PDGcode)
    pselect.param('ListName', list_name)
    pselect.param('Select', criteria)
    path.add_module(pselect)


def applyCuts(path, list_name, criteria):
    pselect = register_module('ParticleSelector')
    pselect.param('ListName', list_name)
    pselect.param('Select', criteria)
    path.add_module(pselect)


def makeParticle(
    path,
    list_name,
    PDGcode,
    list_of_lists,
    mL,
    mH,
    ):

    pmake = register_module('ParticleCombiner')
    pmake.param('PDG', PDGcode)
    pmake.param('ListName', list_name)
    pmake.param('InputListNames', list_of_lists)
    pmake.param('MassCutLow', mL)
    pmake.param('MassCutHigh', mH)
    path.add_module(pmake)


def fitVertex(path, list_name, confidenceLevel):
    pvfit = register_module('ParticleVertexFitter')
    pvfit.param('ListName', list_name)
    pvfit.param('ConfidenceLevel', confidenceLevel)
    path.add_module(pvfit)


def printList(path, list_name, full):
    prlist = register_module('ParticlePrinter')
    prlist.param('ListName', list_name)
    prlist.param('FullPrint', full)
    path.add_module(prlist)


