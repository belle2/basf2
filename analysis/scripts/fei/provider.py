#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# Thomas Keck 2015
#

from basf2 import *
from modularAnalysis import *

import ROOT
ROOT.gSystem.Load('libanalysis.so')
from ROOT import Belle2

import pdg
import pickle

import re
import os
import subprocess

import typing
import fei.dag
import fei.config

import basf2_mva

Hash = str
Filename = str
ParticleList = str

MaximumNumberOfMVASamples = int(1e7)
MinimumNumberOfMVASamples = int(5e2)


def removeJPsiSlash(string: str) -> str:
    return string.replace('/', '')


def HashRequirements(resource: fei.dag.Resource) -> Hash:
    """
    Returns the hash of all requirments
        @param resource object
    """
    resource.cache = True
    return resource.hash


def PDGConjugate(resource: fei.dag.Resource, particleList: ParticleList) -> ParticleList:
    """
    Returns the pdg conjugated list
        @param resource object
        @param particleList ParticleList
    """
    resource.cache = True
    if particleList is None:
        return
    name, label = particleList.split(':')
    return pdg.conjugate(name) + ':' + label


def LoadParticles(resource: fei.dag.Resource, names: typing.Sequence[str]) -> Hash:
    """
    Load FSP and V0 Particles
    Restricts loading to Particles contained in current Rest Of Event if specific FEI mode is used.
        @param resource object
    """
    resource.cache = True
    cut = 'isInRestOfEvent > 0.5' if resource.env['ROE'] else ''
    fillParticleLists([('K+:FSP', cut), ('pi+:FSP', cut), ('e+:FSP', cut),
                       ('mu+:FSP', cut), ('gamma:FSP', cut), ('K_S0:V0', cut),
                       ('p+:FSP', cut), ('K_L0:FSP', cut), ('Lambda0:FSP', cut)], writeOut=True, path=resource.path)
    fillConvertedPhotonsList('gamma:V0', cut, writeOut=True, path=resource.path)

    if resource.env['monitor']:
        hist_filename = 'Monitor_MCCounts.root'
        unique_abs_pdgs = set([abs(pdg.from_name(name)) for name in names])
        hist_variables = [('NumberOfMCParticlesInEvent({i})'.format(i=pdgcode), 100, -0.5, 99.5) for pdgcode in unique_abs_pdgs]
        variablesToHistogram('', variables=hist_variables,
                             filename=removeJPsiSlash(hist_filename), path=resource.path)
    return resource.hash


def LoadParticlesB2BII(resource: fei.dag.Resource, names: typing.Sequence[str]) -> Hash:
    """
    Load FSP and V0 Particles from B2BII Particle lists
    Restricts loading to Particles contained in current Rest Of Event if specific FEI mode is used.
        @param resource object
    """
    resource.cache = True
    cut = 'isInRestOfEvent > 0.5' if resource.env['ROE'] else ''

    fillParticleLists([('K+:FSP', cut), ('pi+:FSP', cut), ('e+:FSP', cut),
                       ('mu+:FSP', cut), ('p+:FSP', cut), ('K_L0:FSP', cut)], writeOut=True, path=resource.path)

    for outputList, inputList in [('gamma:FSP', 'gamma:mdst'), ('K_S0:V0', 'K_S0:mdst'),
                                  ('pi0:FSP', 'pi0:mdst'), ('gamma:V0', 'gamma:v0mdst')]:
        copyParticles(outputList, inputList, writeOut=True, path=resource.path)
        applyCuts(outputList, cut, path=resource.path)

    if resource.env['monitor']:
        hist_filename = 'Monitor_MCCounts.root'
        unique_abs_pdgs = set([abs(pdg.from_name(name)) for name in names])
        hist_variables = [('NumberOfMCParticlesInEvent({i})'.format(i=pdgcode), 100, -0.5, 99.5) for pdgcode in unique_abs_pdgs]
        variablesToHistogram('', variables=hist_variables,
                             filename=removeJPsiSlash(hist_filename), path=resource.path)
    return resource.hash


