#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Thomas Keck 2014
#
# All the actors functions are defined here.
# To create your own actor:
#   1. Write a normal function which takes the needed arguments and returns a dictonary of provided values.
#      E.g. def foo(path, particleList) ... return {'Stuff': x}
#   2. Make sure your return value depends on all the used arguments, easiest way to accomplish this is the automatic provided hash parameter.
#   3. Add the function to the sequence object like this (in FullEventInterpretation.py):
#      seq.addFunction(foo, path='Path', particleList='K+')

from basf2 import *
import ROOT
from ROOT import Belle2

import modularAnalysis
import pdg

import actorFramework
import preCutDetermination
import automaticReporting

import re
import os
import subprocess
import json
from string import Template


def removeJPsiSlash(filename):
    """
    Removes the character / from the given filename
    """
    return filename.replace('/', '')


def CountMCParticles(path, names):
    """
    Counts the number of MC Particles for every pdg code in all events
        @param path the basf2 path
        @param names of all particles
    """
    B2INFO("Load number of MCParticles for every pdg code seperatly")
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
    getpdg = lambda x: x[len('NumberOfMCParticlesInEvent'):]  # makeROOTCompatible removes parenthesis from variable so we don't worry about them here

    counter = {}
    for branch in [str(k.GetName()) for k in countNtuple.GetListOfBranches()]:
        hallo = ROOT.TH1D('hallo', 'hallo', 1, 0, countNtuple.GetMaximum(branch) + 1)
        countNtuple.Project('hallo', branch, branch)
        counter[getpdg(branch)] = hallo.Integral()
        del hallo
    counter['NEvents'] = countNtuple.GetEntries()
    B2INFO("Loaded number of MCParticles for every pdg code seperatly")
    return {'mcCounts': counter, '__cache__': True}


def FSPDistribution(path, hash, identifier, inputList, mvaConfigTarget):
    """
    Returns signal and background distribution of FSP
    Counts the number of MC Particles for every pdg code in all events
        @param path the basf2 path
        @param identifier unique identifier of the particle
        @param inputList particle list name
        @param mvaConfig configuration for the multivariate analysis
    """
    B2INFO("Calculate signal and background candiates of FSP {i}".format(i=inputList))
    filename = removeJPsiSlash('{i}_{h}.root'.format(i=inputList, h=hash))

    if not os.path.isfile(filename):
        output = register_module('VariablesToNtuple')
        output.param('particleList', inputList)
        output.param('variables', [mvaConfigTarget])
        output.param('fileName', filename)
        output.param('treeName', 'distribution')
        path.add_module(output)
        B2INFO("Add VariableToNtuple to count signal and background candiates of FSP {i}".format(i=inputList))
        return {}

    rootfile = ROOT.TFile(filename)
    distribution = rootfile.Get('distribution')
    result = {'nSignal': distribution.GetEntries(mvaConfigTarget + ' == 1'), 'nBackground': distribution.GetEntries(mvaConfigTarget + ' == 0')}
    B2INFO("Calculated signal and background candiates of FSP {i}".format(i=inputList))
    return {'Distribution_{i}'.format(i=identifier): result, '__cache__': True}


def LoadParticles(path):
    """
    Loads Particles
    @param path the basf2 path
    @return Resource named ParticleLoader
    """
    B2INFO("Adding ParticleLoader")
    if preloader.treeContainsObject('Particles'):
        B2INFO("Preload Particles Array")
        return {'particleLoader': 'dummy', '__needed__': False}
    path.add_module(register_module('ParticleLoader'))
    B2INFO("Added Particles Array")
    return {'particleLoader': 'dummy', '__cache__': True}


