#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# Thomas Keck 2015
#

from basf2 import *
import ROOT

import modularAnalysis
import pdg

from fei import preCutDetermination
import pickle


def removeJPsiSlash(string):
    return string.replace('/', '')


def joinCuts(*cuts):
    """
    Join given cut string together using correct cut-syntax with and and brackets
    @param cuts cut string
    """
    notempty = sum(cut != '' for cut in cuts)
    if notempty == 0:
        return ''
    if notempty == 1:
        for cut in cuts:
            if cut != '':
                return cut
    return '[' + '] and ['.join(cut for cut in cuts if cut != '') + ']'


import re
import os
import subprocess
import json
from string import Template


def HashRequirements(resource):
    """
    Returns the hash of all requirments
        @param resource object
    """
    resource.cache = True
    return resource.hash


def PDGConjugate(resource, particleList):
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


def LoadGearbox(resource):
    """
    Loads Gearbox module
        @param resource object
    """
    gearbox = register_module('Gearbox')
    resource.path.add_module(gearbox)


def LoadGeometry(resource):
    """
    Loads Geometry module
        @param resource object
    """
    resource.path.add_module('Geometry', ignoreIfPresent=True, components=['MagneticField'])


def LoadParticles(resource):
    """
    Load Particle module
        @param resource object
    """
    resource.cache = True
    modularAnalysis.fillParticleLists([('K+:FSP', ''), ('pi+:FSP', ''), ('e+:FSP', ''),
                                       ('mu+:FSP', ''), ('gamma:FSP', ''), ('K_S0:FSP', ''),
                                       ('p+:FSP', ''), ('K_L0:FSP', ''), ('Lambda0:FSP', '')], writeOut=True, path=resource.path)
    return 'Dummy'


def MatchParticleList(resource, particleList, mvaTarget):
    """
    Match MC truth of the given ParticleList
        @param resource object
        @param particleList raw ParticleList
        @return name of matched ParticleList
        @param mvaTarget which distinguishs between signal and background
    """
    resource.cache = True
    if particleList is None:
        return
    resource.condition = ('EventType', '==0')
    modularAnalysis.matchMCTruth(particleList, path=resource.path)

    cut = ''
    if resource.env['ROE']:
        sigmc = 'eventCached(countInList(' + resource.env['ROE'] + ', isSignalAcceptMissingNeutrino == 1))'
        cut = '[{sigmc} > 0 and {target} == 1] or {sigmc} == 0'.format(sigmc=sigmc, target=mvaTarget)
    modularAnalysis.applyCuts(particleList, cut, path=resource.path)
    # If the MatchedParticleList is different from the RawParticleList the CPU Statistics will work not correctly,
    # because the name of the MatchedParticleList is used to identify all modules which process this channel.
    # You have to fix this issue before changing the returned value in this function.
    return particleList


def MakeParticleList(resource, particleName, daughterParticleLists, preCut, userCut, decayModeID):
    """
    Creates a ParticleList by combining other ParticleLists via the ParticleCombiner module or
    if only one daughter particle is given all FSP particles with the given corresponding name are gathered up.
        @param resource object
        @param particleName valid pdg particle name
        @param daughterParticleLists list of ParticleList name of every daughter particles
        @param preCut dictionary containing 'cutstring', a string which defines the cut which is applied
               before the combination of the daughter particles.
        @param userCut user-defined cut
        @param decayModeID integer ID of this decay channel, added to extra-info of Particles
        @return name of new ParticleList
    """
    resource.cache = True
    if preCut is None:
        return

    particleList = particleName + ':' + resource.hash
    if len(daughterParticleLists) == 1:
        cut = joinCuts(userCut, 'isInRestOfEvent > 0.5' if resource.env['ROE'] else '', preCut['cutstring'])
        modularAnalysis.cutAndCopyList(particleList, daughterParticleLists[0], cut, writeOut=True, path=resource.path)
        modularAnalysis.variablesToExtraInfo(particleList, {'constant({dmID})'.format(dmID=decayModeID): 'decayModeID'},
                                             path=resource.path)
    else:
        decayString = particleList + ' ==> ' + ' '.join(daughterParticleLists)
        pmake = register_module('ParticleCombiner')
        pmake.set_name('ParticleCombiner_' + decayString)
        pmake.param('decayString', decayString)
        pmake.param('cut', joinCuts(userCut, preCut['cutstring']))
        pmake.param('maximumNumberOfCandidates', 1000)
        pmake.param('decayMode', decayModeID)
        pmake.param('writeOut', True)
        resource.path.add_module(pmake)

    return particleList


