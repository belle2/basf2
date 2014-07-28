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
import ROOT
import modularAnalysis
import pdg

import actorFramework
import preCutDetermination
import automaticReporting

import os
import subprocess
import json
from string import Template
import IPython


def CountMCParticles(path):
    """
    Counts the number of MC Particles for every pdg code in all events
        @param path the basf2 path
    """
    filename = 'mcParticlesCount.json'
    if not os.path.isfile(filename):
        class MCParticleCounter(Module):
            def initialize(self):
                self.counter = {'NEvents': 0}

            def event(self):
                self.counter['NEvents'] += 1
                particles = ROOT.Belle2.PyStoreArray("MCParticles")
                for particle in particles:
                    pdg = abs(particle.getPDG())
                    if pdg not in self.counter:
                        self.counter[pdg] = 0
                    self.counter[pdg] += 1

            def terminate(self):
                json.dump(self.counter, open(filename, 'w'))

        path.add_module(MCParticleCounter())
        B2INFO("Count number of MCParticles for every pdg code seperatly")
        return {}
    else:
        counter = json.load(open(filename, 'r'))
        B2INFO("Loaded number of MCParticles for every pdg code seperatly")
        return {'MCParticleCounts': counter}


def SelectParticleList(path, particleName):
    """
    Creates a ParticleList gathering up all particles with the given particleName
        @param path the basf2 path
        @param particleName returned key is named ParticleList_{particleName} corresponding ParticleList is stored as {particleName}:{hash}
    """
    userLabel = actorFramework.createHash(particleName)
    outputList = particleName + ':' + userLabel
    modularAnalysis.selectParticle(outputList, path=path)

    B2INFO("Select Particle List " + outputList + " and charged conjugated")
    return {'RawParticleList_' + particleName: outputList,
            'RawParticleList_' + pdg.conjugate(particleName): pdg.conjugate(particleName) + ':' + userLabel}


def CopyParticleLists(path, particleName, channelName, inputLists, postCut):
    """
    Creates a ParticleList gathering up all particles in the given inputLists
        @param path the basf2 path
        @param particleName returned key is named ParticleList_{particleName} corresponding ParticleList is stored as {particleName}:{hash}
        @param channelName if not None returned key is named ParticleList_{channelName}_{particleName} corresponding ParticleList is stored as {particleName}:{hash}
        @param inputLists names of inputLists which are copied to the new list
        @param postCut cuts which are applied after the reconstruction of the particle
    """
    inputLists = actorFramework.removeNones(inputLists)

    if inputLists == []:
        if channelName is None:
            B2INFO("Gather Particle List for particle " + particleName + " and charged conjugated. But there are no particles to gather :-(.")
            return {'ParticleList_' + particleName: None,
                    'ParticleList_' + pdg.conjugate(particleName): None}
        else:
            B2INFO("Gather Particle List for particle " + particleName + " " + channelName + " and charged conjugated. But there are no particles to gather :-(.")
            return {'ParticleList_' + channelName + '_' + particleName: None,
                    'ParticleList_' + channelName + '_' + pdg.conjugate(particleName): None}

    userLabel = actorFramework.createHash(particleName, channelName, inputLists, postCut)
    outputList = particleName + ':' + userLabel
    modularAnalysis.cutAndCopyLists(outputList, inputLists, postCut['cutstring'], path=path)

    if channelName is None:
        B2INFO("Gather Particle List for particle " + particleName + " and charged conjugated")
        return {'ParticleList_' + particleName: outputList,
                'ParticleList_' + pdg.conjugate(particleName): pdg.conjugate(particleName) + ':' + userLabel}
    else:
        B2INFO("Gather Particle List for particle " + particleName + " " + channelName + " and charged conjugated")
        return {'ParticleList_' + channelName + '_' + particleName: outputList,
                'ParticleList_' + channelName + '_' + pdg.conjugate(particleName): pdg.conjugate(particleName) + ':' + userLabel}


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
        B2INFO("Make and Match Particle List for channel " + channelName + " and charged conjugated. But the channel is ignored :-(.")
        return {'RawParticleList_' + channelName + '_' + particleName: None,
                'RawParticleList_' + channelName + '_' + pdg.conjugate(particleName): None}

    userLabel = actorFramework.createHash(particleName, channelName, inputLists, preCut)
    outputList = particleName + ':' + userLabel + ' ==> ' + ' '.join(inputLists)
    listName = particleName + ':' + userLabel
    modularAnalysis.makeParticle(outputList, preCut['cutstring'], path=path)
    modularAnalysis.matchMCTruth(listName, path=path)
    B2INFO("Make and Match Particle List for channel " + channelName + " and charged conjugated.")
    return {'RawParticleList_' + channelName + '_' + particleName: listName,
            'RawParticleList_' + channelName + '_' + pdg.conjugate(particleName): pdg.conjugate(particleName) + ':' + userLabel}