def SelectParticleList(path, hash, particleLoader, particleName, particleLabel, runs_in_ROE):
    """
    Creates a ParticleList gathering up all particles with the given particleName
        @param path the basf2 path
        @param hash of all input parameters
        @param particleLoader
        @param particleName valid pdg particle name
        @param particleLabel user defined label
        @param runs_in_ROE boolean determines if particles are selected frmo ROE
        @return Resource named RawParticleList_{particleName}:{particleLabel} corresponding ParticleList is stored as {particleName}:{hash}
    """
    B2INFO("Enter: Select Particle List {p} with label {l}".format(p=particleName, l=particleLabel))
    outputList = particleName + ':' + hash
    modularAnalysis.selectParticle(outputList, 'isInRestOfEvent > 0.5' if runs_in_ROE else '', persistent=True, path=path)
    B2INFO("Select Particle List {p} with label {l} in list {list}".format(p=particleName, l=particleLabel, list=outputList))
    return {'RawParticleList_{p}:{l}'.format(p=particleName, l=particleLabel): outputList, '__cache__': True}


def MakeAndMatchParticleList(path, hash, particleName, particleLabel, channelName, daughterParticleLists, preCut):
    """
    Creates a ParticleList by combining other particleLists via the ParticleCombiner module and match MC truth for this new list.
        @param path the basf2 path
        @param hash of all input parameters
        @param particleName valid pdg particle name
        @param particleLabel user defined label
        @param channelName unique name describing the channel
        @param daughterParticleLists list of ParticleList name of every daughter particles
        @param preCut dictionary containing 'cutstring', a string which defines the cut which is applied before the combination of the daughter particles.
        @return Resource named RawParticleList_{channelName} corresponding list is stored as {particleName}:{hash}
    """
    B2INFO("Enter: Make and Match Particle List {p} with label {l} for channel {c}".format(p=particleName, l=particleLabel, c=channelName))
    outputList = particleName + ':' + hash

    # If preCut is None this channel is ignored
    if preCut is None:
        B2INFO("Make and Match Particle List {p} with label {l} for channel {c} in list {o}".format(p=particleName, l=particleLabel, c=channelName, o=outputList))
        return {'RawParticleList_{c}'.format(c=channelName): None, '__cache__': True}

    decayString = outputList + ' ==> ' + ' '.join(daughterParticleLists)
    modularAnalysis.reconstructDecay(decayString, preCut['cutstring'], 0, persistent=True, path=path)
    modularAnalysis.matchMCTruth(outputList, path=path)
    B2INFO("Make and Match Particle List {p} with label {l} for channel {c} in list {o}".format(p=particleName, l=particleLabel, c=channelName, o=outputList))
    return {'RawParticleList_{c}'.format(c=channelName): outputList, '__cache__': True}


def CopyParticleLists(path, hash, particleName, particleLabel, inputLists, postCut):
    """
    Creates a ParticleList gathering up all particles in the given inputLists
        @param path the basf2 path
        @param hash of all input parameters
        @param particleName valid pdg particle name
        @param particleLabel user defined label
        @param inputLists list of ParticleLists name defning which ParticleLists are copied to the new list
        @param postCut dictionary containing 'cutstring'
        @return Resource named ParticleList_{particleName}:{particleLabel} corresponding ParticleList is stored as {particleName}:{hash}
    """
    B2INFO("Enter: Gather Particle List {p} with label {l}".format(p=particleName, l=particleLabel))
    outputList = particleName + ':' + hash

    inputLists = [l for l in inputLists if l is not None]
    if inputLists == []:
        B2INFO("Gather Particle List {p} with label {l} in list {o}. But there are no particles to gather :-(.".format(p=particleName, l=particleLabel, o=outputList))
        return {'ParticleList_{p}:{l}'.format(p=particleName, l=particleLabel): None,
                'ParticleList_{p}:{l}'.format(p=pdg.conjugate(particleName), l=particleLabel): None,
                '__cache__': True}

    modularAnalysis.cutAndCopyLists(outputList, inputLists, postCut['cutstring'] if postCut is not None else '', persistent=True, path=path)
    modularAnalysis.cutAndCopyLists(particleName + ':' + particleLabel, outputList, '', persistent=True, path=path)
    #modularAnalysis.summaryOfLists(inputLists + [outputList], path=path)

    B2INFO("Gather Particle List {p} with label {l} in list {o}".format(p=particleName, l=particleLabel, o=outputList))
    return {'ParticleList_{p}:{l}'.format(p=particleName, l=particleLabel): outputList,
            'ParticleList_{p}:{l}'.format(p=pdg.conjugate(particleName), l=particleLabel): pdg.conjugate(particleName) + ':' + hash,
            '__cache__': True}