def MakeParticleList(resource: fei.dag.Resource, particleName: str, daughterParticleLists: typing.Sequence[ParticleList],
                     preCutConfig: fei.config.PreCutConfiguration,
                     mvaConfig: fei.config.MVAConfiguration, decayModeID: int) -> ParticleList:
    """
    Creates a ParticleList by combining other ParticleLists via the ParticleCombiner module or
    if only one daughter particle is given all FSP particles with the given corresponding name are gathered up.
        @param resource object
        @param particleName valid pdg particle name
        @param daughterParticleLists list of ParticleList name of every daughter particles
        @param preCutConfig PreCutConfiguration object for this channel
        @param decayModeID integer ID of this decay channel, added to extra-info of Particles
        @return name of new ParticleList
    """
    resource.cache = True
    if any(p is None for p in daughterParticleLists):
        return

    particleList = particleName + ':' + resource.hash
    if len(daughterParticleLists) == 1:
        cutAndCopyList(particleList, daughterParticleLists[0], preCutConfig.userCut, writeOut=True, path=resource.path)
        variablesToExtraInfo(particleList, {'constant({dmID})'.format(dmID=decayModeID): 'decayModeID'}, path=resource.path)
    else:
        decayString = particleList + ' ==> ' + ' '.join(daughterParticleLists)
        reconstructDecay(decayString, preCutConfig.userCut, decayModeID, writeOut=True, path=resource.path)

    if resource.env['monitor']:
        matchMCTruth(particleList, path=resource.path)
        if preCutConfig.bestCandidateVariable is None:
            hist_variables = ['mcErrors', 'mcParticleStatus', mvaConfig.target]
            hist_variables_2d = []
        else:
            hist_variables = ['mcErrors', 'mcParticleStatus', mvaConfig.target, preCutConfig.bestCandidateVariable]
            hist_variables_2d = [(preCutConfig.bestCandidateVariable, mvaConfig.target),
                                 (preCutConfig.bestCandidateVariable, 'mcErrors'),
                                 (preCutConfig.bestCandidateVariable, 'mcParticleStatus')]
        hist_filename = 'Monitor_MakeParticleList_BeforeRanking_' + particleList + '.root'
        variablesToHistogram(particleList,
                             variables=fei.config.variables2binnings(hist_variables),
                             variables_2d=fei.config.variables2binnings_2d(hist_variables_2d),
                             filename=removeJPsiSlash(hist_filename), path=resource.path)

    if preCutConfig.bestCandidateVariable is not None:
        if preCutConfig.bestCandidateMode == 'lowest':
            rankByLowest(particleList, preCutConfig.bestCandidateVariable, preCutConfig.bestCandidateCut,
                         'preCut_rank', path=resource.path)
        elif preCutConfig.bestCandidateMode == 'highest':
            rankByHighest(particleList, preCutConfig.bestCandidateVariable, preCutConfig.bestCandidateCut,
                          'preCut_rank', path=resource.path)
        else:
            raise RuntimeError("Unkown bestCandidateMode " + repr(preCutConfig.bestCandidateMode))

    if resource.env['monitor']:
        hist_filename = 'Monitor_MakeParticleList_AfterRanking_' + particleList + '.root'
        hist_variables += ['extraInfo(preCut_rank)']
        hist_variables_2d += [('extraInfo(preCut_rank)', mvaConfig.target),
                              ('extraInfo(preCut_rank)', 'mcErrors'),
                              ('extraInfo(preCut_rank)', 'mcParticleStatus')]
        variablesToHistogram(particleList,
                             variables=fei.config.variables2binnings(hist_variables),
                             variables_2d=fei.config.variables2binnings_2d(hist_variables_2d),
                             filename=removeJPsiSlash(hist_filename), path=resource.path)

    return particleList


def MatchParticleList(resource: fei.dag.Resource, particleList: ParticleList,
                      mvaConfig: fei.config.MVAConfiguration) -> ParticleList:
    """
    Match MC truth of the given ParticleList
        @param resource object
        @param particleList raw ParticleList
        @param mvaConfig MVA configuration object
        @return name of matched ParticleList
    """
    resource.cache = True
    if particleList is None:
        return
    resource.condition = ('EventType', '==0')
    matchMCTruth(particleList, path=resource.path)

    if resource.env['monitor']:
        hist_variables = ['mcErrors', 'mcParticleStatus', mvaConfig.target]
        hist_variables_2d = []
        hist_filename = 'Monitor_MatchParticleList_AfterMatch_' + particleList + '.root'
        variablesToHistogram(particleList,
                             variables=fei.config.variables2binnings(hist_variables),
                             variables_2d=fei.config.variables2binnings_2d(hist_variables_2d),
                             filename=removeJPsiSlash(hist_filename), path=resource.path)
    # If the MatchedParticleList is different from the RawParticleList the CPU Statistics will work not correctly,
    # because the name of the MatchedParticleList is used to identify all modules which process this channel.
    # You have to fix this issue before changing the returned value in this function.
    return particleList


