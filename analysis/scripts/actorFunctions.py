#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Thomas Keck 2014
#
# All the actors functions are defined here.
# To create your own actor:
#   1. Write a normal function which takes the needed arguments and returns a dictonary of provided values.
#      E.g. def foo(path, particleList) ... return {'Stuff': x}
#   2. Make sure your return value depends on all the used arguments, easiest way to accomplish this is using the createHash function.
#   3. Add the function to the sequence object like this (in FullEventInterpretation.py):
#      seq.addFunction(foo, path='Path', particleList='K+')

from basf2 import *
from ROOT import Belle2
import modularAnalysis
import pdg

import actorFramework
import preCutDetermination
import os
import subprocess
from string import Template


def SelectParticleList(path, particleName, explicitCuts):
    """
    Creates a ParticleList gathering up all particles with the given particleName
        @param path the basf2 path
        @param particleName returned key is named ParticleList_{particleName} corresponding ParticleList is stored as {particleName}:{hash}
    """
    userLabel = actorFramework.createHash(particleName, explicitCuts)
    outputList = particleName + ':' + userLabel
    modularAnalysis.selectParticle(outputList, explicitCuts, path=path)
    return {'ParticleList_' + particleName: outputList,
            'ParticleList_' + pdg.conjugate(particleName): pdg.conjugate(particleName) + ':' + userLabel}


def CopyParticleLists(path, particleName, inputLists):
    """
    Creates a ParticleList gathering up all particles in the given inputLists
        @param path the basf2 path
        @param particleName returned key is named ParticleList_{particleName} corresponding ParticleList is stored as {particleName}:{hash}
        @param inputLists names of inputLists which are copied to the new list
    """
    inputLists = actorFramework.removeNones(inputLists)
    if inputLists == []:
        return {'ParticleList_' + particleName: None, 'ParticleList_' + pdg.conjugate(particleName): None}

    userLabel = actorFramework.createHash(particleName, inputLists)
    outputList = particleName + ':' + userLabel
    modularAnalysis.copyLists(outputList, inputLists, path=path)
    return {'ParticleList_' + particleName: outputList,
            'ParticleList_' + pdg.conjugate(particleName): pdg.conjugate(particleName) + ':' + userLabel}


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

    userLabel = actorFramework.createHash(particleName, channelName, inputLists, preCut)
    outputList = particleName + ':' + userLabel + ' ==> ' + ' '.join(inputLists)
    listName = particleName + ':' + userLabel
    modularAnalysis.makeParticle(outputList, preCut, path=path)
    modularAnalysis.matchMCTruth(listName, path=path)
    return {'ParticleList_' + channelName + '_' + particleName: listName,
            'ParticleList_' + channelName + '_' + pdg.conjugate(particleName): pdg.conjugate(particleName) + ':' + userLabel}


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
        return {'SignalProbability_' + channelName + '_' + particleName: None,
                'SignalProbability_' + channelName + '_' + pdg.conjugate(particleName): None}

    hash = actorFramework.createHash(mvaConfig, particleList, daughterSignalProbabilities)

    filename = '{particleList}_{hash}.config'.format(particleList=particleList, hash=hash)
    if os.path.isfile(filename):

        expert = register_module('TMVAExpert')
        expert.set_name('TMVAExpert_' + particleList)
        expert.param('prefix', particleList + '_' + hash)
        expert.param('method', mvaConfig.name)
        expert.param('signalProbabilityName', 'SignalProbability')
        expert.param('signalCluster', mvaConfig.targetCluster)
        expert.param('listNames', [particleList])
        path.add_module(expert)

        if particleName == channelName:
            return {'SignalProbability_' + particleName: filename,
                    'SignalProbability_' + pdg.conjugate(particleName): filename}
        else:
            return {'SignalProbability_' + channelName + '_' + particleName: filename,
                    'SignalProbability_' + channelName + '_' + pdg.conjugate(particleName): filename}

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