def LoadGeometry(path):
    """
    Loads Geometry module
    @param path the basf2 path
    @return Resource named Geometry
    """
    B2INFO("Adding Geometry and Gearbox to Path")
    gearbox = register_module('Gearbox')
    path.add_module(gearbox)
    geometry = register_module('Geometry')
    geometry.param('components', ['MagneticField'])
    path.add_module(geometry)
    B2INFO("Added Geometry and Gearbox to Path")
    return {'geometry': 'dummy'}


def FitVertex(path, hash, channelName, particleList, daughterVertices, geometry):
    """
    Fit secondary vertex of all particles in this ParticleList
        @param path the basf2 path
        @param hash of all input parameters
        @param channelName unique name describing the channel
        @param particleList ParticleList name
        @param daughterVertices to ensure all daughter particles have valid error matrices
        @param additional requirement to ensure that geometry module is loaded
        @return Resource named VertexFit_{channelName}
    """
    B2INFO("Enter: Fitted vertex for channel {c}.".format(c=channelName))
    if particleList is None:
        B2INFO("Didn't fitted vertex for channel {c}, because channel is ignored.".format(c=channelName))
        return {'VertexFit_{c}'.format(c=channelName): None, '__cache__': True}

    if re.findall(r"[\w']+", channelName).count('pi0') > 1:
        B2INFO("Ignore vertex fit for this channel because multiple pi0 are not supported yet {c}.".format(c=channelName))
        return {'VertexFit_{c}'.format(c=channelName): hash, '__cache__': True}

    pvfit = register_module('ParticleVertexFitter')
    pvfit.set_name('ParticleVertexFitter_' + particleList)
    pvfit.param('listName', particleList)
    pvfit.param('confidenceLevel', 0)
    pvfit.param('vertexFitter', 'kfitter')
    pvfit.param('fitType', 'vertex')
    pvfit.param('dontDiscardOnError', True)
    path.add_module(pvfit)

    B2INFO("Fitted vertex for channel {c}.".format(c=channelName))
    return {'VertexFit_{c}'.format(c=channelName): hash, '__cache__': True}


def CreatePreCutHistogram(path, hash, particleName, channelName, mvaConfigTarget, preCutConfig, daughterParticleLists, additionalDependencies):
    """
    Creates ROOT file with chosen pre cut variable histogram of this channel (signal/background)
    for a given particle, before any intermediate cuts are applied.
        @param path the basf2 path
        @param particleName valid pdg particle name
        @param channelName unique name describing the channel
        @param daughterParticleLists list of ParticleList names defining all daughter particles
        @param target variable which defines signal and background
        @param additionalDependencies like SignalProbability of all daughter particles if needed.
        @return Resource named PreCutHistogram_{channelName} providing root filename 'CutHistograms_{channelName}:{hash}.root'
    """
    B2INFO("Enter: Create pre cut histogram for channel {c}.".format(c=channelName))
    if any([daughterParticleList is None for daughterParticleList in daughterParticleLists]) or any([x is None for x in additionalDependencies]):
        B2INFO("Create pre cut histogram for channel {c}. But channel is ignored.".format(c=channelName))
        return {'PreCutHistogram_{c}'.format(c=channelName): None, '__cache__': True}

    filename = removeJPsiSlash('CutHistograms_{c}:{h}.root'.format(c=channelName, h=hash))

    if os.path.isfile(filename):
        B2INFO("Create pre cut histogram for channel {c}. But file already exists, so nothing to do here.".format(c=channelName))
        return {'PreCutHistogram_{c}'.format(c=channelName): (filename, particleName + ':' + hash), '__cache__': True}
    else:
        outputList = particleName + ':' + hash + ' ==> ' + ' '.join(daughterParticleLists)
        pmake = register_module('PreCutHistMaker')
        pmake.set_name('PreCutHistMaker_{c}'.format(c=channelName))
        pmake.param('fileName', filename)
        pmake.param('decayString', outputList)
        pmake.param('cut', preCutConfig.userCut)
        pmake.param('target', mvaConfigTarget)
        pmake.param('variable', preCutConfig.variable)
        if isinstance(preCutConfig.binning, tuple):
            pmake.param('histParams', preCutConfig.binning)
        else:
            pmake.param('customBinning', preCutConfig.binning)
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
    B2INFO("Enter: Calculation of pre cuts")
    results = {'PreCut_{c}'.format(c=channelName): None for channelName in channelNames}
    for channelName in channelNames:
        B2INFO("Calculate pre cut for channel {c}".format(c=channelName))

    channelNames, preCutConfigs, preCutHistograms = actorFramework.removeNones(channelNames, preCutConfigs, preCutHistograms)

    if len(channelNames) == 0:
        return results

    if not all(preCutConfigs[0] == preCutConfig for preCutConfig in preCutConfigs):
        B2WARNING("Different pre cuts for channels of the same particle, aren't supported at the moment. Using only first cut.")
    preCutConfig = preCutConfigs[0]

    for (channelName, cut) in preCutDetermination.CalculatePreCuts(preCutConfig, channelNames, preCutHistograms).iteritems():
        if preCutConfig.userCut != '':
            cut['cutstring'] += ' and ' + preCutConfig.userCut
        results['PreCut_{c}'.format(c=channelName)] = None if cut['isIgnored'] else cut
        B2INFO("Calculated pre cut for channel {c}".format(c=channelName))
    results['__cache__'] = True
    return results