def FitVertex(path, particleName, channelName, particleList):
    """
    Fit secondary vertex of this particle
        @param path the basf2 path
        @param particleName name of the reconstructed particle
        @param channelName decay channel name
        @param particleList the particleList which is fitted
    """
    modularAnalysis.fitVertex(particleList, 0)
    B2INFO("Fitted vertex for channel " + channelName + " and charged conjugated.")
    return {'VertexFit_' + channelName + '_' + particleName: 'dummy',
            'VertexFit_' + channelName + '_' + pdg.conjugate(particleName): 'dummy'}


def SignalProbability(path, particleName, channelName, mvaConfig, particleList, additionalDependencies=[]):
    """
    Calculates the SignalProbability of a ParticleList. If the files required from TMVAExpert aren't available they're created.
        @param path the basf2 path
        @param particleName of the particle which is classified
        @param channelName of channel which is classified
        @param mvaConfig configuration for the multivariate analysis
        @param particleList the particleList which is used for training and classification
        @param additionalDependencies for variables like SignalProbability of daughters or VertexFit
    """
    if particleList is None or any([d is None for d in additionalDependencies]):
        B2INFO("Calculate SignalProbability for channel " + channelName + " and charged conjugated. But the channel is ignored :-(.")
        return {'SignalProbability_' + channelName + '_' + particleName: None,
                'SignalProbability_' + channelName + '_' + pdg.conjugate(particleName): None}

    hash = actorFramework.createHash(particleName, channelName, mvaConfig, particleList, additionalDependencies)

    rootFilename = '{particleList}_{hash}.root'.format(particleList=particleList, hash=hash)
    configFilename = '{particleList}_{hash}.config'.format(particleList=particleList, hash=hash)

    if not os.path.isfile(rootFilename):
        teacher = register_module('TMVATeacher')
        teacher.set_name('TMVATeacher_' + particleList)
        teacher.param('prefix', particleList + '_' + hash)
        teacher.param('methods', [(mvaConfig.name, mvaConfig.type, mvaConfig.config)])
        teacher.param('factoryOption', '!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification')
        teacher.param('prepareOption', 'SplitMode=random:!V')
        teacher.param('variables', mvaConfig.variables)
        teacher.param('target', mvaConfig.target)
        teacher.param('listNames', [particleList])
        teacher.param('maxEventsPerClass', 10000000)
        teacher.param('doNotTrain', True)
        path.add_module(teacher)
        B2INFO("Calculate SignalProbability for channel " + channelName + " and charged conjugated. Create root file with variables first.")
        return {}

    if not os.path.isfile(configFilename):
        B2INFO("Calculate SignalProbability for channel " + channelName + " and charged conjugated. Run Teacher in extern process.")
        subprocess.call("externTeacher --methodName '{name}' --methodType '{type}' --methodConfig '{config}' --target '{target}'"
                        " --variables '{variables}' --factoryOption '{foption}' --prepareOption '{poption}' --prefix '{prefix}'"
                        " --maxEventsPerClass {maxEvents}".format(name=mvaConfig.name, type=mvaConfig.type, config=mvaConfig.config,
                                                                  target=mvaConfig.target, variables="' '".join(mvaConfig.variables),
                                                                  foption='!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification',
                                                                  poption='SplitMode=random:!V', maxEvents=10000000,
                                                                  prefix=particleList + '_' + hash), shell=True)

    if os.path.isfile(configFilename):

        expert = register_module('TMVAExpert')
        expert.set_name('TMVAExpert_' + particleList)
        expert.param('prefix', particleList + '_' + hash)
        expert.param('method', mvaConfig.name)
        expert.param('signalFraction', -2)  # Use signalFraction from training
        expert.param('signalProbabilityName', 'SignalProbability')
        expert.param('signalClass', mvaConfig.targetCluster)
        expert.param('listNames', [particleList])
        path.add_module(expert)

        B2INFO("Calculate SignalProbability for channel " + channelName + " and charged conjugated.")
        if particleName == channelName:
            return {'SignalProbability_' + particleName: configFilename,
                    'SignalProbability_' + pdg.conjugate(particleName): configFilename}
        else:
            return {'SignalProbability_' + channelName + '_' + particleName: configFilename,
                    'SignalProbability_' + channelName + '_' + pdg.conjugate(particleName): configFilename}

    B2ERROR("Training of channel " + channelName + " failed")
    return {}