def CopyParticleLists(resource: fei.dag.Resource, particleName: str, particleLabel: str,
                      particleLists: typing.Sequence[ParticleList],
                      postCutConfig: fei.config.PostCutConfiguration,
                      mvaConfig: fei.config.MVAConfiguration,
                      signalProbabilities: typing.Sequence[str]) -> ParticleList:
    """
    Creates a ParticleList gathering up all particles in the given inputLists
        @param resource object
        @param particleName valid pdg particle name
        @param particleLabel user defined label
        @param particleLists list of ParticleLists name defning which ParticleLists are copied to the new list
        @param postCutConfig postCutConfig
        @param signalProbabilities signal probability of the particle lists
        @return name of new ParticleList
    """
    resource.cache = True
    particleLists = [p for p, s in zip(particleLists, signalProbabilities) if p is not None and s is not None]
    if particleLists == []:
        return

    cutstring = ''
    if postCutConfig.value > 0.0:
        cutstring = str(postCutConfig.value) + ' < extraInfo(SignalProbability)'

    particleList = particleName + ':' + resource.hash
    copyLists(particleList, particleLists, writeOut=True, path=resource.path)

    if resource.env['monitor']:
        hist_variables = ['mcErrors', 'mcParticleStatus', mvaConfig.target, 'extraInfo(SignalProbability)',
                          'extraInfo(decayModeID)']
        hist_variables_2d = [('extraInfo(decayModeID)', mvaConfig.target),
                             ('extraInfo(decayModeID)', 'mcErrors'),
                             ('extraInfo(decayModeID)', 'mcParticleStatus')]
        hist_filename = 'Monitor_CopyParticleList_BeforeCut_' + particleList + '.root'
        variablesToHistogram(particleList,
                             variables=fei.config.variables2binnings(hist_variables),
                             variables_2d=fei.config.variables2binnings_2d(hist_variables_2d),
                             filename=removeJPsiSlash(hist_filename), path=resource.path)

    applyCuts(particleList, cutstring, path=resource.path)

    if resource.env['monitor']:
        hist_filename = 'Monitor_CopyParticleList_BeforeRanking_' + particleList + '.root'
        variablesToHistogram(particleList,
                             variables=fei.config.variables2binnings(hist_variables),
                             variables_2d=fei.config.variables2binnings_2d(hist_variables_2d),
                             filename=removeJPsiSlash(hist_filename), path=resource.path)

    rankByHighest(particleList, 'extraInfo(SignalProbability)', postCutConfig.bestCandidateCut, 'postCut_rank', path=resource.path)

    if resource.env['monitor']:
        hist_filename = 'Monitor_CopyParticleList_AfterRanking_' + particleList + '.root'
        hist_variables += ['extraInfo(postCut_rank)']
        hist_variables_2d += [('extraInfo(decayModeID)', 'extraInfo(postCut_rank)'),
                              (mvaConfig.target, 'extraInfo(postCut_rank)'),
                              ('mcErrors', 'extraInfo(postCut_rank)'),
                              ('mcParticleStatus', 'extraInfo(postCut_rank)')]
        variablesToHistogram(particleList,
                             variables=fei.config.variables2binnings(hist_variables),
                             variables_2d=fei.config.variables2binnings_2d(hist_variables_2d),
                             filename=removeJPsiSlash(hist_filename), path=resource.path)

    return particleList


def CopyIntoHumanReadableParticleList(resource: fei.dag.Resource, particleName: str, particleLabel: str,
                                      mvaConfig: fei.config.MVAConfiguration,
                                      particleList: ParticleList) -> ParticleList:
    """
    Copys a ParticleList into a new ParticleList with a human readable name
        @param resource object
        @param particleName valid pdg particle name
        @param particleLabel label of the new ParticleList
        @param particleList ParticleList which is copied
        @return name of new ParticleList
    """
    resource.cache = True
    if particleList is None:
        return
    humanReadableParticleList = particleName + ':' + particleLabel
    copyLists(humanReadableParticleList, particleList, writeOut=True, path=resource.path)

    if resource.env['monitor']:
        variables = ['extraInfo(SignalProbability)', 'Mbc', 'mcErrors', 'mcParticleStatus', mvaConfig.target,
                     'cosThetaBetweenParticleAndTrueB', 'extraInfo(uniqueSignal)', 'extraInfo(decayModeID)']
        filename = 'Monitor_Final_' + humanReadableParticleList + '.root'
        variablesToNTuple(humanReadableParticleList, variables, treename='variables',
                          filename=removeJPsiSlash(filename), path=resource.path)

    return humanReadableParticleList