def PostCutDetermination(identifier, postCutConfig, signalProbabilities):
    """
    Determines the PostCut for all the channels of a particle.
        @param identifier of the particle
        @param postCutConfig configurations for post cut determination
        @param signalProbabilities of the channels
        @param Resource named PostCut_{identifier} providing a dictionary with the key 'cutstring'
    """
    B2INFO("Enter: Calculation of post cuts")
    if postCutConfig is None:
        B2INFO("Calculate post cut for {i} but nothing todo becaus PostCutConfig is None".format(i=identifier))
        return {'PostCut_{i}'.format(i=identifier): None, '__needed__': False, '__cache__': True}
    else:
        B2INFO("Calculate post cut for {i}".format(i=identifier))
        return {'PostCut_{i}'.format(i=identifier): {'cutstring': str(postCutConfig.value) + ' < getExtraInfo(SignalProbability)', 'range': (postCutConfig.value, 1)}, '__cache__': True}


def SignalProbability(path, hash, identifier, particleList, mvaConfig, distribution, additionalDependencies):
    """
    Calculates the SignalProbability of a ParticleList. If the files required from TMVAExpert aren't available they're created.
        @param path the basf2 path
        @param hash of all input parameters
        @param identifier unique identifier describing the channel or the particle
        @param particleList the particleList which is used for training and classification
        @param mvaConfig configuration for the multivariate analysis
        @param additionalDependencies for variables like SignalProbability of daughters or VertexFit
        @param distribution information about number of background an signal events
        @return Resource named SignalProbability_{identifier} providing config filename
    """
    B2INFO("Enter: Calculate SignalProbability for {i}.".format(i=identifier))
    if particleList is None or (additionalDependencies is not None and any([d is None for d in additionalDependencies])):
        B2INFO("Calculate SignalProbability for {i}, but particle/channel is ignored".format(i=identifier))
        return{'SignalProbability_{i}'.format(i=identifier): None, '__cache__': True}

    rootFilename = removeJPsiSlash('{particleList}_{hash}.root'.format(particleList=particleList, hash=hash))
    configFilename = removeJPsiSlash('{particleList}_{hash}.config'.format(particleList=particleList, hash=hash))

    maxEvents = int(1e7)
    inverseSamplingRates = {}
    if distribution['nBackground'] > maxEvents:
        inverseSamplingRates[0] = int(distribution['nBackground'] / maxEvents) + 1
    if distribution['nSignal'] > maxEvents:
        inverseSamplingRates[1] = int(distribution['nSignal'] / maxEvents) + 1

    Nbins = 'NbinsMVAPdf=50:'
    if distribution['nSignal'] > 1e5:
        Nbins = 'NbinsMVAPdf=100:'
    if distribution['nSignal'] > 1e6:
        Nbins = 'NbinsMVAPdf=200:'

    if not os.path.isfile(rootFilename):
        teacher = register_module('TMVATeacher')
        teacher.set_name('TMVATeacher_' + particleList)
        teacher.param('prefix', removeJPsiSlash(particleList + '_' + hash))
        teacher.param('methods', [(mvaConfig.name, mvaConfig.type, Nbins + mvaConfig.config)])  # Add number of bins for pdfs
        teacher.param('factoryOption', '!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification')
        teacher.param('prepareOption', 'SplitMode=random:!V')
        teacher.param('variables', mvaConfig.variables)
        teacher.param('createMVAPDFs', True)
        teacher.param('target', mvaConfig.target)
        teacher.param('listNames', [particleList])
        teacher.param('inverseSamplingRates', inverseSamplingRates)
        teacher.param('doNotTrain', True)
        path.add_module(teacher)
        B2INFO("Calculate SignalProbability for {i}. Create root file with variables first with prefix {p}.".format(i=identifier, p=removeJPsiSlash(particleList + '_' + hash)))
        return {}

    if not os.path.isfile(configFilename):
        B2INFO("Calculate SignalProbability for {i}. Run Teacher in extern process.".format(i=identifier))
        command = ("externTeacher --methodName '{name}' --methodType '{type}' --methodConfig '{config}' --target '{target}'"
                   " --variables '{variables}' --factoryOption '{foption}' --prepareOption '{poption}' --prefix '{prefix}'"
                   " --maxEventsPerClass {maxEvents}"
                   " > '{prefix}'.log".format(name=mvaConfig.name, type=mvaConfig.type, config='CreateMVAPdfs:' + Nbins + mvaConfig.config,
                                              target=mvaConfig.target, variables="' '".join(mvaConfig.variables),
                                              foption='!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification',
                                              poption='SplitMode=random:!V', maxEvents=maxEvents,
                                              prefix=removeJPsiSlash(particleList + '_' + hash)))
        B2INFO("Use following command to invoke teacher\n" + command)
        actorFramework.global_lock.release()
        subprocess.call(command, shell=True)
        actorFramework.global_lock.acquire()

    if os.path.isfile(configFilename):
        expert = register_module('TMVAExpert')
        expert.set_name('TMVAExpert_' + particleList)
        expert.param('prefix', removeJPsiSlash(particleList + '_' + hash))
        expert.param('method', mvaConfig.name)
        expert.param('signalFraction', -2)  # Use signalFraction from training
        #expert.param('signalFraction', -1)  # No transformation of output
        expert.param('signalProbabilityName', 'SignalProbability')
        expert.param('signalClass', 1)
        expert.param('inverseSamplingRates', inverseSamplingRates)
        expert.param('listNames', [particleList])
        path.add_module(expert)
        B2INFO("Calculating SignalProbability for {i}".format(i=identifier))
        return{'SignalProbability_{i}'.format(i=identifier): configFilename, '__cache__': True}

    B2ERROR("Training of {i} failed!".format(i=identifier))
    return {'__needed__': False}