def CreatePreCutHistogram(path, particleName, channelName, preCutConfig, daughterLists, additionalDependencies):
    """
    Creates ROOT file with invariant mass and signal probability product histogram of this channel (signal/background)
    for a given particle, before any intermediate cuts are applied.
        @param path the basf2 path
        @param particleName for which this histogram is created
        @param channelName of the channel
        @param daughterLists all particleLists of all the daughter particles
        @param daughterSignalProbabilities all daughter particles need a SignalProbability
    """
    if any([daughterList is None for daughterList in daughterLists]) or any([x is None for x in additionalDependencies]):
        return {'PreCutHistogram_' + channelName: None}

    # Check if the file is available. If the file isn't available yet, create it with
    # the HistMaker Module and return Nothing. If a function is called which depends on
    # the PreCutHistogram, the process will stop, because the PreCutHistogram isn't provided.
    hash = actorFramework.createHash(particleName, channelName, preCutConfig.variable, daughterLists, additionalDependencies)
    filename = 'CutHistograms_{pname}_{cname}_{hash}.root'.format(pname=particleName, cname=channelName, hash=hash)

    if os.path.isfile(filename):
        return {'PreCutHistogram_' + channelName: (filename, particleName + ':' + hash)}
    else:
        # Combine all the particles according to the decay channels
        outputList = particleName + ':' + hash + ' ==> ' + ' '.join(daughterLists)
        pmake = register_module('PreCutHistMaker')
        pmake.set_name('PreCutHistMaker_' + channelName)
        pmake.param('fileName', filename)
        pmake.param('decayString', outputList)
        if preCutConfig.variable == 'Mass':
            mass = pdg.get(pdg.from_name(particleName)).Mass()
            pmake.param('variable', 'M')
            pmake.param('histParams', (200, mass / 2, mass + mass / 2))
        else:
            pmake.param('variable', 'daughterProductOf(getExtraInfo(SignalProbability))')
            pmake.param('customBinning', list(reversed([1.0 / (1.5 ** i) for i in range(0, 20)])))
        path.add_module(pmake)
    return {}


def PreCutDetermination(particleName, channelNames, preCutConfig, preCutHistograms):
    """
    Determines the PreCuts for all the channels of a particle.
        @param particleName name of the particle
        @param channelNames list of the names of all the channels
        @param preCutConfig configuration for PreCut determination e.g. signal efficiency for this particle
        @param preCutHistograms filenames of the histogram files created for every channel by PreCutDistribution
    """

    results = {'PreCut_' + channel: None for channel, _ in zip(*actorFramework.getNones(channelNames, preCutHistograms))}
    channelNames, preCutHistograms = actorFramework.removeNones(channelNames, preCutHistograms)

    cuts = preCutDetermination.CalculatePreCuts(preCutConfig, channelNames, preCutHistograms)

    for (channel, cut) in cuts.iteritems():
        results['PreCut_' + channel] = None if cut['isIgnored'] else {cut['variable']: cut['range']}
    return results


def WriteAnalysisFileForChannel(particleName, channelName, preCutConfig, preCutHistogram, preCut, mvaConfig, signalProbability):
    """
    Creates a pdf document with the PreCut and Training plots
        @param particleName name of the particle
        @param channelName name of the channel
        @param preCutConfig configuration for pre cut
        @param preCutHistogram preCutHistogram (filename, histogram postfix)
        @param preCut used preCuts for this channel
        @param mvaConfig configuration for mva
        @param signalProbability config filename for TMVA training
    """

    if signalProbability is None or preCut is None:
        return {'dummy': None}

    stripped_tmva_filename = signalProbability[:-7]  # Strip .config of filename
    stripped_preCut_filename = preCutHistogram[0][:-5]  # Strip .root of filename
    subprocess.call(['createAnalysisPlots', stripped_tmva_filename, stripped_preCut_filename, str(preCut.values()[0][0]), str(preCut.values()[0][1])])

    placeholders = {}
    placeholders['particleName'] = particleName
    placeholders['channelName'] = channelName

    placeholders['preCutVariable'] = preCutConfig.variable
    placeholders['preCutEfficiency'] = preCutConfig.efficiency
    placeholders['preCutRange'] = 'Ignored' if preCut is None else str(preCut.values()[0])

    placeholders['preCutAllPlot'] = 'all.png'
    placeholders['preCutSignalPlot'] = 'signal.png'
    placeholders['preCutBackgroundPlot'] = 'background.png'
    placeholders['preCutRatioPlot'] = 'ratio.png'

    placeholders['mvaName'] = mvaConfig.name
    placeholders['mvaType'] = mvaConfig.type
    placeholders['mvaConfig'] = mvaConfig.config
    placeholders['mvaVariables'] = ', '.join(mvaConfig.variables)
    placeholders['mvaTarget'] = mvaConfig.target
    placeholders['mvaTargetCluster'] = mvaConfig.targetCluster

    placeholders['mvaROCPlot'] = 'roc.png'
    placeholders['mvaOvertrainingPlot'] = 'overtraining.png'

    template = Template(file(Belle2.FileSystem.findFile('analysis/scripts/FullEventInterpretationTemplate.tex'), 'r').read())
    page = template.substitute(placeholders)
    file(channelName + '.tex', 'w').write(page)
    subprocess.call(['pdflatex', channelName + '.tex'])

    return {}
    return {'dummy': None}
