#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Thomas Keck 2015
#

from basf2 import *
import ROOT
from ROOT import Belle2

import modularAnalysis
import pdg

from fei import preCutDetermination
from fei import automaticReporting
from fei.automaticReporting import removeJPsiSlash

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
    geometry = register_module('Geometry')
    geometry.param('components', ['MagneticField'])
    resource.path.add_module(geometry)


def SelectParticleList(resource, particleName):
    """
    Creates a ParticleList gathering up all Particles with the given particleName
        @param resource object
        @param particleName valid pdg particle name
        @param particleLabel user defined label
        @return name of ParticleList
    """
    resource.cache = True
    particleList = particleName + ':' + resource.hash
    cut = 'isInRestOfEvent > 0.5' if resource.env['ROE'] else ''
    modularAnalysis.fillParticleList(particleList, cut, writeOut=True, path=resource.path)
    return particleList


def MatchParticleList(resource, particleList):
    """
    Match MC truth of the given ParticleList
        @param resource object
        @param particleList raw ParticleList
        @return name of matched ParticleList
    """
    resource.cache = True
    if particleList is None:
        return
    resource.condition = ('EventType', '==0')
    modularAnalysis.matchMCTruth(particleList, path=resource.path)
    return particleList


def MakeParticleList(resource, particleName, daughterParticleLists, preCut, decayModeID):
    """
    Creates a ParticleList by combining other ParticleLists via the ParticleCombiner module.
        @param resource object
        @param particleName valid pdg particle name
        @param daughterParticleLists list of ParticleList name of every daughter particles
        @param preCut dictionary containing 'cutstring', a string which defines the cut which is applied
               before the combination of the daughter particles.
        @param decayModeID integer ID of this decay channel, added to extra-info of Particles
        @return name of new ParticleList
    """
    resource.cache = True
    if preCut is None:
        return
    particleList = particleName + ':' + resource.hash
    decayString = particleList + ' ==> ' + ' '.join(daughterParticleLists)
    modularAnalysis.reconstructDecay(decayString, preCut['cutstring'], decayModeID, writeOut=True, path=resource.path)
    return particleList


def CopyParticleLists(resource, particleName, particleLists, postCut, signalProbabilities):
    """
    Creates a ParticleList gathering up all particles in the given inputLists
        @param resource object
        @param particleName valid pdg particle name
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
        postCut['cutstring'] if postCut is not None else '',
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


def FitVertex(resource, channelName, particleList):
    """
    Fit secondary vertex of all particles in this ParticleList
        @param resource object
        @param channelName unique name describing the channel
        @param particleList ParticleList name
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
    pvfit.param('confidenceLevel', -2)  # don't remove Particles with failed fit (pValue = -1)
    pvfit.param('vertexFitter', 'kfitter')
    pvfit.param('fitType', 'vertex')
    resource.path.add_module(pvfit)

    return resource.hash