def VariablesToNTuple(path, hash, particleIdentifier, particleList, signalProbability):
    """
    Saves the calculated signal probability for this particle list
        @param path the basf2 path
        @param hash of all input parameters
        @param particleIdentifier valid pdg particle name + optional user label seperated by :
        @param particleList the particleList
        @param signalProbability signalProbability as additional dependency
        @return Resource named VariablesToNTuple_{particleIdentifier} providing root filename
    """
    B2INFO("Enter: Write variables to ntuple for particle {i}".format(i=particleIdentifier))
    if particleList is None or signalProbability is None:
        B2INFO("Write variables to ntuple for particle {p}. But list is ignored.".format(p=particleIdentifier))
        return {'VariablesToNTuple_{i}'.format(i=particleIdentifier): None, '__cache__': True}

    filename = removeJPsiSlash('var_{i}_{h}.root'.format(i=particleIdentifier, h=hash))

    if not os.path.isfile(filename):
        output = register_module('VariablesToNtuple')
        output.set_name('VariablesToNtuple_' + particleList)
        output.param('particleList', particleList)
        output.param('variables', ['getExtraInfo(SignalProbability)', 'isSignal', 'isSignalAcceptMissingNeutrino', 'Mbc', 'mcStatus', 'cosThetaBetweenParticleAndTrueB'])
        output.param('fileName', filename)
        output.param('treeName', 'variables')
        path.add_module(output)
        B2INFO("Write variables to ntuple for particle {i}.".format(i=particleIdentifier))
        return {}

    B2INFO("Write variables to ntuple for particle {i}. But file already exists, so nothing to do here.".format(i=particleIdentifier))
    return {'VariablesToNTuple_{i}'.format(i=particleIdentifier): filename, '__cache__': True}


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

    B2INFO("Write analysis tex file for channel {c}.".format(c=channelName))
    placeholders = {}
    placeholders['particleName'] = particleName
    placeholders['particleLabel'] = particleLabel
    placeholders['channelName'] = channelName
    placeholders['isIgnored'] = False
    placeholders['mvaConfigObject'] = mvaConfig
    placeholders = automaticReporting.createPreCutTexFile(placeholders, preCutHistogram, preCutConfig, preCut)
    placeholders = automaticReporting.createMVATexFile(placeholders, mvaConfig, signalProbability, postCutConfig, postCut)

    hash = actorFramework.create_hash([placeholders])
    placeholders['texFile'] = removeJPsiSlash('{name}_channel_{hash}.tex'.format(name=placeholders['particleName'], hash=hash))
    if not os.path.isfile(placeholders['texFile']):
        automaticReporting.createTexFile(placeholders['texFile'], 'analysis/scripts/FEI/templates/ChannelTemplate.tex', placeholders)

    B2INFO("Written analysis tex file for channel {c}.".format(c=channelName))
    return {'Placeholders_{c}'.format(c=channelName): placeholders, '__needed__': False}


