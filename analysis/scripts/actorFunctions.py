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


def SelectParticleList(path, particleName, particleLabel):
    """
    Creates a ParticleList gathering up all particles with the given particleName
        @param path the basf2 path
        @param particleName
        @param particleLabel
        @return key is named RawParticleList_{particleName}_{label} corresponding ParticleList is stored as {particleName}:{hash}
    """
    userLabel = actorFramework.createHash(particleName, particleLabel)
    outputList = particleName + ':' + userLabel
    modularAnalysis.selectParticle(outputList, path=path)

    B2INFO("Select Particle List {p} with label {l} in list {list}".format(p=particleName, l=particleLabel, list=outputList))
    return {'RawParticleList_{p}_{l}'.format(p=particleName, l=particleLabel): outputList}


def CopyParticleLists(path, particleName, particleLabel, inputLists, postCuts):
    """
    Creates a ParticleList gathering up all particles in the given inputLists
        @param path the basf2 path
        @param particleName
        @param particleLabel
        @param inputLists names of inputLists which are copied to the new list
        @param postCuts cuts which are applied after the reconstruction of the particle
        @return  key is named ParticleList_{particleName}_{label} corresponding ParticleList is stored as {particleName}:{hash}
    """
    inputLists, postCuts = actorFramework.removeNones(inputLists, postCuts)

    if inputLists == []:
        B2INFO("Gather Particle List for particle " + particleName + " and charged conjugated. But there are no particles to gather :-(.")
        return {'ParticleList_{p}_{l}'.format(p=particleName, l=particleLabel): None,
                'ParticleList_{p}_{l}'.format(p=pdg.conjugate(particleName), l=particleLabel): None}

    userLabel = actorFramework.createHash(particleName, particleLabel, inputLists, postCuts)
    outputList = particleName + ':' + userLabel
    # TODO Use only first post cut at the moment
    modularAnalysis.cutAndCopyLists(outputList, inputLists, postCut[0]['cutstring'], path=path)

    B2INFO("Gather Particle List {p} with label {l} in list {o}".format(p=particleName, l=particleLabel, o=outputList))
    return {'ParticleList_{p}_{l}'.format(p=particleName, l=particleLabel): outputList,
            'ParticleList_{p}_{l}'.format(p=pdg.conjugate(particleName), l=particleLabel): pdg.conjugate(particleName) + ':' + userLabel}


def MakeAndMatchParticleList(path, particleName, particleLabel, channelName, inputLists, preCut):
    """
    Creates a ParticleList by combining other particleLists via the ParticleCombiner module and match MC truth for this new list.
        @param path the basf2 path
        @param particleName name of the reconstructed particle
        @param particleLabel
        @param channelName describes the combined decay
        @param inputLists the inputs lists which are combined
        @param preCut cuts which are applied before the combining of the particles
        @return key is named RawParticleList_{channelName}_{particleName}_{particleLabel} corresponding list is stored as {particleName}:{hash}
    """
    userLabel = actorFramework.createHash(particleName, particleLabel, channelName, inputLists, preCut)
    outputList = particleName + ':' + userLabel
    # If preCut is None this channel is ignored
    if preCut is None:
        B2INFO("Make and Match Particle List {p} with label {l} for channel {c} in list {o}".format(p=particleName, l=particleLabel, c=channelName, o=outputList))
        return {'RawParticleList_{c}'.format(c=channelName): None}

    decayString = outputList + ' ==> ' + ' '.join(inputLists)
    modularAnalysis.makeParticle(decayString, preCut['cutstring'], path=path)
    modularAnalysis.matchMCTruth(outputList, path=path)
    B2INFO("Make and Match Particle List {p} with label {l} for channel {c} in list {o}".format(p=particleName, l=particleLabel, c=channelName, o=outputList))
    return {'RawParticleList_{c}'.format(c=channelName): outputList}


def FitVertex(path, channelName, particleList):
    """
    Fit secondary vertex of this particle
        @param path the basf2 path
        @param channelName decay channel name
        @param particleList the particleList which is fitted
    """
    modularAnalysis.fitVertex(particleList, 0)
    B2INFO("Fitted vertex for channel {c} and charged conjugated.".format(c=channelName))
    return {'VertexFit_{c}'.format(c=channelName): 'dummy'}