def CopyParticleLists(resource, particleName, particleLabel, particleLists, postCut, signalProbabilities):
    """
    Creates a ParticleList gathering up all particles in the given inputLists
        @param resource object
        @param particleName valid pdg particle name
        @param particleLabel user defined label
        @param particleLists list of ParticleLists name defning which ParticleLists are copied to the new list
        @param postCut dictionary containing 'cutstring'
        @param signalProbabilities signal probability of the particle lists
        @return name of new ParticleList
    """
    resource.cache = True
    particleLists = [p for p, s in zip(particleLists, signalProbabilities) if p is not None and s is not None]
    if particleLists == []:
        return

    particleList = particleName + ':' + resource.hash
    modularAnalysis.cutAndCopyLists(
        particleList,
        particleLists,
        postCut['cutstring'],
        writeOut=True,
        path=resource.path)
    return particleList


def CopyIntoHumanReadableParticleList(resource, particleName, particleLabel, particleList):
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
    modularAnalysis.cutAndCopyLists(humanReadableParticleList, particleList, '', writeOut=True, path=resource.path)
    return humanReadableParticleList


def FitVertex(resource, channelName, particleList, vertexCut):
    """
    Fit secondary vertex of all particles in this ParticleList
        @param resource object
        @param channelName unique name describing the channel
        @param particleList ParticleList name
        @param vertexCut user-defined vertex cut
        @return hash
    """
    resource.cache = True
    if particleList is None:
        return

    if re.findall(r"[\w']+", channelName).count('pi0') > 1:
        B2INFO("Ignoring vertex fit for this channel because multiple pi0 are not supported yet {c}.".format(c=channelName))
        return

    pvfit = register_module('ParticleVertexFitter')
    pvfit.set_name('ParticleVertexFitter_' + particleList)
    pvfit.param('listName', particleList)
    pvfit.param('confidenceLevel', vertexCut)
    pvfit.param('vertexFitter', 'kfitter')
    # pvfit.param('vertexFitter', 'rave')
    pvfit.param('fitType', 'vertex')
    pvfit.set_log_level(logging.log_level.ERROR)  # let's not produce gigabytes of uninteresting warnings
    resource.path.add_module(pvfit)

    return resource.hash


def CreatePreCutHistogram(resource, particleName, channelName, mvaTarget, preCutConfig, userCut, daughterParticleLists,
                          additionalDependencies):
    """
    Creates ROOT file with chosen pre cut variable histogram of this channel (signal/background)
    for a given particle, before any intermediate cuts are applied.
        @param resource object
        @param particleName valid pdg particle name
        @param channelName unique name describing the channel
        @param mvaTarget variable which defines signal and background
        @param preCutConfig intermediate cut configuration
        @param userCut user-defined cut
        @param daughterParticleLists list of ParticleList names defining all daughter particles
        @param additionalDependencies Additional dependencies on signal probability if necessary
        @return ROOT filename 'CutHistograms_{channelName}:{hash}.root' and tree key {particleName}:{hash}
    """
    resource.cache = True
    if any([daughterParticleList is None for daughterParticleList in daughterParticleLists]) or\
            any([x is None for x in additionalDependencies]):
        return

    filename = removeJPsiSlash('CutHistograms_{c}:{h}.root'.format(c=channelName, h=resource.hash))

    if os.path.isfile(filename):
        return (filename, particleName + ':' + resource.hash)

    outputList = particleName + ':' + resource.hash + ' ==> ' + ' '.join(daughterParticleLists)
    pmake = register_module('PreCutHistMaker')
    pmake.set_name('PreCutHistMaker_{c}'.format(c=channelName))
    pmake.param('fileName', filename)
    pmake.param('decayString', outputList)
    pmake.param('cut', userCut)
    pmake.param('maximumNumberOfCandidates', 1000)
    pmake.param('target', mvaTarget)
    pmake.param('variable', preCutConfig.variable)
    if isinstance(preCutConfig.binning, tuple):
        pmake.param('histParams', preCutConfig.binning)
    else:
        pmake.param('customBinning', preCutConfig.binning)
    resource.path.add_module(pmake)

    resource.condition = ('EventType', '==0')
    resource.halt = True
    return


