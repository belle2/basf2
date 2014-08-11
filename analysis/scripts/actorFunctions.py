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


def removeJPsiSlash(filename):
    return filename.replace('/', '')


def CountMCParticles(path, names):
    """
    Counts the number of MC Particles for every pdg code in all events
        @param path the basf2 path
        @param names of all particles
    """
    filename = 'mcParticlesCount.root'

    if not os.path.isfile(filename):
        output = register_module('VariablesToNtuple')
        output.param('variables', ['NumberOfMCParticlesInEvent({i})'.format(i=abs(pdg.from_name(name))) for name in names])
        output.param('fileName', filename)
        output.param('treeName', 'mccounts')
        path.add_module(output)
        B2INFO("Count number of MCParticles for every pdg code seperatly")
        return {}

    rootfile = ROOT.TFile(filename)
    countNtuple = rootfile.Get('mccounts')
    keys = [str(k.GetName()) for k in countNtuple.GetListOfBranches()]
    getpdg = lambda x: x[len('NumberOfMCParticlesInEvent'):]  # makeROOTCompatible removes parenthesis from variable so we don't worry about them here
    counter = {getpdg(key): sum([getattr(t, key) for t in countNtuple]) for key in keys}
    counter['NEvents'] = countNtuple.GetEntries()

    print counter

    B2INFO("Loaded number of MCParticles for every pdg code seperatly")
    return {'MCParticleCounts': counter}


def SelectParticleList(path, particleName, particleLabel):
    """
    Creates a ParticleList gathering up all particles with the given particleName
        @param path the basf2 path
        @param particleName valid pdg particle name
        @param particleLabel user defined label
        @return Resource named RawParticleList_{particleName}:{particleLabel} corresponding ParticleList is stored as {particleName}:{hash}
    """
    userLabel = actorFramework.createHash(particleName, particleLabel)
    outputList = particleName + ':' + userLabel
    modularAnalysis.selectParticle(outputList, path=path)

    B2INFO("Select Particle List {p} with label {l} in list {list}".format(p=particleName, l=particleLabel, list=outputList))
    return {'RawParticleList_{p}:{l}'.format(p=particleName, l=particleLabel): outputList}


def MakeAndMatchParticleList(path, particleName, particleLabel, channelName, daughterParticleLists, preCut):
    """
    Creates a ParticleList by combining other particleLists via the ParticleCombiner module and match MC truth for this new list.
        @param path the basf2 path
        @param particleName valid pdg particle name
        @param particleLabel user defined label
        @param channelName unique name describing the channel
        @param daughterParticleLists list of ParticleList name of every daughter particles
        @param preCut dictionary containing 'cutstring', a string which defines the cut which is applied before the combination of the daughter particles.
        @return Resource named RawParticleList_{channelName} corresponding list is stored as {particleName}:{hash}
    """
    userLabel = actorFramework.createHash(particleName, particleLabel, channelName, daughterParticleLists, preCut)
    outputList = particleName + ':' + userLabel
    # If preCut is None this channel is ignored
    if preCut is None:
        B2INFO("Make and Match Particle List {p} with label {l} for channel {c} in list {o}".format(p=particleName, l=particleLabel, c=channelName, o=outputList))
        return {'RawParticleList_{c}'.format(c=channelName): None}

    decayString = outputList + ' ==> ' + ' '.join(daughterParticleLists)
    modularAnalysis.reconDecay(decayString, preCut['cutstring'], 0, path=path)
    modularAnalysis.matchMCTruth(outputList, path=path)
    B2INFO("Make and Match Particle List {p} with label {l} for channel {c} in list {o}".format(p=particleName, l=particleLabel, c=channelName, o=outputList))
    return {'RawParticleList_{c}'.format(c=channelName): outputList}