def FitVertex(resource: fei.dag.Resource, channelName: str, particleList: ParticleList,
              mvaConfig: fei.config.MVAConfiguration,
              preCutConfig: fei.config.PreCutConfiguration) -> Hash:
    """
    Fit secondary vertex of all particles in this ParticleList
        @param resource object
        @param channelName unique name describing the channel
        @param particleList ParticleList name
        @param preCutConfig preCutConfig
        @return hash
    """
    resource.cache = True
    if particleList is None:
        return

    if re.findall(r"[\w']+", channelName).count('pi0') > 1:
        B2INFO("Ignoring vertex fit for this channel because multiple pi0 are not supported yet {c}.".format(c=channelName))
        return

    if resource.env['monitor']:
        hist_variables = ['mcErrors', 'mcParticleStatus', mvaConfig.target]
        hist_filename = 'Monitor_FitVertex_Before_' + particleList + '.root'
        variablesToHistogram(particleList, variables=fei.config.variables2binnings(hist_variables),
                             variables_2d=fei.config.variables2binnings_2d([]),
                             filename=removeJPsiSlash(hist_filename), path=resource.path)

    pvfit = register_module('ParticleVertexFitter')
    pvfit.set_name('ParticleVertexFitter_' + particleList)
    pvfit.param('listName', particleList)
    pvfit.param('confidenceLevel', preCutConfig.vertexCut)
    pvfit.param('vertexFitter', 'kfitter')
    pvfit.param('fitType', 'vertex')
    pvfit.set_log_level(logging.log_level.ERROR)  # let's not produce gigabytes of uninteresting warnings
    resource.path.add_module(pvfit)

    if resource.env['monitor']:
        hist_variables = ['mcErrors', 'chiProb', 'mcParticleStatus', mvaConfig.target]
        hist_variables_2d = [('chiProb', mvaConfig.target),
                             ('chiProb', 'mcErrors'),
                             ('chiProb', 'mcParticleStatus')]
        hist_filename = 'Monitor_FitVertex_After_' + particleList + '.root'
        variablesToHistogram(particleList, variables=fei.config.variables2binnings(hist_variables),
                             variables_2d=fei.config.variables2binnings_2d(hist_variables_2d),
                             filename=removeJPsiSlash(hist_filename), path=resource.path)

    return resource.hash


def GenerateTrainingData(resource: fei.dag.Resource, particleName: str, particleList: ParticleList,
                         mcCounts: typing.Mapping[int, typing.Mapping[str, float]],
                         preCutConfig: fei.config.PreCutConfiguration,
                         mvaConfig: fei.config.MVAConfiguration) -> Filename:
    """
    Generates the training data for the training of an MVC
        @param resource object
        @param particleList the particleList which is used for training and classification
        @param mvaConfig configuration for the multivariate analysis
        @param inverseSamplingRates dictionary of inverseSampling Rates for signal (1) and background (0)
        @return string ROOT filename
    """
    global MaximumNumberOfMVASamples

    resource.cache = True
    if particleList is None:
        return

    if preCutConfig.bestCandidateVariable is None:
        B2WARNING("Best Candidate variable for particle {} not set.".format(particleName) +
                  "cannot calculate inverse sampling rates correctly")

    pdgcode = abs(pdg.from_name(particleName))
    nSignal = mcCounts[pdgcode]['sum']
    # HACK For everything above D-Mesons we usually have a branching fraction of 10**(-3)
    if pdgcode > 400:
        nSignal /= 300
    nBackground = mcCounts[0]['sum'] * preCutConfig.bestCandidateCut

    inverseSamplingRates = {}
    if nBackground > MaximumNumberOfMVASamples:
        inverseSamplingRates[0] = int(nBackground / MaximumNumberOfMVASamples) + 1
    if nSignal > MaximumNumberOfMVASamples:
        inverseSamplingRates[1] = int(nSignal / MaximumNumberOfMVASamples) + 1

    rootFilename = removeJPsiSlash('{particleList}_{hash}.root'.format(particleList=particleList, hash=resource.hash))

    if not os.path.isfile(rootFilename):
        spectators = [mvaConfig.target]
        if mvaConfig.sPlotVariable is not None:
            spectators.append(mvaConfig.sPlotVariable)

        if resource.env['monitor']:
            hist_variables = ['mcErrors', 'mcParticleStatus'] + mvaConfig.variables + spectators
            hist_variables_2d = [(x, mvaConfig.target) for x in mvaConfig.variables + spectators if x is not mvaConfig.target]
            hist_filename = 'Monitor_GenerateTrainingData_' + particleList + '.root'
            variablesToHistogram(particleList,
                                 variables=fei.config.variables2binnings(hist_variables),
                                 variables_2d=fei.config.variables2binnings_2d(hist_variables_2d),
                                 filename=removeJPsiSlash(hist_filename), path=resource.path)

        teacher = register_module('VariablesToNtuple')
        teacher.set_name('VariablesToNtuple_' + particleList)
        teacher.param('fileName', removeJPsiSlash(particleList + '_' + resource.hash) + '.root')
        teacher.param('treeName', 'variables')
        teacher.param('variables', mvaConfig.variables + spectators)
        teacher.param('particleList', particleList)
        teacher.param('sampling', (mvaConfig.target, inverseSamplingRates))
        resource.path.add_module(teacher)
        if mvaConfig.sPlotVariable is None:
            resource.condition = ('EventType', '==0')
        resource.halt = True
        return
    return rootFilename[:-5]