def PreCutDeterminationPerChannel(resource, channelName, preCut):
    """
    Returns the preCut for given channel as Resource
        @param resource object
        @param channelName unique name describing the channel
        @param preCut global pre cut dictionary
        @param dictionary providing a key 'cutstring', 'nSignal', 'nBackground'
    """
    resource.cache = True
    if preCut is None:
        return None
    return preCut[channelName]


def PreCutDetermination(resource, channelNames, preCutConfig, preCutHistograms):
    """
    Determines the PreCuts for all the channels of a particle.
        @param resource object
        @param channelNames list of unique names describing the channels
        @param preCutConfig configuration for PreCut determination
        @param preCutHistograms filenames of the histogram files created for every channel by CreatePreCutHistogram
        @param dictionary containing  preCut for each channel
    """
    resource.cache = True
    if all(p is None for p in preCutHistograms):
        return None

    # Remove all channelsNames and PreCutHistograms which are ignored
    results = {c: None for c in channelNames}
    channelNames, preCutHistograms = list(zip(*[(c, p) for c, p in zip(channelNames, preCutHistograms) if p is not None]))

    # Calculate common PreCuts
    commonPreCuts = preCutDetermination.CalculatePreCuts(preCutConfig, channelNames, preCutHistograms)
    for (channelName, cut) in commonPreCuts.items():
        results[channelName] = None if cut['isIgnored'] else cut
    return results


def PostCutDetermination(resource, postCutConfig):
    """
    Determines the PostCut for all the channels of a particle.
        @param resource object
        @param postCutConfig configurations for post cut determination
        @param dictionary with the key 'cutstring' and 'range'
    """
    resource.cache = True
    if postCutConfig.value <= 0.0:
        return {'cutstring': '', 'range': (0, 1)}
    return {'cutstring': str(postCutConfig.value) + ' < extraInfo(SignalProbability)',
            'range': (postCutConfig.value, 1)}


def CalculateInverseSamplingRate(resource, distribution):
    """
    Calculates the inverse sampling rates used in the MVC training
        @param resource object
        @param distribution information about number of background an signal events
        @return dictionary of inverseSampling Rates for signal (1) and background (0)
    """
    resource.cache = True
    if distribution is None:
        return
    maxEvents = int(1e7)
    inverseSamplingRates = {}
    if distribution['nBackground'] > maxEvents:
        inverseSamplingRates[0] = int(distribution['nBackground'] / maxEvents) + 1
    if distribution['nSignal'] > maxEvents:
        inverseSamplingRates[1] = int(distribution['nSignal'] / maxEvents) + 1
    return inverseSamplingRates


def CalculateNumberOfBins(resource, distribution):
    """
    Calculates the number of bins used in the MVC training pdfs
        @param resource object
        @param distribution information about number of background an signal events
        @return int number of bins
    """
    resource.cache = True
    if distribution is None:
        return
    Nbins = 'CreateMVAPdfs:NbinsMVAPdf=50:'
    if distribution['nSignal'] > 1e5:
        Nbins = 'CreateMVAPdfs:NbinsMVAPdf=100:'
    if distribution['nSignal'] > 1e6:
        Nbins = 'CreateMVAPdfs:NbinsMVAPdf=200:'
    return Nbins