def WriteAnalysisFileForFSParticle(particleName, particleLabel, mvaConfig, signalProbability, postCutConfig, postCut, distribution, nTuple, mcCounts):
    """
    Creates a pdf document with the PreCut and Training plots
        @param particleName valid pdg particle name
        @param particleLabel user defined label
        @param mvaConfig configuration for mva
        @param signalProbability config filename for TMVA training
        @param postCutConfig configuration for postCut
        @param postCut
        @param mcCounts
        @param distribution
        @return Resource named Placeholders_{particleName}:{particleLabel} providing latex placeholders of this particle
    """

    B2INFO("Write analysis tex file for final state particle {p} with label {l}.".format(p=particleName, l=particleLabel))
    placeholders = {}
    placeholders['particleName'] = particleName
    placeholders['particleLabel'] = particleLabel
    placeholders['isIgnored'] = False

    placeholders = automaticReporting.createMVATexFile(placeholders, mvaConfig, signalProbability, postCutConfig, postCut)
    placeholders = automaticReporting.createFSParticleTexFile(placeholders, nTuple, mcCounts, distribution, mvaConfig)

    B2INFO("Written analysis tex file for final state particle {p} with label {l}.".format(p=particleName, l=particleLabel))
    return {'Placeholders_{p}:{l}'.format(p=particleName, l=particleLabel): placeholders, '__needed__': False}


def WriteAnalysisFileForCombinedParticle(particleName, particleLabel, channelPlaceholders, nTuple, mcCounts):
    """
    Creates a pdf document with the PreCut and Training plots
        @param particleName valid pdg particle name
        @param particleLabel user defined label
        @param channelPlaceholders list of all tex placeholder dictionaries of all channels
        @param mcCounts
        @return Resource named Placeholders_{particleName}:{particleLabel} providing latex placeholders of this particle
    """

    B2INFO("Write analysis tex file for intermediate particle {p} with label {l}.".format(p=particleName, l=particleLabel))
    placeholders = {'channels': channelPlaceholders}
    placeholders['particleName'] = particleName
    placeholders['particleLabel'] = particleLabel
    placeholders['isIgnored'] = False

    mvaConfig = channelPlaceholders[0]['mvaConfigObject']
    placeholders = automaticReporting.createCombinedParticleTexFile(placeholders, channelPlaceholders, nTuple, mcCounts, mvaConfig)

    B2INFO("Written analysis tex file for intermediate particle {p} with label {l}.".format(p=particleName, l=particleLabel))
    return {'Placeholders_{p}:{l}'.format(p=particleName, l=particleLabel): placeholders, '__needed__': False}


