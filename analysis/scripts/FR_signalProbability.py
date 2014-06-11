#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Thomas Keck 2014
#

from basf2 import *
from FR_utility import createHash
import os
import pdg


def SignalProbability(path, particleName, channelName, mvaConfig, particleList, daughterSignalProbabilities=[]):
    """
    Calculates the SignalProbability of a ParticleList. If the files required from TMVAExpert aren't available they're created.
        @param path the basf2 path
        @param mvaConfig configuration for the multivariate analysis
        @param name of the particle or channel which is classified
        @param particleList the particleList which is used for training and classification
        @param daughterSignalProbabilities all daughter particles need a SignalProbability
    """
    if particleList is None or any([daughterSignalProbability is None for daughterSignalProbability in daughterSignalProbabilities]):
        if particleName == channelName:
            return {'SignalProbability_' + particleName: None, 'SignalProbability_' + pdg.conjugate(particleName): None}
        else:
            return {'SignalProbability_' + channelName + '_' + particleName: None, 'SignalProbability_' + channelName + '_' + pdg.conjugate(particleName): None}

    hash = createHash(mvaConfig, particleList, daughterSignalProbabilities)

    if IsExpertFileAvailable(hash, particleList):
        expert = register_module('TMVAExpert')
        expert.set_name('TMVAExpert_' + particleList)
        expert.param('prefix', particleList + '_' + hash)
        expert.param('method', mvaConfig.name)
        expert.param('signalProbabilityName', 'SignalProbability')
        expert.param('signalCluster', mvaConfig.targetCluster)
        expert.param('listNames', [particleList])
        path.add_module(expert)
        if particleName == channelName:
            return {'SignalProbability_' + particleName: hash, 'SignalProbability_' + pdg.conjugate(particleName): hash}
        else:
            return {'SignalProbability_' + channelName + '_' + particleName: hash, 'SignalProbability_' + channelName + '_' + pdg.conjugate(particleName): hash}

    else:
        teacher = register_module('TMVATeacher')
        teacher.set_name('TMVATeacher_' + particleList)
        teacher.param('prefix', particleList + '_' + hash)
        teacher.param('methods', [(mvaConfig.name, mvaConfig.type, mvaConfig.config)])
        teacher.param('factoryOption', '!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification')
        teacher.param('variables', mvaConfig.variables)
        teacher.param('target', mvaConfig.target)
        teacher.param('listNames', [particleList])
        path.add_module(teacher)
        return {}


def IsExpertFileAvailable(hash, particleList):
    """
    Checks if the config file created by TMVATeacher is available. This config file is required by the TMVAExpert Module.
        @param hash
        @param particleList
    """
    filename = '{particleList}_{hash}.config'.format(particleList=particleList, hash=hash)
    return os.path.isfile(filename)