def GenerateTrainingData(resource, particleList, mvaConfig, inverseSamplingRates, additionalDependencies):
    """
    Generates the training data for the training of an MVC
        @param resource object
        @param particleList the particleList which is used for training and classification
        @param mvaConfig configuration for the multivariate analysis
        @param inverseSamplingRates dictionary of inverseSampling Rates for signal (1) and background (0)
        @return string ROOT filename
    """
    resource.cache = True
    if particleList is None or (additionalDependencies is not None and any([d is None for d in additionalDependencies])):
        return

    rootFilename = removeJPsiSlash('{particleList}_{hash}.root'.format(particleList=particleList, hash=resource.hash))

    if not os.path.isfile(rootFilename):
        teacher = register_module('TMVATeacher')
        teacher.set_name('TMVATeacher_' + particleList)
        teacher.param('prefix', removeJPsiSlash(particleList + '_' + resource.hash))
        teacher.param('variables', mvaConfig.variables)
        teacher.param('sample', mvaConfig.target)
        teacher.param('spectators', [mvaConfig.target])
        teacher.param('listNames', [particleList])
        teacher.param('inverseSamplingRates', inverseSamplingRates)
        teacher.param('maxSamples', int(2e7))
        resource.path.add_module(teacher)
        resource.condition = ('EventType', '==0')
        resource.halt = True
        return
    return rootFilename


def GenerateSPlotModel(resource, name, mvaConfig, distribution):
    """
    Generates the SPlot model for the training of an MVC
        @param resource object
        @param name of the particle or channel
        @param mvaConfig configuration for the multivariate analysis
        @param distribution information about number of background an signal events
        @return dict sPlotTeacher parameters in a dictionary
    """
    if mvaConfig.model != 'M':
        raise RuntimeError('SPlot is only supported for M model')

    resource.cache = True

    if distribution is None:
        return

    from ROOT import (RooRealVar, RooGaussian, RooChebychev, RooAddPdf, RooArgList, RooFit, RooAbsReal, kFALSE, kTRUE)

    # Parameters for TMVASPlotTeacher
    # TODO allow user to specify modelFileName using the model parameter
    # TODO Set Initial Fit values using Monte Carlo!
    modelFileName = removeJPsiSlash('model_{name}_{hash}.root'.format(name=name, hash=resource.hash))

    low = max(distribution['range'][0], distribution['signalPeak'] - 3 * distribution['signalWidth'])
    high = min(distribution['signalPeak'] + 3 * distribution['signalWidth'], distribution['range'][1])
    var = distribution['variable']

    if not os.path.isfile(modelFileName):
        # observable
        M = RooRealVar(var, var, low, high)
        M.setBins(250)

        # Setup component PDFs
        m = RooRealVar("m", "Mass", distribution['signalPeak'])
        sigma = RooRealVar("sigma", "Width of Gaussian", distribution['signalWidth'])
        sig = RooGaussian("sig", "Model", M, m, sigma)
        m.setConstant(kTRUE)
        sigma.setConstant(kFALSE)

        a0 = RooRealVar("a0", "a0", -0.69)
        a1 = RooRealVar("a1", "a1", 0.1)
        a0.setConstant(kFALSE)
        a1.setConstant(kFALSE)
        bkg = RooChebychev("bkg", "Background", M, RooArgList(a0, a1))

        # Add signal and background
        # initial value and maximal value will be set inside TMVASPlotTeacher
        total = float(distribution['nBackground'] + distribution['nSignal'])
        bkgfrac = RooRealVar("background", "fraction of background", distribution['nBackground'] / total)
        sigfrac = RooRealVar("signal", "fraction of background", distribution['nSignal'] / total)

        bkgfrac.setConstant(kFALSE)
        sigfrac.setConstant(kFALSE)

        model = RooAddPdf("model", "bkg+sig", RooArgList(bkg, sig), RooArgList(bkgfrac, sigfrac))

        # Write model to file and close the file, so TMVASPlotTeacher can open it
        # It's important to use ROOT.TFile here, otherwise the test will fail
        modelFile = ROOT.TFile(modelFileName, "RECREATE")
        model.Write("model")
        modelFile.ls()
        modelFile.Close()

    model = [{'cut': '{} < {} < {}'.format(low, var, high)},
             {'modelFileName': modelFileName,
              'discriminatingVariables': [var]}]
    return model