def WriteAnalysisFileSummary(finalStateParticlePlaceholders, combinedParticlePlaceholders, finalParticleNTuples, cpuTimeSummaryPlaceholders, mcCounts, particles):
    """
    Creates a pdf summarizing all networks trained.
        @param finalStateParticlePlaceholders list of all tex placeholder dictionaries of fsp
        @param combinedParticlePlaceholders list of all tex placeholder dictionaries of intermediate particles
        @param finalParticleNTuples list of ntuples of all final particles
        @param mcCounts
        @param particles particle objects
        @return Resource named FEIsummary.pdf
    """

    B2INFO("Create analysis summary pdf file.")
    finalParticlePlaceholders = []
    for ntuple in finalParticleNTuples:
        if ntuple is not None:
            type = 'CosBDL' if 'semileptonic' in ntuple else 'Mbc'
            finalParticlePlaceholders.append(automaticReporting.createMoneyPlotTexFile(ntuple, type))
    placeholders = automaticReporting.createSummaryTexFile(finalStateParticlePlaceholders, combinedParticlePlaceholders, finalParticlePlaceholders, cpuTimeSummaryPlaceholders, mcCounts, particles)

    subprocess.call('cp {f} .'.format(f=ROOT.Belle2.FileSystem.findFile('analysis/scripts/nordbert.pdf')), shell=True)
    for i in range(0, 2):
        ret = subprocess.call(['pdflatex', '-halt-on-error', '-interaction=nonstopmode', placeholders['texFile']])
        if ret == 0:
            B2INFO("Created FEI summary PDF.")
        else:
            B2ERROR("pdflatex failed to create FEI summary PDF, please check.")

    if ret == 0:
        filename = 'sent_mail'
        if not os.path.isfile(filename):
            #automaticReporting.sendMail()
            open(filename, 'w').close()

    B2INFO("Created analysis summary pdf file.")
    return {'FEIsummary.pdf': None, '__needed__': False}


def SaveModuleStatistics(path, hash, finalParticleSignalProbabilities):
    """
    Creates .root file that contains statistics for all modules running in final execution.
        @param path the basf2 path
        @param hash of all input parameters
        @param finalParticleSignalProbabilities used to start execution after all final particles are completed
        @return root file name
    """

    B2INFO("Enter: SaveModuleStatistics")
    filename = 'moduleStatistics_' + hash + '.root'
    if not os.path.isfile(filename):
        output = register_module('RootOutput')
        output.param('outputFileName', filename)
        output.param('branchNames', ['EventMetaData'])  # cannot be removed, but of only small effect on file size
        output.param('branchNamesPersistent', ['ProcessStatistics'])
        output.param('ignoreCommandLineOverride', True)
        path.add_module(output)
        B2INFO("SaveModuleStatistics: Added RootOutput")
        return {}

    B2INFO("Provided SaveModuleStatistics")
    return {'ModuleStatisticsFile': filename, '__cache__': True}


def WriteCPUTimeSummary(channelNames, inputLists, channelPlaceholders, mcCounts, moduleStatisticsFile):
    """
    Creates CPU time summary
        @param mcCounts
        @param moduleStatisticsFile file name of the TFile containing actual statistics
        @return dictionary of placeholders used in the .tex file
    """
    stats = automaticReporting.getModuleStatsFromFile(moduleStatisticsFile)
    placeholders = automaticReporting.createCPUTimeTexFile(channelNames, inputLists, channelPlaceholders, mcCounts, moduleStatisticsFile, stats)

    return {'CPUTimeSummary': placeholders}