def CopyParticleLists(path, particleName, particleLabel, inputLists, postCuts):
    """
    Creates a ParticleList gathering up all particles in the given inputLists
        @param path the basf2 path
        @param particleName valid pdg particle name
        @param particleLabel user defined label
        @param inputLists list of ParticleLists name defning which ParticleLists are copied to the new list
        @param postCuts list of dictionaries containing 'cutstring', a string which defines the cut which is applied after the reconstruction of the particle.
        @return Resource named ParticleList_{particleName}:{particleLabel} corresponding ParticleList is stored as {particleName}:{hash}
    """
    inputLists, postCuts = actorFramework.removeNones(inputLists, postCuts)
    userLabel = actorFramework.createHash(particleName, particleLabel, inputLists, postCuts)
    outputList = particleName + ':' + userLabel

    if inputLists == []:
        B2INFO("Gather Particle List {p} with label {l} in list {o}. But there are no particles to gather :-(.".format(p=particleName, l=particleLabel, o=outputList))
        return {'ParticleList_{p}:{l}'.format(p=particleName, l=particleLabel): None,
                'ParticleList_{p}:{l}'.format(p=pdg.conjugate(particleName), l=particleLabel): None}

    if not all(postCuts[0] == postCut for postCut in postCuts):
        B2WARNING("Different post cuts for ParticleLists which are gathered up in the same list isn't supported at the moment. Using only first cut.")
    modularAnalysis.cutAndCopyLists(outputList, inputLists, postCuts[0]['cutstring'], path=path)

    B2INFO("Gather Particle List {p} with label {l} in list {o}".format(p=particleName, l=particleLabel, o=outputList))
    return {'ParticleList_{p}:{l}'.format(p=particleName, l=particleLabel): outputList,
            'ParticleList_{p}:{l}'.format(p=pdg.conjugate(particleName), l=particleLabel): pdg.conjugate(particleName) + ':' + userLabel}


def LoadGeometry(path):
    """
    Loads Geometry module
    @param path the basf2 path
    @return Resource named Geometry
    """
    gearbox = register_module('Gearbox')
    path.add_module(gearbox)
    geometry = register_module('Geometry')
    geometry.param('components', ['MagneticField'])
    path.add_module(geometry)
    return {'Geometry': 'dummy'}


def FitVertex(path, channelName, particleList, daughterVertices, geometry):
    """
    Fit secondary vertex of all particles in this ParticleList
        @param path the basf2 path
        @param channelName unique name describing the channel
        @param particleList ParticleList name
        @param daughterVertices to ensure all daughter particles have valid error matrices
        @param additional requirement to ensure that geometry module is loaded
        @return Resource named VertexFit_{channelName}
    """
    if particleList is None:
        B2INFO("Didn't fitted vertex for channel {c}, because channel is ignored.".format(c=channelName))
        return {'VertexFit_{c}'.format(c=channelName): None}

    pvfit = register_module('ParticleVertexFitter')
    pvfit.set_name('ParticleVertexFitter_' + particleList)
    pvfit.param('listName', particleList)
    pvfit.param('confidenceLevel', 0)
    pvfit.param('vertexFitter', 'kfitter')
    pvfit.param('fitType', 'vertex')
    path.add_module(pvfit)

    B2INFO("Fitted vertex for channel {c}.".format(c=channelName))
    return {'VertexFit_{c}'.format(c=channelName): actorFramework.createHash(channelName, particleList, daughterVertices, geometry)}


def CreatePreCutHistogram(path, particleName, channelName, preCutConfig, daughterParticleLists, additionalDependencies):
    """
    Creates ROOT file with chosen pre cut variable histogram of this channel (signal/background)
    for a given particle, before any intermediate cuts are applied.
        @param path the basf2 path
        @param particleName valid pdg particle name
        @param channelName unique name describing the channel
        @param daughterParticleLists list of ParticleList names defining all daughter particles
        @param additionalDependencies like SignalProbability of all daughter particles if needed.
        @return Resource named PreCutHistogram_{channelName} providing root filename 'CutHistograms_{channelName}:{hash}.root'
    """
    if any([daughterParticleList is None for daughterParticleList in daughterParticleLists]) or any([x is None for x in additionalDependencies]):
        B2INFO("Create pre cut histogram for channel {c}. But channel is ignored.".format(c=channelName))
        return {'PreCutHistogram_{c}'.format(c=channelName): None}

    hash = actorFramework.createHash(particleName, channelName, preCutConfig.variable, daughterParticleLists, additionalDependencies)
    filename = removeJPsiSlash('CutHistograms_{c}:{h}.root'.format(c=channelName, h=hash))

    if os.path.isfile(filename):
        B2INFO("Create pre cut histogram for channel {c}. But file already exists, so nothing to do here.".format(c=channelName))
        return {'PreCutHistogram_{c}'.format(c=channelName): (filename, particleName + ':' + hash)}
    else:
        outputList = particleName + ':' + hash + ' ==> ' + ' '.join(daughterParticleLists)
        pmake = register_module('PreCutHistMaker')
        pmake.set_name('PreCutHistMaker_{c}'.format(c=channelName))
        pmake.param('fileName', filename)
        pmake.param('decayString', outputList)
        pmake.param('variable', preCutConfig.variable)
        if preCutConfig.variable in ['M', 'Mbc']:
            mass = pdg.get(pdg.from_name(particleName)).Mass()
            pmake.param('histParams', (200, mass / 2, mass + mass / 2))
        elif preCutConfig.variable in ['Q']:
            pmake.param('histParams', (200, -1, 1))
        else:
            pmake.param('customBinning', list(reversed([1.0 / (1.5 ** i) for i in range(0, 20)])))
        path.add_module(pmake)

    B2INFO("Create pre cut histogram for channel {c}.".format(c=channelName))
    return {}