def GenerateTrainingDataUsingSPlot(resource, particleList, mvaConfig, sPlotParameters, additionalDependencies):
    """
    Generates the training data for the training of an MVC
        @param resource object
        @param particleList the particleList which is used for training and classification
        @param mvaConfig configuration for the multivariate analysis
        @param sPlotParameters dictionary containing the sPlot parameters, like the model filename
        @param Nbins number of bins
        @return string ROOT filename
    """
    resource.cache = True
    if particleList is None or (additionalDependencies is not None and any([d is None for d in additionalDependencies])):
        return

    rootFilename = removeJPsiSlash('{particleList}_{hash}.root'.format(particleList=particleList, hash=resource.hash))

    if not os.path.isfile(rootFilename):
        modularAnalysis.cutAndCopyList(particleList + '_tmp', particleList, sPlotParameters[0]['cut'],
                                       path=resource.path, writeOut=False)
        teacher = register_module('TMVATeacher')
        teacher.set_name('TMVATeacher_' + particleList)
        teacher.param('prefix', removeJPsiSlash(particleList + '_' + resource.hash))
        teacher.param('variables', mvaConfig.variables)
        teacher.param('listNames', [particleList + '_tmp'])
        teacher.param('spectators', sPlotParameters[1]['discriminatingVariables'])
        teacher.param('maxSamples', int(2e7))
        resource.path.add_module(teacher)

        B2WARNING("SPlot is still using MC-data! Otherwise we couldn't test it, due to the lack of real data!")
        resource.condition = ('EventType', '==0')  # TODO Replace 0 with 1
        resource.halt = True
        return
    return rootFilename


def TrainMultivariateClassifier(resource, mvaConfig, Nbins, trainingData):
    """
    Train multivariate classifier using the trainingData
        @param resource object
        @param mvaConfig configuration for the multivariate analysis
        @param Nbins number of bins
        @param trainingData name of ROOT-File which contains training data
        @return string config filename
    """
    resource.cache = True
    if trainingData is None:
        return

    configFilename = trainingData[:-5] + '_1.config'

    if not os.path.isfile(configFilename):
        command = (
            "{externTeacher} --methodName '{name}' --methodType '{type}' --methodConfig '{config}' --target '{target}'"
            " --variables '{variables}' --prefix '{prefix}' > '{prefix}'.log 2>&1".format(
                externTeacher=resource.env['externTeacher'],
                name=mvaConfig.name,
                type=mvaConfig.type,
                config=Nbins + mvaConfig.config,
                target=mvaConfig.target,
                variables="' '".join(mvaConfig.variables),
                prefix=trainingData[:-5]))
        B2INFO("Used following command to invoke teacher\n" + command)
        # The training of the MVC can run in parallel!
        # FIXME Bug? Because subprocess is not thread-safe, did not cause any problems so far.
        with resource.EnableMultiThreading():
            subprocess.call(command, shell=True)

    if not os.path.isfile(configFilename):
        B2ERROR("Training of MVC failed. Ignoring channel.")
        return

    return configFilename


def SignalProbability(resource, particleList, mvaConfig, configFilename):
    """
    Calculates the SignalProbability of a ParticleList using the previously trained MVC
        @param resource object
        @param particleList the particleList which is used for training and classification
        @param mvaConfig configuration for the multivariate analysis
        @param configFilename name of file which contains the weights of thhe previously trained MVC
        @return string config filename
    """
    resource.cache = True
    if configFilename is None or particleList is None:
        return
    expert = register_module('TMVAExpert')
    expert.set_name('TMVAExpert_' + particleList)
    expert.param('prefix', configFilename[:-9])  # without _1.config suffix
    expert.param('method', mvaConfig.name)
    expert.param('signalFraction', -1)  # Use signalFraction from training
    expert.param('transformToProbability', True)
    expert.param('expertOutputName', 'SignalProbability')
    expert.param('signalClass', 1)
    expert.param('listNames', [particleList])
    resource.path.add_module(expert)
    return resource.hash


def TagUniqueSignal(resource, particleList, signalProbability, target):
    """
    Saves the calculated signal probability for this particle list
        @param resource object
        @param particleList the particleList
        @param signalProbability signalProbability as additional dependency
        @param target target variable
        @return string variable name that provides tag
    """
    resource.cache = True
    if particleList is None or signalProbability is None:
        return

    uniqueSignal = register_module('TagUniqueSignal')
    uniqueSignal.param('particleList', particleList)
    uniqueSignal.param('target', target)
    uniqueSignal.param('extraInfoName', 'uniqueSignal')
    uniqueSignal.set_name('TagUniqueSignal_' + particleList)
    resource.path.add_module(uniqueSignal)
    return 'extraInfo(uniqueSignal)'