def CreatePreCutHistogram(resource, particleName, channelName, mvaConfigTarget, preCutConfig, daughterParticleLists,
                          additionalDependencies):
    """
    Creates ROOT file with chosen pre cut variable histogram of this channel (signal/background)
    for a given particle, before any intermediate cuts are applied.
        @param resource object
        @param particleName valid pdg particle name
        @param channelName unique name describing the channel
        @param mvaConfigTarget variable which defines signal and background
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
    pmake.param('cut', preCutConfig.userCut)
    pmake.param('target', mvaConfigTarget)
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
    channelNames, preCutHistograms = zip(*[(c, p) for c, p in zip(channelNames, preCutHistograms) if p is not None])

    # Calculate common PreCuts
    # !! You probabily do NOT want to change the next line, otherwise you're probabily going to break the corresponding test
    #    of this function.
    commonPreCuts = preCutDetermination.CalculatePreCuts(preCutConfig, channelNames, preCutHistograms)
    for (channelName, cut) in commonPreCuts.iteritems():
        if preCutConfig.userCut != '':
            cut['cutstring'] += ' and [' + preCutConfig.userCut + ']'
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
    if postCutConfig is None:
        return
    return {'cutstring': str(postCutConfig.value) + ' < extraInfo(SignalProbability)',
            'range': (postCutConfig.value, 1)}


def FSPDistribution(resource, inputList, mvaConfigTarget):
    """
    Returns signal and background distribution of FSP
    Counts the number of MC Particles for every pdg code in all events
        @param resource object
        @param inputList particle list name
        @param mvaConfigTarget configuration for the multivariate analysis
    """
    resource.cache = True
    filename = removeJPsiSlash('{i}_{h}.root'.format(i=inputList, h=resource.hash))

    if not os.path.isfile(filename):
        output = register_module('VariablesToNtuple')
        output.param('particleList', inputList)
        output.param('variables', [mvaConfigTarget])
        output.param('fileName', filename)
        output.param('treeName', 'distribution')
        resource.path.add_module(output)
        resource.condition = ('EventType', '==0')
        resource.halt = True
        return None

    rootfile = ROOT.TFile(filename)
    distribution = rootfile.Get('distribution')
    return {'nSignal': int(distribution.GetEntries(mvaConfigTarget + ' == 1')),
            'nBackground': int(distribution.GetEntries(mvaConfigTarget + ' == 0'))}


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
    Nbins = 'NbinsMVAPdf=50:'
    if distribution['nSignal'] > 1e5:
        Nbins = 'NbinsMVAPdf=100:'
    if distribution['nSignal'] > 1e6:
        Nbins = 'NbinsMVAPdf=200:'
    return Nbins


def GenerateTrainingData(resource, particleList, mvaConfig, inverseSamplingRates, Nbins, additionalDependencies):
    """
    Generates the training data for the training of an MVC
    TODO Implement training with sPlot on real data
        @param resource object
        @param particleList the particleList which is used for training and classification
        @param mvaConfig configuration for the multivariate analysis
        @param Nbins number of bins
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
        teacher.param('methods', [(mvaConfig.name, mvaConfig.type, Nbins + mvaConfig.config)])  # Add number of bins for pdfs
        teacher.param('factoryOption', '!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification')
        teacher.param('prepareOption', 'SplitMode=random:!V')
        teacher.param('variables', mvaConfig.variables)
        teacher.param('createMVAPDFs', True)
        teacher.param('target', mvaConfig.target)
        teacher.param('listNames', [particleList])
        teacher.param('inverseSamplingRates', inverseSamplingRates)
        teacher.param('doNotTrain', True)
        resource.path.add_module(teacher)

        resource.condition = ('EventType', '==0')
        resource.halt = True
        return
    return rootFilename


def SignalProbability(resource, particleList, mvaConfig, inverseSamplingRates, Nbins, trainingData):
    """
    Calculates the SignalProbability of a ParticleList. If the files required from TMVAExpert aren't available they're created.
        @param resource object
        @param particleList the particleList which is used for training and classification
        @param mvaConfig configuration for the multivariate analysis
        @param inverseSamplingRates dictionary of inverseSampling Rates for signal (1) and background (0)
        @param Nbins number of bins
        @param trainingData name of ROOT-File which contains training data
        @return string config filename
    """
    resource.cache = True
    if trainingData is None or particleList is None:
        return
    configFilename = trainingData[:-5] + '.config'

    if not os.path.isfile(configFilename):
        command = (
            "externTeacher --methodName '{name}' --methodType '{type}' --methodConfig '{config}' --target '{target}'"
            " --variables '{variables}' --factoryOption '{foption}' --prepareOption '{poption}' --prefix '{prefix}'"
            " --maxEventsPerClass {maxEvents}"
            " > '{prefix}'.log 2>&1".format(
                name=mvaConfig.name,
                type=mvaConfig.type,
                config='CreateMVAPdfs:' + Nbins + mvaConfig.config,
                target=mvaConfig.target,
                variables="' '".join(mvaConfig.variables),
                foption='!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification',
                poption='SplitMode=random:!V',
                maxEvents=int(1e7),
                prefix=trainingData[:-5]))
        B2INFO("Used following command to invoke teacher\n" + command)
        # The training of the MVC can run in parallel!
        # FIXME Bug?
        with resource.EnableMultiThreading():
            subprocess.call(command, shell=True)

    if os.path.isfile(configFilename):
        expert = register_module('TMVAExpert')
        expert.set_name('TMVAExpert_' + particleList)
        expert.param('prefix', trainingData[:-5])
        expert.param('method', mvaConfig.name)
        expert.param('signalFraction', -2)  # Use signalFraction from training
        expert.param('expertOutputName', 'SignalProbability')
        expert.param('signalClass', 1)
        expert.param('inverseSamplingRates', inverseSamplingRates)
        expert.param('listNames', [particleList])
        resource.path.add_module(expert)
        return configFilename

    resource.halt = True
    return


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