def PreCutDetermination(channelNames, preCutConfigs, preCutHistograms):
    """
    Determines the PreCuts for all the channels of a particle.
        @param channelNames list of unique names describing the channels
        @param preCutConfig configuration for PreCut determination for every chanel
        @param preCutHistograms filenames of the histogram files created for every channel by CreatePreCutHistogram
        @param Resource named PreCut_{channelName} for every channel providing a dictionary with the key 'cutstring'
    """

    results = {'PreCut_{c}'.format(c=channelName): None for channelName, _, __ in zip(*actorFramework.getNones(channelNames, preCutConfigs, preCutHistograms))}
    for channelName in channelNames:
        B2INFO("Calculate pre cut for channel {c}".format(c=channelName))
    channelNames, preCutConfigs, preCutHistograms = actorFramework.removeNones(channelNames, preCutConfigs, preCutHistograms)

    if len(channelNames) == 0:
        return results

    if not all(preCutConfig[0] == preCutConfig for preCutConfig in preCutConfigs):
        B2WARNING("Different pre cuts for channels of the same particle, aren't supported at the moment. Using only first cut.")
    preCutConfig = preCutConfigs[0]

    for (channelName, cut) in preCutDetermination.CalculatePreCuts(preCutConfig, channelNames, preCutHistograms).iteritems():
        results['PreCut_{c}'.format(c=channelName)] = None if cut['isIgnored'] else cut
        B2INFO("Calculated pre cut for channel {c}".format(c=channelName))
    return results


def PostCutDetermination(identifiers, postCutConfigs, signalProbabilities):
    """
    Determines the PostCut for all the channels of a particle.
        @param identifiers list of unique identifiers describing the channels or the particle
        @param postCutConfigs list of configurations for post cut determination
        @param signalProbabilities of the channels
        @param Resource named PostCut_{identifier} for every channel providing a dictionary with the key 'cutstring'
    """
    results = {'PostCut_{i}'.format(i=identifier): None for identifier, _, __ in zip(*actorFramework.getNones(identifiers, postCutConfigs, signalProbabilities))}
    identifiers, postCutConfigs, signalProbabilities = actorFramework.removeNones(identifiers, postCutConfigs, signalProbabilities)
    for identifier, postCutConfig in zip(identifiers, postCutConfigs):
        results['PostCut_{i}'.format(i=identifier)] = {'cutstring': str(postCutConfig.value) + ' < getExtraInfo(SignalProbability)', 'range': (postCutConfig.value, 1)}
        B2INFO("Calculate post cut for {i}".format(i=identifier))
    return results


