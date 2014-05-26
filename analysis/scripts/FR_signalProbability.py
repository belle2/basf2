#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Thomas Keck 2014
#

from basf2 import *
import FR_utility
import os


def SignalProbability(path, method, variables, name, particleList, daughterSignalProbabilities=[]):
    """
    Calculates the SignalProbability of a ParticleList. If the needed experts aren't available they're created.
        @param path the basf2 path
        @param method method given to the TMVAInterface (see TMVAExpert and TMVATeacher)
        @param variables used for classification (see VariableManager)
        @param name of the particle or channel which is classified
        @param particleList the particleList which is used for training and classification
        @param daughterSignalProbabilities all daughter particles need a SignalProbability
    """
    if particleList is None or any([daughterSignalProbability is None for daughterSignalProbability in daughterSignalProbabilities]):
        return {'SignalProbability_' + name: None}

    # Create hash with all parameters on which this training depends
    hash = FR_utility.createHash(method, variables, particleList, daughterSignalProbabilities)
    if IsExpertFileAvailable(hash, particleList):
        path.add_module(GetExpertModule(hash, particleList, method, signalCluster=1))
        return {'SignalProbability_' + name: hash}
    else:
        path.add_module(GetTeacherModule(hash, particleList, method, variables, target='isSignal'))
        return {}


def SignalProbabilityFSPCluster(path, method, variables, name, pdg, particleList):
    """
    Calculates the SignalProbability of a ParticleList. If the needed experts aren't available they're created.
        @param path the basf2 path
        @param method method given to the TMVAInterface (see TMVAExpert and TMVATeacher)
        @param variables used for classification (see VariableManager)
        @param name of the particle which is classified
        @param pdg of the particle which is classified
        @param particleList the particleList which is used for training and classification
    """
    if particleList is None:
        return {'SignalProbability_' + name: None}

    # Create hash with all parameters on which this training depends
    hash = FR_utility.createHash(method, variables, name, particleList, pdg)
    if IsExpertFileAvailable(hash, particleList):
        path.add_module(GetExpertModule(hash, particleList, method, signalCluster=pdg))
        return {'SignalProbability_' + name: hash}
    else:
        path.add_module(GetTeacherModule(hash, particleList, method, variables, target='abs_mcPDG'))
        return {}


def GetExpertModule(hash, particleList, method, signalCluster):
    """
    Creates new TMVAExpert Module
        @param hash used to create prefix of the training
        @param particleList to which the expertise is applied
        @param method which is used
        @param signalCluster which is used as signal component
    """
    expert = register_module('TMVAExpert')
    expert.set_name('TMVAExpert_' + particleList)
    expert.param('prefix', particleList + '_' + hash)
    expert.param('method', method[0])
    expert.param('signalProbabilityName', 'SignalProbability')
    expert.param('signalCluster', signalCluster)
    expert.param('listNames', [particleList])
    return expert


def GetTeacherModule(hash, particleList, method, variables, target):
    """
    Creates new TMVATeacher Module
        @param hash used to create prefix of the training
        @param particleList to which the expertise is applied
        @param method which is used
        @param variables which are used
        @param target which is used as signal component
    """
    teacher = register_module('TMVATeacher')
    teacher.set_name('TMVATeacher_' + particleList)
    teacher.param('prefix', particleList + '_' + hash)
    teacher.param('methods', [method])
    teacher.param('factoryOption', '!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification')
    teacher.param('variables', variables)
    teacher.param('target', target)
    teacher.param('listNames', [particleList])
    return teacher


def IsExpertFileAvailable(hash, particleList):
    """
    Checks if the ExpertFile is available
        @param hash
        @param particleList
    """
    filename = '{particleList}_{hash}.config'.format(particleList=particleList, hash=hash)
    return os.path.isfile(filename)