def TrainMultivariateClassifier(resource: fei.dag.Resource, databasePrefix: str, mvaConfig: fei.config.MVAConfiguration,
                                trainingData: Filename) -> Filename:
    """
    Train multivariate classifier using the trainingData
        @param resource object
        @param databasePrefix used to store the generated weightfile
        @param mvaConfig configuration for the multivariate analysis
        @param trainingData name of ROOT-File which contains training data
        @return string config filename
    """
    global MinimumNumberOfMVASamples

    resource.cache = True
    if trainingData is None:
        return

    configFilename = trainingData + '.xml'

    if not os.path.isfile(configFilename):
        f = ROOT.TFile(trainingData + '.root')
        if not f:
            B2WARNING("Training of MVC failed. Couldn't find ROOT file. Ignoring channel.")
            return
        l = [m for m in f.GetListOfKeys()]
        if not l:
            B2WARNING("Training of MVC failed. ROOT file does not contain a tree. Ignoring channel.")
            return
        tree = l[0].ReadObj()
        nSig = tree.GetEntries(mvaConfig.target + ' == 1.0')
        nBg = tree.GetEntries(mvaConfig.target + ' != 1.0')
        if nSig < MinimumNumberOfMVASamples:
            B2WARNING("Training of MVC failed. Tree contains to few signal events {}. Ignoring channel.".format(nSig))
            return
        if nBg < MinimumNumberOfMVASamples:
            B2WARNING("Training of MVC failed. Tree contains to few bckgrd events {}. Ignoring channel.".format(nBg))
            return

        command = (
            "{externTeacher} --method '{method}' --target_variable '{target}' --treename variables --datafile '{prefix}.root' "
            "--signal_class 1 --variables '{variables}' --identifier '{prefix}.xml' {config} > '{prefix}'.log 2>&1"
            " && basf2_mva_upload --identifier '{prefix}.xml' --db_identifier '{databasePrefix}_{prefix}'".format(
                databasePrefix=databasePrefix,
                externTeacher=resource.env['externTeacher'],
                method=mvaConfig.method,
                config=mvaConfig.config,
                target=mvaConfig.target,
                variables="' '".join(mvaConfig.variables),
                prefix=trainingData))
        B2INFO("Used following command to invoke teacher\n" + command)
        # The training of the MVC can run in parallel!
        # FIXME Bug? Because subprocess is not thread-safe, did not cause any problems so far.
        with resource.EnableMultiThreading():
            subprocess.call(command, shell=True)
        # TODO Upload a second time using same basf2 process which also adds he mva expert,
        # this can make a difference because the localdb is relative to the working directory
        # during the start of basf2
        basf2_mva.upload(trainingData + '.xml', databasePrefix + '_' + trainingData)

    if not os.path.isfile(configFilename):
        B2WARNING("Training of MVC failed. Ignoring channel.")
        return

    return trainingData