def VariablesToNTuple(resource, particleList, signalProbability, target, extraVars):
    """
    Saves the calculated signal probability for this particle list
        @param resource object
        @param particleList the particleList
        @param signalProbability signalProbability as additional dependency
        @param target target variable
        @param extraVars list of additional variables (can be empty)
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
        variables = [target, 'extraInfo(SignalProbability)', 'Mbc', 'mcErrors', 'cosThetaBetweenParticleAndTrueB']
        if extraVars is not None and len(extraVars) > 0 and extraVars[0] is not None:
            variables += extraVars
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
        output = register_module('VariablesToNtuple')
        output.param('variables', ['NumberOfMCParticlesInEvent({i})'.format(i=abs(pdg.from_name(name))) for name in names])
        output.param('fileName', filename)
        output.param('treeName', 'mccounts')
        resource.path.add_module(output)
        resource.condition = ('EventType', '==0')
        resource.halt = True
        return

    rootfile = ROOT.TFile(filename)
    countNtuple = rootfile.Get('mccounts')
    # makeROOTCompatible removes parenthesis from variable so we don't worry about them here
    getpdg = lambda x: x[len('NumberOfMCParticlesInEvent'):]

    counter = {}
    for branch in [str(k.GetName()) for k in countNtuple.GetListOfBranches()]:
        allMCParticles = ROOT.TH1D('allMCParticles', 'allMCParticles', 1, 0, countNtuple.GetMaximum(branch) + 1)
        countNtuple.Project('allMCParticles', branch, branch)
        counter[getpdg(branch)] = int(allMCParticles.Integral())
        del allMCParticles
    counter['NEvents'] = int(countNtuple.GetEntries())
    return counter


def WriteAnalysisFileForChannel(
        resource,
        particleName,
        particleLabel,
        channelName,
        preCutConfig,
        preCut,
        preCutHistogram,
        mvaConfig,
        signalProbability,
        postCutConfig,
        postCut):
    """
    Creates a pdf document with the PreCut and Training plots
        @param resource object
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
        @return dictionary containing latex placeholders of this channel
    """
    placeholders = {}
    placeholders['particleName'] = particleName
    placeholders['particleLabel'] = particleLabel
    placeholders['channelName'] = channelName
    placeholders['isIgnored'] = False
    placeholders['mvaConfigObject'] = mvaConfig
    placeholders['texFile'] = removeJPsiSlash('{name}_{channel}_{hash}.tex'.format(
        name=particleName, channel=channelName, hash=resource.hash))
    placeholders = automaticReporting.createPreCutTexFile(placeholders, preCutHistogram, preCutConfig, preCut)
    placeholders = automaticReporting.createMVATexFile(placeholders, mvaConfig, signalProbability, postCutConfig, postCut)
    automaticReporting.createTexFile(placeholders['texFile'], 'analysis/scripts/fei/templates/ChannelTemplate.tex', placeholders)

    resource.needed = False
    resource.cache = True
    return placeholders


def WriteAnalysisFileForFSParticle(
        resource,
        particleName,
        particleLabel,
        mvaConfig,
        signalProbability,
        postCutConfig,
        postCut,
        distribution,
        nTuple,
        mcCounts):
    """
    Creates a pdf document with the PreCut and Training plots
        @param resource object
        @param particleName valid pdg particle name
        @param particleLabel user defined label
        @param mvaConfig configuration for mva
        @param signalProbability config filename for TMVA training
        @param postCutConfig configuration for postCut
        @param postCut
        @param mcCounts
        @param distribution
        @return dictionary containing latex placeholders of this particle
    """
    placeholders = {}
    placeholders['particleName'] = particleName
    placeholders['particleLabel'] = particleLabel
    placeholders['isIgnored'] = False

    placeholders = automaticReporting.createMVATexFile(placeholders, mvaConfig, signalProbability, postCutConfig, postCut)
    placeholders = automaticReporting.createFSParticleTexFile(placeholders, nTuple, mcCounts, distribution, mvaConfig)

    resource.needed = False
    resource.cache = True
    return placeholders


def WriteAnalysisFileForCombinedParticle(resource, particleName, particleLabel, channelPlaceholders, nTuple, mcCounts):
    """
    Creates a pdf document with the PreCut and Training plots
        @param resource object
        @param particleName valid pdg particle name
        @param particleLabel user defined label
        @param channelPlaceholders list of all tex placeholder dictionaries of all channels
        @param mcCounts
        @return dictionary containing latex placeholders of this particle
    """
    placeholders = {'channels': channelPlaceholders}
    placeholders['particleName'] = particleName
    placeholders['particleLabel'] = particleLabel
    placeholders['isIgnored'] = False

    mvaConfig = channelPlaceholders[0]['mvaConfigObject']
    placeholders = automaticReporting.createCombinedParticleTexFile(placeholders, channelPlaceholders, nTuple, mcCounts, mvaConfig)

    resource.needed = False
    resource.cache = True
    return placeholders


def WriteAnalysisFileSummary(
        resource,
        finalStateParticlePlaceholders,
        combinedParticlePlaceholders,
        finalParticleNTuples,
        finalParticleTargets,
        cpuTimeSummaryPlaceholders,
        mcCounts,
        particles):
    """
    Creates a pdf summarizing all networks trained.
        @param resource object
        @param finalStateParticlePlaceholders list of all tex placeholder dictionaries of fsp
        @param combinedParticlePlaceholders list of all tex placeholder dictionaries of intermediate particles
        @param finalParticleNTuples list of ntuples of all final particles
        @param finalParticleTargets list of target variables of all final particles (corresponding to ntuple list)
        @param mcCounts
        @param particles particle objects
    """
    finalParticlePlaceholders = []
    for (ntuple, target) in zip(finalParticleNTuples, finalParticleTargets):
        if ntuple is not None:
            type = 'CosBDL' if 'semileptonic' in ntuple else 'Mbc'
            plot = automaticReporting.createMoneyPlotTexFile(ntuple, type, mcCounts, target)
            rocPlot = automaticReporting.createMoneyPlotTexFile(ntuple, "ROC", mcCounts, target)
            finalParticlePlaceholders.append(plot)
            finalParticlePlaceholders.append(rocPlot)

            rootfile = ROOT.TFile(ntuple)
            tree = rootfile.Get('variables')
            uniqueSignal = ROOT.Belle2.Variable.makeROOTCompatible('extraInfo(uniqueSignal)')
            nUniqueSignal = int(tree.GetEntries(target + ' && ' + uniqueSignal))

            # add nUniqueSignal back in corresponding combined particle placeholder
            for cplaceholder in combinedParticlePlaceholders:
                if cplaceholder['particleName'] == plot['particleName']:
                    cplaceholder['particleNUniqueSignalAfterPostCut'] = nUniqueSignal

    placeholders = automaticReporting.createSummaryTexFile(
        finalStateParticlePlaceholders,
        combinedParticlePlaceholders,
        finalParticlePlaceholders,
        cpuTimeSummaryPlaceholders,
        mcCounts,
        particles)

    subprocess.call('cp {f} .'.format(f=ROOT.Belle2.FileSystem.findFile('analysis/scripts/fei/templates/nordbert.pdf')), shell=True)
    for i in range(0, 2):
        ret = subprocess.call(['pdflatex', '-halt-on-error', '-interaction=nonstopmode', placeholders['texFile']])
        if ret == 0:
            B2INFO("Created FEI summary PDF.")
        else:
            B2ERROR("pdflatex failed to create FEI summary PDF, please check.")

    if ret == 0:
        filename = 'sent_mail'
        if not os.path.isfile(filename):
            # automaticReporting.sendMail()
            open(filename, 'w').close()

    resource.needed = False
    resource.cache = True
    return


def WriteCPUTimeSummary(resource, channelNames, inputLists, channelPlaceholders, mcCounts, moduleStatisticsFile):
    """
    Creates CPU time summary
        @param resource object
        @param mcCounts
        @param moduleStatisticsFile file name of the TFile containing actual statistics
        @return dictionary of placeholders used in the .tex file
    """
    stats = automaticReporting.getModuleStatsFromFile(moduleStatisticsFile)
    placeholders = automaticReporting.createCPUTimeTexFile(
        channelNames,
        inputLists,
        channelPlaceholders,
        mcCounts,
        moduleStatisticsFile,
        stats)

    return placeholders