def VariablesToNTuple(path, particleName, particleList, signalProbability):
    """
    Saves the calculated signal probability for this particle list
        @param path the basf2 path
        @param particleName particleName
        @param particleList the particleList
        @param signalProbability signalProbability as additional dependency
    """

    if particleList is None or signalProbability is None:
        B2INFO("Write variables to ntuple for " + particleName + " and charged conjugated. But list is ignored.")
        return {'VariablesToNTuple': None}

    hash = actorFramework.createHash(particleName, particleList, signalProbability)
    filename = 'var_{particleName}_{hash}.root'.format(particleName=particleName, hash=hash)

    if not os.path.isfile(filename):
        output = register_module('VariablesToNtuple')
        output.param('particleList', particleList)
        output.param('variables', ['getExtraInfo(SignalProbability)', 'isSignal', 'Mbc', 'mcStatus'])
        output.param('fileName', filename)
        output.param('treeName', 'variables')
        path.add_module(output)
        B2INFO("Write variables to ntuple for " + particleName + " and charged conjugated.")
        return {}

    B2INFO("Write variables to ntuple for " + particleList + " and charged conjugated. But file already exists, so nothing to do here.")
    particleName = particleList.split(':')[0]
    return {'VariablesToNTuple_' + particleName: filename}


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
        B2INFO("Create pre cut histogram for channel " + channelName + " and charged conjugated. But channel is ignored.")
        return {'PreCutHistogram_' + channelName: None}

    # Check if the file is available. If the file isn't available yet, create it with
    # the HistMaker Module and return Nothing. If a function is called which depends on
    # the PreCutHistogram, the process will stop, because the PreCutHistogram isn't provided.
    hash = actorFramework.createHash(particleName, channelName, preCutConfig.variable, daughterLists, additionalDependencies)
    filename = 'CutHistograms_{pname}_{cname}_{hash}.root'.format(pname=particleName, cname=channelName, hash=hash)

    if os.path.isfile(filename):
        B2INFO("Create pre cut histogram for channel " + channelName + " and charged conjugated. But file already exists, so nothing to do here.")
        return {'PreCutHistogram_' + channelName: (filename, particleName + ':' + hash)}
    else:
        # Combine all the particles according to the decay channels
        outputList = particleName + ':' + hash + ' ==> ' + ' '.join(daughterLists)
        pmake = register_module('PreCutHistMaker')
        pmake.set_name('PreCutHistMaker_' + channelName)
        pmake.param('fileName', filename)
        pmake.param('decayString', outputList)
        if preCutConfig.variable in ['M']:
            mass = pdg.get(pdg.from_name(particleName)).Mass()
            pmake.param('variable', 'M')
            pmake.param('histParams', (200, mass / 2, mass + mass / 2))
        elif preCutConfig.variable in ['Q']:
            pmake.param('variable', 'Q')
            pmake.param('histParams', (200, -1, 1))
        else:
            pmake.param('variable', 'daughterProductOf(getExtraInfo(SignalProbability))')
            pmake.param('customBinning', list(reversed([1.0 / (1.5 ** i) for i in range(0, 20)])))
        path.add_module(pmake)

    B2INFO("Create pre cut histogram for channel " + channelName + " and charged conjugated.")
    return {}


def PreCutDetermination(particleName, channelNames, preCutConfigs, preCutHistograms):
    """
    Determines the PreCuts for all the channels of a particle.
        @param particleName name of the particle
        @param channelNames list of the names of all the channels
        @param preCutConfig configuration for PreCut determination e.g. signal efficiency for this particle for every chanel
        @param preCutHistograms filenames of the histogram files created for every channel by PreCutDistribution
    """

    results = {'PreCut_' + channel: None for channel, _, __ in zip(*actorFramework.getNones(channelNames, preCutHistograms, preCutConfigs))}
    channelNames, preCutHistograms, preCutConfigs = actorFramework.removeNones(channelNames, preCutHistograms, preCutConfigs)

    cuts = preCutDetermination.CalculatePreCuts(preCutConfigs, channelNames, preCutHistograms)

    for (channel, cut) in cuts.iteritems():
        results['PreCut_' + channel] = None if cut['isIgnored'] else cut

    B2INFO("Calculate pre cut for particle " + particleName + " and charged conjugated.")
    return results