def SignalProbability(resource: fei.dag.Resource, databasePrefix: str, particleList: ParticleList,
                      mvaConfig: fei.config.MVAConfiguration, tmvaPrefix: str) -> Hash:
    """
    Calculates the SignalProbability of a ParticleList using the previously trained MVC
        @param resource object
        @param databasePrefix used to store the generated weightfile
        @param particleList the particleList which is used for training and classification
        @param mvaConfig configuration for the multivariate analysis
        @param tmvaPrefix used to train teh TMVA classifier
        @return string config filename
    """
    resource.cache = True
    if tmvaPrefix is None or particleList is None:
        return
    expert = register_module('MVAExpert')
    expert.set_name('MVAExpert_' + particleList)
    expert.param('identifier', databasePrefix + '_' + tmvaPrefix)
    expert.param('extraInfoName', 'SignalProbability')
    expert.param('listNames', [particleList])
    resource.path.add_module(expert)

    if resource.env['monitor']:
        hist_variables = ['mcErrors', 'mcParticleStatus', 'extraInfo(SignalProbability)', mvaConfig.target]
        hist_variables_2d = [('extraInfo(SignalProbability)', mvaConfig.target),
                             ('extraInfo(SignalProbability)', 'mcErrors'),
                             ('extraInfo(SignalProbability)', 'mcParticleStatus')]
        hist_filename = 'Monitor_SignalProbability_' + particleList + '.root'
        variablesToHistogram(particleList, variables=fei.config.variables2binnings(hist_variables),
                             variables_2d=fei.config.variables2binnings_2d(hist_variables_2d),
                             filename=removeJPsiSlash(hist_filename), path=resource.path)

    return resource.hash


def TagUniqueSignal(resource: fei.dag.Resource, particleList: ParticleList, signalProbability: Hash,
                    mvaConfig: fei.config.MVAConfiguration) -> Hash:
    """
    Saves the calculated signal probability for this particle list
        @param resource object
        @param particleList the particleList
        @param signalProbability signalProbability as additional dependency
        @param mvaConfig multivariate analysis configuration object
        @return string variable name that provides tag
    """
    resource.cache = True
    if particleList is None or signalProbability is None:
        return

    uniqueSignal = register_module('TagUniqueSignal')
    uniqueSignal.param('particleList', particleList)
    uniqueSignal.param('target', mvaConfig.target)
    uniqueSignal.param('extraInfoName', 'uniqueSignal')
    uniqueSignal.set_name('TagUniqueSignal_' + particleList)
    resource.path.add_module(uniqueSignal)

    if resource.env['monitor']:
        hist_variables = ['mcErrors', 'mcParticleStatus', 'extraInfo(uniqueSignal)', mvaConfig.target,
                          'extraInfo(decayModeID)']
        hist_variables_2d = [('extraInfo(decayModeID)', mvaConfig.target),
                             ('extraInfo(decayModeID)', 'mcErrors'),
                             ('extraInfo(decayModeID)', 'extraInfo(uniqueSignal)'),
                             ('extraInfo(decayModeID)', 'mcParticleStatus')]
        hist_filename = 'Monitor_TagUniqueSignal_' + particleList + '.root'
        variablesToHistogram(particleList,
                             variables=fei.config.variables2binnings(hist_variables),
                             variables_2d=fei.config.variables2binnings_2d(hist_variables_2d),
                             filename=removeJPsiSlash(hist_filename), path=resource.path)

    resource.condition = ('EventType', '==0')
    return resource.hash