def SignalProbability(path, identifier, particleList, mvaConfig, additionalDependencies=[]):
    """
    Calculates the SignalProbability of a ParticleList. If the files required from TMVAExpert aren't available they're created.
        @param path the basf2 path
        @param identifier unique identifier describing the channel or the particle
        @param particleList the particleList which is used for training and classification
        @param mvaConfig configuration for the multivariate analysis
        @param additionalDependencies for variables like SignalProbability of daughters or VertexFit
        @return Resource named SignalProbability_{identifier} providing config filename
    """
    if particleList is None or any([d is None for d in additionalDependencies]):
        B2INFO("Calculate SignalProbability for {i}, but particle/channel is ignored".format(i=identifier))
        return{'SignalProbability_{i}'.format(i=identifier): None}

    hash = actorFramework.createHash(identifier, mvaConfig, particleList, additionalDependencies)
    rootFilename = removeJPsiSlash('{particleList}_{hash}.root'.format(particleList=particleList, hash=hash))
    configFilename = removeJPsiSlash('{particleList}_{hash}.config'.format(particleList=particleList, hash=hash))

    if not os.path.isfile(rootFilename):
        teacher = register_module('TMVATeacher')
        teacher.set_name('TMVATeacher_' + particleList)
        teacher.param('prefix', removeJPsiSlash(particleList + '_' + hash))
        teacher.param('methods', [(mvaConfig.name, mvaConfig.type, mvaConfig.config)])
        teacher.param('factoryOption', '!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification')
        teacher.param('prepareOption', 'SplitMode=random:!V')
        teacher.param('variables', mvaConfig.variables)
        teacher.param('target', mvaConfig.target)
        teacher.param('listNames', [particleList])
        teacher.param('maxEventsPerClass', 10000000)
        teacher.param('doNotTrain', True)
        path.add_module(teacher)
        B2INFO("Calculate SignalProbability for {i}. Create root file with variables first.".format(i=identifier))
        return {}

    if not os.path.isfile(configFilename):
        B2INFO("Calculate SignalProbability for {i}. Run Teacher in extern process.".format(i=identifier))
        subprocess.call("externTeacher --methodName '{name}' --methodType '{type}' --methodConfig '{config}' --target '{target}'"
                        " --variables '{variables}' --factoryOption '{foption}' --prepareOption '{poption}' --prefix '{prefix}'"
                        " --maxEventsPerClass {maxEvents}"
                        " > '{prefix}'.log".format(name=mvaConfig.name, type=mvaConfig.type, config=mvaConfig.config,
                                                   target=mvaConfig.target, variables="' '".join(mvaConfig.variables),
                                                   foption='!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification',
                                                   poption='SplitMode=random:!V', maxEvents=10000000,
                                                   prefix=removeJPsiSlash(particleList + '_' + hash)), shell=True)

    if os.path.isfile(configFilename):
        expert = register_module('TMVAExpert')
        expert.set_name('TMVAExpert_' + particleList)
        expert.param('prefix', removeJPsiSlash(particleList + '_' + hash))
        expert.param('method', mvaConfig.name)
        expert.param('signalFraction', -2)  # Use signalFraction from training
        expert.param('signalProbabilityName', 'SignalProbability')
        expert.param('signalClass', mvaConfig.targetCluster)
        expert.param('listNames', [particleList])
        path.add_module(expert)
        B2INFO("Calculating SignalProbability for {i}".format(i=identifier))
        return{'SignalProbability_{i}'.format(i=identifier): configFilename}

    B2ERROR("Training of {i} failed!".format(i=identifier))
    return {}


def VariablesToNTuple(path, particleIdentifier, particleList, signalProbability):
    """
    Saves the calculated signal probability for this particle list
        @param path the basf2 path
        @param particleIdentifier valid pdg particle name + optional user label seperated by :
        @param particleList the particleList
        @param signalProbability signalProbability as additional dependency
        @return Resource named VariablesToNTuple_{particleIdentifier} providing root filename
    """
    if particleList is None or signalProbability is None:
        B2INFO("Write variables to ntuple for particle {p}. But list is ignored.".format(p=particleIdentifier))
        return {'VariablesToNTuple_{i}'.format(i=particleIdentifier): None}

    hash = actorFramework.createHash(particleIdentifier, particleList, signalProbability)
    filename = removeJPsiSlash('var_{i}_{h}.root'.format(i=particleIdentifier, h=hash))

    if not os.path.isfile(filename):
        output = register_module('VariablesToNtuple')
        output.param('particleList', particleList)
        output.param('variables', ['getExtraInfo(SignalProbability)', 'isSignal', 'Mbc', 'mcStatus'])
        output.param('fileName', filename)
        output.param('treeName', 'variables')
        path.add_module(output)
        B2INFO("Write variables to ntuple for particle {i}.".format(i=particleIdentifier))
        return {}

    B2INFO("Write variables to ntuple for particle {i}. But file already exists, so nothing to do here.".format(i=particleIdentifier))
    return {'VariablesToNTuple_{i}'.format(i=particleIdentifier): filename}