def SignalProbability(path, particleName, particleLabel, channelName, mvaConfig, particleList, additionalDependencies=[]):
    """
    Calculates the SignalProbability of a ParticleList. If the files required from TMVAExpert aren't available they're created.
        @param path the basf2 path
        @param particleName of the particle which is classified
        @param particleLabel
        @param channelName of channel which is classified
        @param mvaConfig configuration for the multivariate analysis
        @param particleList the particleList which is used for training and classification
        @param additionalDependencies for variables like SignalProbability of daughters or VertexFit
    """
    if particleList is None or any([d is None for d in additionalDependencies]):
        if particleName == channelName:
            B2INFO("Calculate SignalProbability for particle {p} with label {l}, but particle is ignored. This can never happen!".format(p=particleName, l=particleLabel))
            return{'SignalProbability_{p}_{l}'.format(p=particleName, l=particleLabel): None}
        else:
            B2INFO("Calculate SignalProbability for channel {c}, but the channel is ignored :-(.".format(c=channelName))
            return {'SignalProbability_{c}'.format(c=channelName): None}

    hash = actorFramework.createHash(particleName, particleLabel, channelName, mvaConfig, particleList, additionalDependencies)

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
        B2INFO("Calculate SignalProbability for particle {p} with label {l} for channel {c}. Create root file with variables first.".format(p=particleName, l=particleLabel, c=channelName))
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

        B2INFO("Calculate SignalProbability for particle {p} with label {l} for channel {c}.".format(p=particleName, l=particleLabel, c=channelName))
        if particleName == channelName:
            return {'SignalProbability_{p}_{l}'.format(p=particleName, l=particleLabel): configFilename}
        else:
            return {'SignalProbability_{c}'.format(c=channelName): configFilename}

    B2ERROR("Training of channel " + channelName + " failed")
    return {}


def VariablesToNTuple(path, particleName, particleLabel, particleList, signalProbability):
    """
    Saves the calculated signal probability for this particle list
        @param path the basf2 path
        @param particleName particleName
        @param particleLabel
        @param particleList the particleList
        @param signalProbability signalProbability as additional dependency
    """

    if particleList is None or signalProbability is None:
        B2INFO("Write variables to ntuple for " + particleName + " and charged conjugated. But list is ignored.")
        return {'VariablesToNTuple': None}

    hash = actorFramework.createHash(particleName, particleLabel, particleList, signalProbability)
    filename = 'var_{p}_{l}_{h}.root'.format(p=particleName, l=particleLabel, h=hash)

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
    return {'VariablesToNTuple_{p}_{l}'.format(p=particleName, l=particleLabel): filename}


def CreatePreCutHistogram(path, particleName, particleLabel, channelName, preCutConfig, daughterLists, additionalDependencies):
    """
    Creates ROOT file with invariant mass and signal probability product histogram of this channel (signal/background)
    for a given particle, before any intermediate cuts are applied.
        @param path the basf2 path
        @param particleName for which this histogram is created
        @param particleLabel
        @param channelName of the channel
        @param daughterLists all particleLists of all the daughter particles
        @param daughterSignalProbabilities all daughter particles need a SignalProbability
    """
    if any([daughterList is None for daughterList in daughterLists]) or any([x is None for x in additionalDependencies]):
        B2INFO("Create pre cut histogram for channel " + channelName + " and charged conjugated. But channel is ignored.")
        return {'PreCutHistogram_{c}'.format(c=channelName): None}

    # Check if the file is available. If the file isn't available yet, create it with
    # the HistMaker Module and return Nothing. If a function is called which depends on
    # the PreCutHistogram, the process will stop, because the PreCutHistogram isn't provided.
    hash = actorFramework.createHash(particleName, particleLabel, channelName, preCutConfig.variable, daughterLists, additionalDependencies)
    filename = 'CutHistograms_{c}_{h}.root'.format(c=channelName, h=hash)

    if os.path.isfile(filename):
        B2INFO("Create pre cut histogram for channel " + channelName + " and charged conjugated. But file already exists, so nothing to do here.")
        return {'PreCutHistogram_{c}'.format(c=channelName): (filename, particleName + ':' + hash)}
    else:
        # Combine all the particles according to the decay channels
        outputList = particleName + ':' + hash + ' ==> ' + ' '.join(daughterLists)
        pmake = register_module('PreCutHistMaker')
        pmake.set_name('PreCutHistMaker_{c}'.format(c=channelName))
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


def PreCutDetermination(channelNames, preCutConfigs, preCutHistograms):
    """
    Determines the PreCuts for all the channels of a particle.
        @param channelNames list of the names of all the channels
        @param preCutConfig configuration for PreCut determination e.g. signal efficiency for this particle for every chanel
        @param preCutHistograms filenames of the histogram files created for every channel by PreCutDistribution
    """

    results = {'PreCut_{c}'.format(c=channelName): None for channelName, _, __ in zip(*actorFramework.getNones(channelNames, preCutConfigs, preCutHistograms))}
    channelNames, preCutConfigs, preCutHistograms = actorFramework.removeNones(channelNames, preCutConfigs, preCutHistograms)
    for (channelName, cut) in preCutDetermination.CalculatePreCuts(preCutConfigs, channelNames, preCutHistograms).iteritems():
        results['PreCut_{c}'.format(c=channelName)] = None if cut['isIgnored'] else cut
        B2INFO("Calculate pre cut for channel {c}".format(c=channelName))
    return results