def SaveModuleStatistics(resource: fei.dag.Resource) -> typing.Mapping[str, float]:
    """
    Creates .root file that contains statistics for all modules running in final execution.
    And converts it into a dictionary of ModuleName: UsedCpuTime
        @param resource object
        @return cpu time statistic
    """
    resource.cache = True

    if resource.env['monitor']:
        output = register_module('RootOutput')
        output.param('outputFileName', 'Monitor_ModuleStatistics.root')
        output.param('branchNames', ['EventMetaData'])  # cannot be removed, but of only small effect on file size
        output.param('branchNamesPersistent', ['ProcessStatistics'])
        output.param('ignoreCommandLineOverride', True)
        resource.path.add_module(output)

    filename = 'moduleStatistics_' + resource.hash + '.root'
    if not os.path.isfile(filename):
        output = register_module('RootOutput')
        output.param('outputFileName', filename)
        output.param('branchNames', ['EventMetaData'])  # cannot be removed, but of only small effect on file size
        output.param('branchNamesPersistent', ['ProcessStatistics'])
        output.param('ignoreCommandLineOverride', True)
        resource.path.add_module(output)
        resource.halt = True
        return

    root_file = ROOT.TFile(filename)
    persistentTree = root_file.Get('persistent')
    persistentTree.GetEntry(0)
    # Clone() needed so we actually own the object (original dies when tfile is deleted)
    stats = persistentTree.ProcessStatistics.Clone()

    # merge statistics from all persistent trees into 'stats'
    numEntries = persistentTree.GetEntriesFast()
    for i in range(1, numEntries):
        persistentTree.GetEntry(i)
        stats.merge(persistentTree.ProcessStatistics)

    # TODO .getTimeSum returns always 0 at the moment ?!
    statistic = {m.getName(): m.getTimeSum(m.c_Event) / 1e9 for m in stats.getAll()}
    return statistic


def CountMCParticles(resource: fei.dag.Resource, names: typing.Sequence[str]) -> typing.Mapping[int, typing.Mapping[str, float]]:
    """
    Counts the number of MC Particles for every pdg code in all events
        @param resource object
        @param names of all particles
    """
    resource.cache = True
    filename = 'mcParticlesCount.root'

    if not os.path.isfile(filename):
        output = register_module('VariablesToHistogram')
        output.set_name("VariablesToHistogram_MCCount")
        unique_abs_pdgs = set([abs(pdg.from_name(name)) for name in names])
        output.param('variables', [('NumberOfMCParticlesInEvent({i})'.format(i=pdgcode), 100, -0.5, 99.5)
                                   for pdgcode in unique_abs_pdgs])
        output.param('fileName', filename)
        resource.path.add_module(output)
        resource.condition = ('EventType', '==0')
        resource.halt = True
        return

    root_file = ROOT.TFile(filename)
    mc_counts = {}

    Belle2.Variable.Manager

    for key in root_file.GetListOfKeys():
        variable = Belle2.invertMakeROOTCompatible(key.GetName())
        pdgcode = abs(int(variable[len('NumberOfMCParticlesInEvent('):-len(")")]))
        hist = key.ReadObj()
        mc_counts[pdgcode] = {}
        mc_counts[pdgcode]['sum'] = sum(hist.GetXaxis().GetBinCenter(bin + 1) * hist.GetBinContent(bin + 1)
                                        for bin in range(hist.GetNbinsX()))
        mc_counts[pdgcode]['std'] = hist.GetStdDev()
        mc_counts[pdgcode]['avg'] = hist.GetMean()
        mc_counts[pdgcode]['max'] = hist.GetXaxis().GetBinCenter(hist.FindLastBinAbove(0.0))
        mc_counts[pdgcode]['min'] = hist.GetXaxis().GetBinCenter(hist.FindFirstBinAbove(0.0))

    mc_counts[0] = {}
    mc_counts[0]['sum'] = hist.GetEntries()
    return mc_counts


def SaveSummary(resource: fei.dag.Resource, mcCounts: typing.Mapping[int, typing.Mapping[str, float]],
                moduleStatistics: typing.Mapping[str, float],
                particles: typing.Sequence[fei.config.Particle],
                plists: typing.Sequence[ParticleList],
                clists: typing.Sequence[ParticleList],
                mlists: typing.Sequence[ParticleList],
                cnames: typing.Sequence[str],
                trainingData: typing.Sequence[Filename]) -> Filename:
    """
    Save all important files, determined cuts and the all configurations objects in a pickled file,
    so we can use it later to produce a compact or a detailed training report
        @param resource object
        @param mcCounts filename containing mcCounts
        @param particles all particle objects given by the user
        @param plists particle list names
        @param clists raw channel list names
        @param mlists matched channel list names
        @param cnames channel names of each channel
        @param trainingData filename of TMVA training data input for each channel
    """

    resource.cache = True
    filename = 'Summary_' + resource.hash + '.pickle'

    obj = {'mc_counts': mcCounts,
           'module_statistics': moduleStatistics,
           'particles': particles,
           'particle2list': {n: l for n, l in zip([p.identifier for p in particles], plists)},
           'channel2lists': {n: (c, m, t) for n, c, m, t in zip(cnames, clists, mlists, trainingData)}}

    with open(filename, 'wb') as out:
        pickle.dump(obj, out)

    return filename