def WriteAnalysisFileForChannel(particleName, particleLabel, channelName, preCutConfig, preCut, preCutHistogram, mvaConfig, signalProbability, postCutConfig, postCut):
    """
    Creates a pdf document with the PreCut and Training plots
        @param particleName valid pdg particle name
        @param particleLabel user defined label
        @param channelName name of the channel
        @param preCutConfig configuration for pre cut
        @param preCut used preCuts for this channel
        @param preCutHistogram preCutHistogram (filename, histogram postfix)
        @param mvaConfig configuration for mva
        @param signalProbability config filename for TMVA training
        @param postCutConfig configuration for postCut
        @param postCut
        @return Resource named Placeholders_{channelName} providing latex placeholders of this channel
    """

    placeholders = {}
    placeholders['particleName'] = particleName
    placeholders['particleLabel'] = particleLabel
    placeholders['channelName'] = channelName
    placeholders['isIgnored'] = False
    placeholders = automaticReporting.createPreCutTexFile(placeholders, preCutHistogram, preCutConfig, preCut)
    placeholders = automaticReporting.createMVATexFile(placeholders, mvaConfig, signalProbability, postCutConfig, postCut)

    hash = actorFramework.createHash(placeholders)
    placeholders['texFile'] = removeJPsiSlash('{name}_channel_{hash}.tex'.format(name=placeholders['particleName'], hash=hash))
    if not os.path.isfile(placeholders['texFile']):
        automaticReporting.createTexFile(placeholders['texFile'], 'analysis/scripts/FullEventInterpretationChannelTemplate.tex', placeholders)

    B2INFO("Written analysis tex file for channel {c}.".format(c=channelName))
    return {'Placeholders_{c}'.format(c=channelName): placeholders}


def WriteAnalysisFileForFSParticle(particleName, particleLabel, mvaConfig, signalProbability, postCutConfig, postCut, mcCounts):
    """
    Creates a pdf document with the PreCut and Training plots
        @param particleName valid pdg particle name
        @param particleLabel user defined label
        @param mvaConfig configuration for mva
        @param signalProbability config filename for TMVA training
        @param postCutConfig configuration for postCut
        @param postCut
        @param mcCounts
        @return Resource named Placeholders_{particleName}:{particleLabel} providing latex placeholders of this particle
    """

    placeholders = {}
    placeholders['particleName'] = particleName
    placeholders['particleLabel'] = particleLabel
    placeholders['isIgnored'] = False

    placeholders = automaticReporting.createMVATexFile(placeholders, mvaConfig, signalProbability, postCutConfig, postCut)
    placeholders = automaticReporting.createFSParticleTexFile(placeholders, mcCounts)

    B2INFO("Written analysis tex file for final state particle {p} with label {l}.".format(p=particleName, l=particleLabel))
    return {'Placeholders_{p}:{l}'.format(p=particleName, l=particleLabel): placeholders}


def WriteAnalysisFileForCombinedParticle(particleName, particleLabel, channelPlaceholders, mcCounts):
    """
    Creates a pdf document with the PreCut and Training plots
        @param particleName valid pdg particle name
        @param particleLabel user defined label
        @param channelPlaceholders list of all tex placeholder dictionaries of all channels
        @param mcCounts
        @return Resource named Placeholders_{particleName}:{particleLabel} providing latex placeholders of this particle
    """

    placeholders = {'channels': channelPlaceholders}
    placeholders['particleName'] = particleName
    placeholders['particleLabel'] = particleLabel
    placeholders['isIgnored'] = False
    placeholders = automaticReporting.createCombinedParticleTexFile(placeholders, channelPlaceholders, mcCounts)

    B2INFO("Written analysis tex file for intermediate particle {p} with label {l}.".format(p=particleName, l=particleLabel))
    return {'Placeholders_{p}:{l}'.format(p=particleName, l=particleLabel): placeholders}


def WriteAnalysisFileSummary(finalStateParticlePlaceholders, combinedParticlePlaceholders, finalParticleNTuples, mcCounts, particles):
    """
    Creates a pdf summarizing all networks trained.
        @param finalStateParticlePlaceholders list of all tex placeholder dictionaries of fsp
        @param combinedParticlePlaceholders list of all tex placeholder dictionaries of intermediate particles
        @param finalParticleNTuples list of ntuples of all final particles
        @param mcCounts
        @param particles particle objects
        @return Resource named FEIsummary.pdf
    """

    finalParticlePlaceholders = []
    for ntuple in finalParticleNTuples:
        if ntuple is not None:
            finalParticlePlaceholders.append(automaticReporting.createMBCTexFile(ntuple))
    placeholders = automaticReporting.createSummaryTexFile(finalStateParticlePlaceholders, combinedParticlePlaceholders, finalParticlePlaceholders, mcCounts, particles)

    for i in range(0, 2):
        ret = subprocess.call(['pdflatex', '-halt-on-error', placeholders['texFile']])
        if ret == 0:
            B2INFO("Created FEI summary PDF.")
        else:
            B2ERROR("pdflatex failed to create FEI summary PDF, please check.")

    # Return None - Therefore Particle List depends not on TMVAExpert directly
    B2INFO("Created analysis summary pdf file.")
    return {'FEIsummary.pdf': None}