def PostCutDeterminationFSP(particleName, particleLabel, postCutConfig, signalProbability):
    """
    Determines the PostCut of a particle.
        @param particleName
        @param particleLabel
        @param channelNames names of the channels
        @param postCutConfig configuration for post cut determination
        @param signalProbability
    """
    result['PostCut_{p}_{l}'.format(p=particleName, l=particleLabel)] = {'cutstring': str(postCutConfig.value) + ' < getExtraInfo(SignalProbability)', 'range': (postCutConfig.value, 1)}
    B2INFO("Calculate post cut for particle {p} with label {l}".format(p=particleName, l=particleLabel))
    return result


def PostCutDeterminationNonFSP(channelNames, postCutConfigs, signalProbabilities):
    """
    Determines the PostCut of a particle.
        @param channelNames names of the channels
        @param postCutConfig configuration for post cut determination
        @param signalProbabilities of the channels
    """
    results = {'PostCut_{c}'.format(c=channelName): None for channelName, _, __ in zip(*actorFramework.getNones(channelNames, postCutConfigs, signalProbabilities))}
    channelNames, postCutConfigs, signalProbabilities = actorFramework.removeNones(channelNames, preCutConfigs, signalProbabilities)
    for channelName, postCutConfig in zip(channelNames, postCutConfigs):
        result['PostCut_{c}'.format(c=channelName)] = {'cutstring': str(postCutConfig.value) + ' < getExtraInfo(SignalProbability)', 'range': (postCutConfig.value, 1)}
        B2INFO("Calculate post cut for channel {c}".format(c=channelName))
    return result


def WriteAnalysisFileForChannel(particleName, particleLabel, channelName, preCutConfig, preCut, preCutHistogram, mvaConfig, signalProbability, postCutConfig, postCut):
    """
    Creates a pdf document with the PreCut and Training plots
        @param particleName name of the particle
        @param particleLabel
        @param channelName name of the channel
        @param preCutConfig configuration for pre cut
        @param preCutHistogram preCutHistogram (filename, histogram postfix)
        @param preCut used preCuts for this channel
        @param mvaConfig configuration for mva
        @param signalProbability config filename for TMVA training
    """

    placeholders = {}
    placeholders['particleName'] = particleName
    placeholders['particleLabel'] = particleLabel
    placeholders['channelName'] = channelName
    placeholders['isIgnored'] = False
    placeholders = automaticReporting.createPreCutTexFile(placeholders, preCutHistogram, preCutConfig, preCut)
    placeholders = automaticReporting.createMVATexFile(placeholders, mvaConfig, signalProbability, postCutConfig, postCut)

    hash = actorFramework.createHash(placeholders)
    placeholders['texFile'] = '{name}_channel_{hash}.tex'.format(name=placeholders['particleName'], hash=hash)
    if not os.path.isfile(placeholders['texFile']):
        automaticReporting.createTexFile(placeholders['texFile'], 'analysis/scripts/FullEventInterpretationChannelTemplate.tex', placeholders)

    B2INFO("Written analysis tex file for channel " + channelName)
    return {'Placeholders_{c}'.format(c=channelName): placeholders}


def WriteAnalysisFileForFSParticle(particleName, particleLabel, mvaConfig, signalProbability, postCutConfig, postCut, mcCounts):
    """
    Creates a pdf document with the PreCut and Training plots
        @param particleName name of the particle
        @param particleLabel
        @param mvaConfig configuration for mva
        @param signalProbability config filename for TMVA training
    """

    placeholders = {}
    placeholders['particleName'] = particleName
    placeholders['particleLabel'] = particleLabel
    placeholders['isIgnored'] = False
    placeholders = automaticReporting.createMVATexFile(placeholders, mvaConfig, signalProbability, postCutConfig, postCut)
    placeholders = automaticReporting.createFSParticleTexFile(placeholders, mcCounts)

    B2INFO("Written analysis tex file for final state particle " + particleName)
    return {'Placeholders_{p}_{l}'.format(p=particleName, l=particleLabel): placeholders}


def WriteAnalysisFileForCombinedParticle(particleName, particleLabel, channelPlaceholders, mcCounts):
    """
    Creates a pdf document with the PreCut and Training plots
        @param particleName name of the particle
        @param particleLabel
        @param channelPlaceholders list of all tex placeholder dictionaries of all channels
    """

    placeholders = {'channels': channelPlaceholders}
    placeholders['particleName'] = particleName
    placeholders['particleLabel'] = particleLabel
    placeholders['isIgnored'] = False
    placeholders = automaticReporting.createCombinedParticleTexFile(placeholders, channelPlaceholders, mcCounts)

    B2INFO("Written analysis tex file for intermediate particle " + particleName)
    return {'Placeholders_{p}_{l}'.format(p=particleName, l=particleLabel): placeholders}


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
