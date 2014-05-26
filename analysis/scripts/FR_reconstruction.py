#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Thomas Keck 2014
#

from basf2 import *
import FR_utility
import modularAnalysis

import pdg


def ParticleList(path, name, pdgcode, particleLists, criteria=[]):
    """
    Creates a ParticleList gathering up all particles with the given pdgcode.
        @param path the basf2 path
        @param name name of outputList
        @param pdgcode the pdgcode of the particle
        @param particleLists additional requirements before the Particles can be gathered. E.g. All ParticleList of created by the ParticleListFromChannel exist for this particle
        @param criteria filter criteria
    """
    print particleLists
    particleLists = FR_utility.removeNones(particleLists)
    print particleLists
    # Select all the reconstructed (or loaded) particles with this pdg into one list.
    list_name = name + FR_utility.createHash(name, pdgcode, particleLists, criteria)
    modularAnalysis.selectParticle(list_name, pdgcode, criteria, False, particleLists, path=path)
    return {'ParticleList_' + name: list_name}


def ParticleListFromChannel(path, pdgcode, name, preCut, inputLists, hasMissing):
    """
    Creates a ParticleList by combining other particleLists via the ParticleCombiner module.
        @param path the basf2 path
        @param pdgcode pdgcode of the particle which is reconstructed
        @param name name of the channel or particle which shall be combined
        @param preCut cuts which are applied before the combining of the particles
        @param inputLists the inputs lists which are combined
        @param hasMissing true if the channel is incomplete
    """
    if preCut is None:
        return {'ParticleList_' + name: None}

    list_name = name + FR_utility.createHash(pdgcode, name, preCut, inputLists)

    pmake = register_module('ParticleCombiner')
    pmake.set_name('ParticleCombiner_' + name)
    pmake.param('PDG', pdgcode)
    pmake.param('ListName', list_name)
    pmake.param('InputListNames', inputLists)
    pmake.param('cuts', preCut)
    path.add_module(pmake)
    modularAnalysis.matchMCTruth(list_name, path=path)
    return {'ParticleList_' + name: list_name}