def PostCutDetermination(particleName, postCutConfig, signalProbability):
    """
    Determines the PostCut of a particle.
        @param particleName name of the particle
        @param postCutConfig configuration for post cut determination
        @param signalProbability of the particle
    """
    B2INFO("Calculate post cut for particle " + particleName + " and charged conjugated.")
    return {'PostCut_' + particleName: {'cutstring': str(postCutConfig.value) + ' < getExtraInfo(SignalProbability)', 'range': (postCutConfig.value, 1)}}


def WriteAnalysisFileForChannel(particleName, channelName, preCutConfig, preCut, preCutHistogram, mvaConfig, signalProbability, postCutConfig, postCut):
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

    placeholders = {}
    placeholders['particleName'] = particleName
    placeholders['channelName'] = channelName
    placeholders['isIgnored'] = False
    placeholders = automaticReporting.createPreCutTexFile(placeholders, preCutHistogram, preCutConfig, preCut)
    placeholders = automaticReporting.createMVATexFile(placeholders, mvaConfig, signalProbability, postCutConfig, postCut)

    hash = actorFramework.createHash(placeholders)
    placeholders['texFile'] = '{name}_channel_{hash}.tex'.format(name=placeholders['particleName'], hash=hash)
    if not os.path.isfile(placeholders['texFile']):
        automaticReporting.createTexFile(placeholders['texFile'], 'analysis/scripts/FullEventInterpretationChannelTemplate.tex', placeholders)

    B2INFO("Written analysis tex file for channel " + channelName)
    return {'Placeholders_' + channelName: placeholders}


def WriteAnalysisFileForFSParticle(particleName, mvaConfig, signalProbability, postCutConfig, postCut, mcCounts):
    """
    Creates a pdf document with the PreCut and Training plots
        @param particleName name of the particle
        @param mvaConfig configuration for mva
        @param signalProbability config filename for TMVA training
    """

    placeholders = {}
    placeholders['particleName'] = particleName
    placeholders['isIgnored'] = False
    placeholders = automaticReporting.createMVATexFile(placeholders, mvaConfig, signalProbability, postCutConfig, postCut)
    placeholders = automaticReporting.createFSParticleTexFile(placeholders, mcCounts)

    B2INFO("Written analysis tex file for final state particle " + particleName)
    return {'Placeholders_' + particleName: placeholders}


def WriteAnalysisFileForCombinedParticle(particleName, channelPlaceholders, mcCounts):
    """
    Creates a pdf document with the PreCut and Training plots
        @param particleName name of the particle
        @param channelPlaceholders list of all tex placeholder dictionaries of all channels
    """

    placeholders = {'channels': channelPlaceholders}
    placeholders['particleName'] = particleName
    placeholders['isIgnored'] = False
    placeholders = automaticReporting.createCombinedParticleTexFile(placeholders, channelPlaceholders, mcCounts)

    B2INFO("Written analysis tex file for intermediate particle " + particleName)
    return {'Placeholders_' + particleName: placeholders}


def WriteAnalysisFileSummary(finalStateParticlePlaceholders, combinedParticlePlaceholders, ntuples, mcCounts, particles):
    """
    Creates a pdf summarizing all networks trained.
        @param texfiles list of tex filenames
    """

    placeholders = automaticReporting.createSummaryTexFile(finalStateParticlePlaceholders, combinedParticlePlaceholders, ntuples, mcCounts, particles)

    for i in range(0, 2):
        ret = subprocess.call(['pdflatex', '-halt-on-error', placeholders['texFile']])
        if ret == 0:
            B2INFO("Created FEI summary PDF.")
        else:
            B2ERROR("pdflatex failed to create FEI summary PDF, please check.")

    # Return None - Therefore Particle List depends not on TMVAExpert directly
    B2INFO("Created analysis summary pdf file ")
    return {'FEIsummary.pdf': None}
