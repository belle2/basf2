#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Thomas Keck 2014
#

from basf2 import *
import FR_utility
import modularAnalysis

import pdg


def SelectParticleList(path, particleName, explicitCuts):
    """
    Creates a ParticleList gathering up all particles with the given particleName
        @param path the basf2 path
        @param particleName returned key is named ParticleList_{particleName} corresponding ParticleList is stored as {particleName}:{hash}
    """
    userLabel = FR_utility.createHash(particleName, explicitCuts)
    outputList = particleName + ':' + userLabel
    modularAnalysis.selectParticle(outputList, explicitCuts, path=path)
    return {'ParticleList_' + particleName: outputList, 'ParticleList_' + pdg.conjugate(particleName): pdg.conjugate(particleName) + ':' + userLabel}


def CopyParticleLists(path, particleName, inputLists):
    """
    Creates a ParticleList gathering up all particles in the given inputLists
        @param path the basf2 path
        @param particleName returned key is named ParticleList_{particleName} corresponding ParticleList is stored as {particleName}:{hash}
        @param inputLists names of inputLists which are copied to the new list
    """
    inputLists = FR_utility.removeNones(inputLists)
    if inputLists == []:
        return {'ParticleList_' + particleName: None, 'ParticleList_' + pdg.conjugate(particleName): None}

    userLabel = FR_utility.createHash(particleName, inputLists)
    outputList = particleName + ':' + userLabel
    modularAnalysis.copyLists(outputList, inputLists, path=path)
    return {'ParticleList_' + particleName: outputList, 'ParticleList_' + pdg.conjugate(particleName): pdg.conjugate(particleName) + ':' + userLabel}


def MakeAndMatchParticleList(path, particleName, channelName, inputLists, preCut):
    """
    Creates a ParticleList by combining other particleLists via the ParticleCombiner module and match MC truth for this new list.
        @param path the basf2 path
        @param particleName name of the reconstructed particle, new list is stored as {particleName}:{hash}, where the hash depends on the channel
        @param channelName describes the combined decay, returned key ParticleList_{channelName}
        @param inputLists the inputs lists which are combined
        @param preCut cuts which are applied before the combining of the particles
    """
    if preCut is None:
        return {'ParticleList_' + channelName + '_' + particleName: None, 'ParticleList_' + channelName + '_' + pdg.conjugate(particleName): None}

    userLabel = FR_utility.createHash(particleName, channelName, inputLists, preCut)
    outputList = particleName + ':' + userLabel + ' ==> ' + ' '.join(inputLists)
    listName = particleName + ':' + userLabel
    modularAnalysis.makeParticle(outputList, preCut, path=path)
    modularAnalysis.matchMCTruth(listName, path=path)
    return {'ParticleList_' + channelName + '_' + particleName: listName, 'ParticleList_' + channelName + '_' + pdg.conjugate(particleName): pdg.conjugate(particleName) + ':' + userLabel}
