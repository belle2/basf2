#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

# importing of these definitions doesn't work yet!
# have to find a proper way to do that!


def Events(numlist, runlist, explist):
    evtmetagen = register_module('EvtMetaGen')
    evtmetagen.param({'EvtNumList': numlist, 'RunList': runlist, 'ExpList'
                     : explist})
    main.add_module(evtmetagen)
    progress = register_module('Progress')
    main.add_module(progress)


def Input(filename):
    roinput = register_module('RootInput')
    roinput.param('inputFileName', filename)
    main.add_module(roinput)


def Output(filename):
    rooutput = register_module('RootOutput')
    rooutput.param('outputFileName', filename)
    main.add_module(rooutput)


def loadMCParticles():
    ploader = register_module('ParticleLoader')
    ploader.param('UseMCParticles', True)
    main.add_module(ploader)


def loadReconstructedParticles():
    ploader = register_module('ParticleLoader')
    ploader.param('UseMCParticles', False)
    main.add_module(ploader)


def selectParticle(list_name, PDGcode, criteria):
    pselect = register_module('ParticleSelector')
    pselect.param('PDG', PDGcode)
    pselect.param('ListName', list_name)
    pselect.param('Select', criteria)
    main.add_module(pselect)


def applyCuts(list_name, criteria):
    pselect = register_module('ParticleSelector')
    pselect.param('ListName', list_name)
    pselect.param('Select', criteria)
    main.add_module(pselect)


def makeParticle(
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
    main.add_module(pmake)


def fitVertex(list_name, confidenceLevel):
    pvfit = register_module('ParticleVertexFitter')
    pvfit.param('ListName', list_name)
    pvfit.param('ConfidenceLevel', confidenceLevel)
    main.add_module(pvfit)


def printList(list_name, full):
    prlist = register_module('ParticlePrinter')
    prlist.param('ListName', list_name)
    prlist.param('FullPrint', full)
    main.add_module(prlist)