def VariablesToNTuple(resource, particleList, signalProbability, target):
    """
    Saves the calculated signal probability for this particle list
        @param resource object
        @param particleList the particleList
        @param signalProbability signalProbability as additional dependency
        @param target target variable
        @return Resource named VariablesToNTuple_{particleIdentifier} providing root filename
    """
    resource.cache = True
    if particleList is None or signalProbability is None:
        return

    filename = removeJPsiSlash('var_{i}_{h}.root'.format(i=particleList, h=resource.hash))

    if not os.path.isfile(filename):
        output = register_module('VariablesToNtuple')
        output.set_name('VariablesToNtuple_' + particleList)
        output.param('particleList', particleList)
        variables = [target, 'extraInfo(SignalProbability)', 'Mbc', 'mcErrors',
                     'cosThetaBetweenParticleAndTrueB', 'extraInfo(uniqueSignal)']
        output.param('variables', variables)
        output.param('fileName', filename)
        output.param('treeName', 'variables')
        resource.path.add_module(output)
        resource.condition = ('EventType', '==0')
        resource.halt = True
        return
    return filename


def SaveModuleStatistics(resource):
    """
    Creates .root file that contains statistics for all modules running in final execution.
        @param resource object
        @return root file name
    """
    resource.cache = True
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
    return filename


def CountMCParticles(resource, names):
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
    return filename


def CountParticleLists(resource, targets, lists):
    """
    Counts the number of particles in all given mc-matched lists.
        @param resource object
        @param targets mva target variable
        @param lists list of all FSP particles and channels
    """
    resource.cache = True
    filename = 'listCounts.root'

    if not os.path.isfile(filename):
        output = register_module('VariablesToHistogram')
        output.set_name("VariablesToHistogram_ListCount")
        output.param('variables', [('countInList({l})'.format(l=l), 1000, -0.5, 999.5)
                                   for l in lists if l is not None] +
                                  [('countInList({l}, {t} == 1)'.format(l=l, t=target), 1000, -0.5, 999.5)
                                   for l, target in zip(lists, targets) if l is not None] +
                                  [('countInList({l}, {t} == 0)'.format(l=l, t=target), 1000, -0.5, 999.5)
                                   for l, target in zip(lists, targets) if l is not None])
        output.param('fileName', filename)
        resource.path.add_module(output)
        resource.condition = ('EventType', '==0')
        resource.halt = True
        return
    return filename


def SaveSummary(resource, mcCounts, listCounts, moduleStatistics, particles, ntuples, preCuts, postCuts,
                plists, clists, mlists, mothers, cnames, preCutHistograms, inverseSamplingRates, trainingData):
    """
    Save all important files, determined cuts and the all configurations objects in a pickled file,
    so we can use it later to produce a compact or a detailed training report
        @param resource object
        @param mcCounts filename containing mcCounts
        @param listCount filename containing list counts
        @param particles all particle objects given by the user
        @param ntuples filenames of ntuples produced for each particle
        @param preCuts determined preCut object for each particle
        @param postCuts deterined postCut object for each particle
        @param plists particle list names
        @param clists raw channel list names
        @param mlists matched channel list names
        @param mothers mother particle identifier of each channel
        @param cnames channel names of each channel
        @param preCutHistograms filename of preCutHistogram for each channel
        @param inverseSamplingRates inverse sampling rates for each channel
        @param trainingData filename of TMVA training data input for each channel
    """

    resource.cache = True
    filename = 'Summary_' + resource.hash + '.pickle'

    obj = {'mc_counts': mcCounts,
           'list_counts': listCounts,
           'module_statistics': moduleStatistics,
           'particles': particles,
           'ntuples': ntuples,
           'pre_cuts': preCuts,
           'post_cuts': postCuts,
           'plists': plists,
           'clists': clists,
           'mlists': mlists,
           'mothers': mothers,
           'cnames': cnames,
           'pre_cut_histograms': preCutHistograms,
           'inverse_sampling_rates': inverseSamplingRates,
           'training_data': trainingData}

    out = open(filename, 'wb')
    pickle.dump(obj, out)

    return filename
