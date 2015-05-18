#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Thomas Keck 2014-2015

# FEI defines own command line options, therefore we disable
# the ROOT command line options, which otherwise interfere sometimes.
from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = True

# FEI uses multi-threading for parallel execution of tasks therefore
# the ROOT gui-thread is disabled, which otherwise interferes sometimes
PyConfig.StartGuiThread = False
import ROOT

# Import basf2
from basf2 import *
# Should come after basf2 import
import pdg

# The dagFramework is used to define a dependency graph of the different
# task necessary within the FEI.
from fei import dagFramework
# The different tasks themselves are defined in the provider module
from fei import provider

# Standard python modules
import collections
import argparse
import copy
import os
import itertools
from functools import reduce


def getCommandLineOptions():
    """ Parses the command line options of the fei and returns the corresponding arguments. """
    parser = argparse.ArgumentParser()
    parser.add_argument('-verbose', '--verbose', dest='verbose', action='store_true', help='Output additional information')
    parser.add_argument('-summary', '--summary', dest='summary', action='store_true', help='Create FEI report as PDF file')
    parser.add_argument('-nThreads', '--nThreads', dest='nThreads', type=int, default=1, help='Number of threads')
    parser.add_argument('-cache', '--cache', dest='cache', type=str, default=None,
                        help='Use the given file to cache results between multiple executions.'
                             'Data from previous runs has to be provided as input!')
    parser.add_argument('-prune', '--prune', dest='prune', action='store_true',
                        help='Prune ParticleStoreArray and ParticleLists')
    parser.add_argument('-boost', '--boost', dest='boost', action='store_true',
                        help='Boost last stage of MVCs with higher statistics')
    parser.add_argument('-rerunCached', '--rerunCached', dest='rerunCachedProviders', action='store_true',
                        help='Runs cached providers again')
    args = parser.parse_args()
    return args


# Define classes at top level to make them pickable
# Create new class called MVAConfiguration via namedtuple. namedtuples are like a struct in C
MVAConfiguration = collections.namedtuple('MVAConfiguration', 'name, type, config, variables, target, model')
# Create new class called PreCutConfiguration via namedtuple. namedtuples are like a struct in C
PreCutConfiguration = collections.namedtuple('PreCutConfiguration', 'variable, binning, efficiency, purity, userCut')
# Create new class called PostCutConfiguration via namedtuple. namedtuples are like a struct in C
PostCutConfiguration = collections.namedtuple('PostCutConfiguration', 'value')
# Create new class called DecayChannel via namedtuple. namedtuples are like a struct in C
DecayChannel = collections.namedtuple('DecayChannel', 'name, daughters, mvaConfig, decayModeID')


class Particle(object):
    """
    The Particle class is the only class the end-user gets into contact with.
    The user creates an instance of this class for every particle he wants to reconstruct with the FEI algorithm,
    and provides MVAConfiguration, PreCutConfiguration and PostCutConfiguration. These can be overwritten per channel.
    """

    def __init__(self, identifier, mvaConfig, preCutConfig=None, postCutConfig=None):
        """
        Creates a Particle without any decay channels. To add decay channels use addChannel method.
            @param identifier is the pdg name of the particle as a string
                   with an optional additional user label seperated by ':'
            @param mvaConfig multivariate analysis configuration
            @param preCutConfig intermediate pre cut configuration
            @param postCutConfig post cut configuration
        """
        # Is the correct pdg name as a string of the particle with an optional additional user label seperated by :
        self.identifier = identifier + ':generic' if len(identifier.split(':')) < 2 else identifier
        v = self.identifier.split(':')
        # The name of the particle as correct pdg name e.g. K+, pi-, D*0.
        self.name = v[0]
        # Additional label like hasMissing or has2Daughters
        self.label = v[1]
        # multivariate analysis configuration (see MVAConfiguration)
        self.mvaConfig = mvaConfig
        # DecayChannel objects added by addChannel() method.
        self.channels = []
        # intermediate cut configuration (see PreCutConfiguration)
        self.preCutConfig = preCutConfig
        # post cut configuration (see PostCutConfiguration)
        self.postCutConfig = postCutConfig

    @property
    def charge_conjugated_identifier(self):
        """ Property returning the identifier of the charge conjugated identifier """
        return pdg.conjugate(self.name) + ':' + self.label

    @property
    def daughters(self):
        """ Property returning list of unique daughter particles of all channels """
        return list(frozenset([daughter for channel in self.channels for daughter in channel.daughters]))

    @property
    def isFSP(self):
        """ Property returning true if the particle is a final state particle """
        return self.channels == []

    def addChannel(self, daughters, mvaConfig=None):
        """
        Appends a new decay channel to the Particle object.
            @param daughters is a list of pdg particle names e.g. ['pi+','K-']
            @param mvaConfig multivariate analysis configuration
        """
        # Append generic label to all defined daughters if no label was set yet
        daughters = [d + ':generic' if ':' not in d else d for d in daughters]
        # Use default mvaConfig of this particle if no channel-specific config is given
        mvaConfig = copy.deepcopy(self.mvaConfig if mvaConfig is None else mvaConfig)
        # At the moment all channels must have the same target variable. Why?
        if mvaConfig is not None and mvaConfig.target != self.mvaConfig.target:
            B2FATAL(
                'Particle %s has common target %s, while channel %s has %s. Each particle must have exactly one target!' %
                (particle.identifier, self.mvaConfig.target, ' '.join(daughters), mvaConfig.target))
        # Replace generic-variables with ordinary variables.
        # All instances of {} are replaced with all combinations of daughter indices
        mvaVars = []
        for v in mvaConfig.variables:
            if v.count('{}') <= len(daughters):
                mvaVars += [v.format(*c) for c in itertools.combinations(range(0, len(daughters)), v.count('{}'))]
        mvaConfig = mvaConfig._replace(variables=mvaVars)
        # Add new channel
        self.channels.append(DecayChannel(name=self.identifier + ' ==> ' + ' '.join(daughters),
                                          daughters=daughters,
                                          mvaConfig=mvaConfig,
                                          decayModeID=len(self.channels)))
        return self


# Simple object containing the output of fei
FeiState = collections.namedtuple('FeiState', 'path, is_trained')


def fullEventInterpretation(selection_path, particles):
    """
    The Full Event Interpretation algorithm has to be executed multiple times, because of the dependencies between
    the MVCs among each other and PreCuts on Histograms.
    These dependencies are automatically solved and a basf2 path is returned containing all needed modules for this stage.
        @param selection_path basf2 module path to execute before any tag-side reconstruction.
               The path should load data, select a signal-side B and create a 'RestOfEvents' list.
               Use None to perform independent tag-side reconstruction (equivalent to old Belle I Full Reconstruction).
        @param particles list of particle objects which shall be reconstructed by this algorithm
        @return FeiState object containing basf2 path to execute, plus status information
    """
    args = getCommandLineOptions()

    # Create a new directed acyclic graph
    dag = dagFramework.DAG()

    # Set environment variables
    dag.env['ROE'] = selection_path is not None
    dag.env['prune'] = args.prune
    dag.env['verbose'] = args.verbose
    dag.env['nThreads'] = args.nThreads
    dag.env['rerunCachedProviders'] = args.rerunCachedProviders

    # Add basic properties defined by the user of all Particles as Resources into the graph
    for particle in particles:
        dag.add('Name_' + particle.identifier, particle.name)
        dag.add('Label_' + particle.identifier, particle.label)
        dag.add('Identifier_' + particle.identifier, particle.identifier)
        dag.add('Object_' + particle.identifier, particle)
        dag.add('MVAConfig_' + particle.identifier, particle.mvaConfig)
        dag.add('PreCutConfig_' + particle.identifier, particle.preCutConfig)
        dag.add('PostCutConfig_' + particle.identifier, particle.postCutConfig)

        for channel in particle.channels:
            dag.add('Name_' + channel.name, channel.name)
            dag.add('MVAConfig_' + channel.name, channel.mvaConfig)
            dag.add('MVAConfigTarget_' + channel.name, channel.mvaConfig.target)
            dag.add('DecayModeID_' + channel.name, channel.decayModeID)
        dag.add('MVAConfigTarget_' + particle.identifier, particle.mvaConfig.target)

    # Add Modules which are always needed
    dag.add('gearbox', provider.LoadGearbox)
    dag.add('geometry', provider.LoadGeometry, 'gearbox')

    # Reconstruct given particle topology
    for particle in particles:
        if particle.isFSP:
            dag.add('RawParticleList_' + particle.identifier, provider.SelectParticleList,
                    particleName='Name_' + particle.identifier)
            dag.add('MatchedParticleList_' + particle.identifier, provider.MatchParticleList,
                    particleList='RawParticleList_' + particle.identifier)
        else:
            # Reconstruct every channel
            for channel in particle.channels:
                dag.add('RawParticleList_' + channel.name, provider.MakeParticleList,
                        particleName='Name_' + particle.identifier,
                        daughterParticleLists=['ParticleList_' + daughter for daughter in channel.daughters],
                        preCut='PreCut_' + channel.name,
                        decayModeID='DecayModeID_' + channel.name)
                dag.add('MatchedParticleList_' + channel.name, provider.MatchParticleList,
                        particleList='RawParticleList_' + channel.name)

        # Copy all channel lists into a single particle list
        dag.add('ParticleList_' + particle.identifier, provider.CopyParticleLists,
                particleName='Name_' + particle.identifier,
                particleLists=['RawParticleList_' + particle.identifier] if particle.isFSP
                else ['RawParticleList_' + channel.name for channel in particle.channels],
                postCut='PostCut_' + particle.identifier,
                signalProbabilities=['SignalProbability_' + particle.identifier] if particle.isFSP
                else ['SignalProbability_' + channel.name for channel in particle.channels])

        # Copy particle list into a list with a human readable name
        dag.add('HumanReadableParticleList_' + particle.identifier, provider.CopyIntoHumanReadableParticleList,
                particleName='Name_' + particle.identifier,
                particleLabel='Label_' + particle.identifier,
                particleList='ParticleList_' + particle.identifier)

        # Add the same Resource for the charge conjugated particle
        if particle.name != pdg.conjugate(particle.name):
            dag.add('ParticleList_' + particle.charge_conjugated_identifier, provider.PDGConjugate,
                    particleList='ParticleList_' + particle.identifier)

    # Vertex fit
    for particle in particles:
        if particle.isFSP:
            dag.add('VertexFit_' + particle.identifier, 'TrackFitIsAlreadyDoneForFSPs')
        else:
            # Do vertex fit for every channel
            for channel in particle.channels:
                dag.add('VertexFit_' + channel.name, provider.FitVertex,
                        'geometry',
                        ['VertexFit_' + daughter for daughter in channel.daughters],
                        channelName='Name_' + channel.name,
                        particleList='RawParticleList_' + channel.name)
            # Create common VertexFit Resource for this particle, thereby its easy to state the dependency
            # on the VertexFit of a daughter particle in the line above!
            dag.add('VertexFit_' + particle.identifier, provider.HashRequirements,
                    ['VertexFit_' + channel.name for channel in particle.channels])

        # Add the same Resource for the charge conjugated particle
        if particle.name != pdg.conjugate(particle.name):
            dag.add('VertexFit_' + particle.charge_conjugated_identifier, 'VertexFit_' + particle.identifier)

    # Determine Intermediate Cuts using the config objects provided by the user
    for particle in particles:
        # Determine PostCut
        dag.add('PostCut_' + particle.identifier, provider.PostCutDetermination,
                'Identifier_' + particle.identifier,
                ['SignalProbability_' + particle.identifier] if particle.isFSP
                else ['SignalProbability_' + channel.name for channel in particle.channels],
                postCutConfig='PostCutConfig_' + particle.identifier)

        # Determine PreCut for non FSPs
        if not particle.isFSP:
            for channel in particle.channels:
                additionalDependencies = []
                if 'SignalProbability' in particle.preCutConfig.variable:
                    additionalDependencies = ['SignalProbability_' + daughter for daughter in channel.daughters]

                dag.add('PreCutHistogram_' + channel.name, provider.CreatePreCutHistogram,
                        particleName='Name_' + particle.identifier,
                        channelName='Name_' + channel.name,
                        mvaConfigTarget='MVAConfigTarget_' + channel.name,
                        preCutConfig='PreCutConfig_' + particle.identifier,
                        daughterParticleLists=['ParticleList_' + daughter for daughter in channel.daughters],
                        additionalDependencies=additionalDependencies)

                dag.add('PreCut_' + channel.name, provider.PreCutDeterminationPerChannel,
                        channelName='Name_' + channel.name,
                        preCut='PreCut_' + particle.identifier)

                dag.add('PreCut_' + particle.identifier, provider.PreCutDetermination,
                        channelNames=['Name_' + channel.name for channel in particle.channels],
                        preCutConfig='PreCutConfig_' + particle.identifier,
                        preCutHistograms=['PreCutHistogram_' + channel.name for channel in particle.channels])

    # Trains multivariate classifiers (MVC) methods and provides signal probabilities
    for particle in particles:
        if particle.isFSP:
            # Create Distribution of Signal and Background
            dag.add('Distribution_' + particle.identifier, provider.FSPDistribution,
                    'gearbox',
                    inputList='MatchedParticleList_' + particle.identifier,
                    mvaConfigTarget='MVAConfigTarget_' + particle.identifier)
            # Calculate inverse sampling rate, thereby limit the maximum amount of statistic
            dag.add('InverseSamplingRate_' + particle.identifier, provider.CalculateInverseSamplingRate,
                    distribution='Distribution_' + particle.identifier)
            # Calculate a reasonable number of bins for the TMVA MVA-PDFs
            dag.add('Nbins_' + particle.identifier, provider.CalculateNumberOfBins,
                    distribution='Distribution_' + particle.identifier)
            # Generate training data from MC-matched ParticleList
            dag.add('TrainingData_' + particle.identifier, provider.GenerateTrainingData,
                    mvaConfig='MVAConfig_' + particle.identifier,
                    particleList='MatchedParticleList_' + particle.identifier,
                    inverseSamplingRates='InverseSamplingRate_' + particle.identifier,
                    Nbins='Nbins_' + particle.identifier,
                    additionalDependencies='None')
            # Train and apply MVC on raw ParticleList
            dag.add('TrainedMVC_' + particle.identifier, provider.TrainMultivariateClassifier,
                    mvaConfig='MVAConfig_' + particle.identifier,
                    Nbins='Nbins_' + particle.identifier,
                    trainingData='TrainingData_' + particle.identifier)
            dag.add('SignalProbability_' + particle.identifier, provider.SignalProbability,
                    mvaConfig='MVAConfig_' + particle.identifier,
                    particleList='RawParticleList_' + particle.identifier,
                    inverseSamplingRates='InverseSamplingRate_' + particle.identifier,
                    Nbins='Nbins_' + particle.identifier,
                    configFilename='TrainedMVC_' + particle.identifier)
        else:
            for channel in particle.channels:
                # Calculate inverse sampling rate, thereby limit the maximum amount of statistic,
                # uses nSignal and nBackground as determined by the PreCut algorithm
                dag.add('InverseSamplingRate_' + channel.name, provider.CalculateInverseSamplingRate,
                        distribution='PreCut_' + channel.name)
                # Calculate a reasonable number of bins for the TMVA MVA-PDFs
                # uses nSignal and nBackground as determined by the PreCut algorithm
                dag.add('Nbins_' + channel.name, provider.CalculateNumberOfBins,
                        distribution='PreCut_' + channel.name)

                # Add additional dependencies like VertexFit and SignalProbabilities if used in the MVC
                additionalDependencies = []
                if any('SignalProbability' in variable for variable in channel.mvaConfig.variables):
                    additionalDependencies += ['SignalProbability_' + daughter for daughter in channel.daughters]
                if any(variable in ['dx', 'dy', 'dz', 'dr', 'chiProb', 'significanceOfDistance', 'distance',
                                    'cosAngleBetweenMomentumAndVertexVector'] for variable in channel.mvaConfig.variables):
                    additionalDependencies += ['VertexFit_' + channel.name]

                # Generate training data from MC-matched ParticleList
                dag.add('TrainingData_' + channel.name, provider.GenerateTrainingData,
                        mvaConfig='MVAConfig_' + channel.name,
                        particleList='MatchedParticleList_' + channel.name,
                        inverseSamplingRates='InverseSamplingRate_' + channel.name,
                        Nbins='Nbins_' + channel.name,
                        additionalDependencies=additionalDependencies)

                # Train and apply MVC on raw ParticleList
                dag.add('TrainedMVC_' + channel.name, provider.TrainMultivariateClassifier,
                        mvaConfig='MVAConfig_' + channel.name,
                        Nbins='Nbins_' + channel.name,
                        trainingData='TrainingData_' + channel.name)
                dag.add('SignalProbability_' + channel.name, provider.SignalProbability,
                        mvaConfig='MVAConfig_' + channel.name,
                        particleList='RawParticleList_' + channel.name,
                        inverseSamplingRates='InverseSamplingRate_' + channel.name,
                        Nbins='Nbins_' + channel.name,
                        configFilename='TrainedMVC_' + channel.name)

                # Train additional sPlot training to test sPlot
                # TODO add command line argument splot, which switches completly to splot if available
                if particle.mvaConfig.model is not None:
                    # Generate training data from raw ParticleList using Splot
                    dag.add('SPlotModel_' + channel.name, provider.GenerateSPlotModel,
                            mvaConfig='MVAConfig_' + channel.name,
                            name='Name_' + channel.name,
                            distribution='PreCut_' + channel.name)
                    dag.add('SPlotTrainingData_' + channel.name, provider.GenerateTrainingDataUsingSPlot,
                            mvaConfig='MVAConfig_' + channel.name,
                            particleList='RawParticleList_' + channel.name,
                            sPlotParameters='SPlotModel_' + channel.name,
                            Nbins='Nbins_' + channel.name,
                            additionalDependencies=additionalDependencies)
                    dag.add('SPlotTrainedMVC_' + channel.name, provider.TrainMultivariateClassifier,
                            mvaConfig='MVAConfig_' + channel.name,
                            Nbins='Nbins_' + channel.name,
                            trainingData='SPlotTrainingData_' + channel.name)
                    dag.addNeeded('SPlotTrainedMVC_' + channel.name)
                else:
                    dag.add('SPlotModel_' + channel.name, None)
                    dag.add('SPlotTrainingData_' + channel.name, None)
                    dag.add('SPlotTrainedMVC_' + channel.name, None)

            # Create common SignalProbability Resource for this particle, thereby its easy to state the dependency
            # on the SignalProbability of a daughter particle in the line above!
            dag.add('SignalProbability_' + particle.identifier, provider.HashRequirements,
                    ['SignalProbability_' + channel.name for channel in particle.channels])

        # Add the same Resource for the charge conjugated particle
        if particle.name != pdg.conjugate(particle.name):
            dag.add('SignalProbability_' + particle.charge_conjugated_identifier, 'SignalProbability_' + particle.identifier)

    # Search all final particles (particles which don't appear as a daughter particle in a channel of any other particle)
    finalParticles = [particle for particle in particles if all(
                      particle.identifier not in o.daughters and
                      pdg.conjugate(particle.name) + ':' + particle.label not in o.daughters for o in particles)]

    # Write out additional information, histograms, ...
    dag.add('mcCounts', provider.CountMCParticles,
            'gearbox',
            names=['Name_' + particle.identifier for particle in particles])

    dag.add('ModuleStatisticsFile', provider.SaveModuleStatistics,
            ['SignalProbability_' + finalParticle.identifier for finalParticle in finalParticles])

    for particle in particles:
        # Tag unique signal candidates, to avoid double-counting
        # e.g. B->D* pi and B->D pi pi can contain the same correctly reconstructed candidate
        extraVars = ['None']
        if particle in finalParticles:
            dag.add('TagUniqueSignal_' + particle.identifier, provider.TagUniqueSignal,
                    'gearbox',
                    particleList='ParticleList_' + particle.identifier,
                    signalProbability='SignalProbability_' + particle.identifier,
                    target='MVAConfigTarget_' + particle.identifier)
            extraVars = ['TagUniqueSignal_' + particle.identifier]

        dag.add('VariablesToNTuple_' + particle.identifier, provider.VariablesToNTuple,
                'gearbox',
                particleList='ParticleList_' + particle.identifier,
                signalProbability='SignalProbability_' + particle.identifier,
                extraVars=extraVars,
                target='MVAConfigTarget_' + particle.identifier)

    # Create the automatic reporting summary pdf
    if args.summary:
        for particle in particles:
            for channel in particle.channels:
                dag.add('Placeholders_' + channel.name, provider.WriteAnalysisFileForChannel,
                        particleName='Name_' + particle.identifier,
                        particleLabel='Label_' + particle.identifier,
                        channelName='Name_' + channel.name,
                        preCutConfig='PreCutConfig_' + particle.identifier,
                        preCut='PreCut_' + channel.name,
                        preCutHistogram='PreCutHistogram_' + channel.name,
                        mvaConfig='MVAConfig_' + channel.name,
                        tmvaTraining='TrainedMVC_' + channel.name,
                        splotTraining='SPlotTrainedMVC_' + channel.name,
                        postCutConfig='PostCutConfig_' + particle.identifier,
                        postCut='PostCut_' + particle.identifier)

            if particle.isFSP:
                dag.add('Placeholders_' + particle.identifier, provider.WriteAnalysisFileForFSParticle,
                        particleName='Name_' + particle.identifier,
                        particleLabel='Label_' + particle.identifier,
                        mvaConfig='MVAConfig_' + particle.identifier,
                        tmvaTraining='TrainedMVC_' + particle.identifier,
                        postCutConfig='PostCutConfig_' + particle.identifier,
                        postCut='PostCut_' + particle.identifier,
                        distribution='Distribution_' + particle.identifier,
                        nTuple='VariablesToNTuple_' + particle.identifier)
            else:
                dag.add('Placeholders_' + particle.identifier, provider.WriteAnalysisFileForCombinedParticle,
                        particleName='Name_' + particle.identifier,
                        particleLabel='Label_' + particle.identifier,
                        channelPlaceholders=['Placeholders_' + channel.name for channel in particle.channels],
                        nTuple='VariablesToNTuple_' + particle.identifier)

        # get channelName and corresponding inputList and placeholders (in separate lists, sadly)
        channelNames = ['Name_' + p.identifier for p in particles if p.isFSP]
        channelNames += ['Name_' + channel.name for p in particles for channel in p.channels]
        inputLists = ['RawParticleList_' + p.identifier for p in particles if p.isFSP]
        inputLists += ['RawParticleList_' + channel.name for p in particles for channel in p.channels]
        channelPlaceholders = ['Placeholders_' + p.identifier for p in particles if p.isFSP]
        channelPlaceholders += ['Placeholders_' + channel.name for p in particles for channel in p.channels]
        dag.add('CPUTimeSummary', provider.WriteCPUTimeSummary,
                channelNames=channelNames,
                inputLists=inputLists,
                channelPlaceholders=channelPlaceholders,
                mcCounts='mcCounts',
                moduleStatisticsFile='ModuleStatisticsFile')

        dag.add('FEISummary.pdf', provider.WriteAnalysisFileSummary,
                finalStateParticlePlaceholders=['Placeholders_' + p.identifier for p in particles if p.isFSP],
                combinedParticlePlaceholders=['Placeholders_' + p.identifier for p in particles if not p.isFSP],
                finalParticleNTuples=['VariablesToNTuple_' + finalParticle.identifier for finalParticle in finalParticles],
                finalParticleTargets=['MVAConfigTarget_' + finalParticle.identifier for finalParticle in finalParticles],
                cpuTimeSummaryPlaceholders='CPUTimeSummary',
                mcCounts='mcCounts',
                particles=['Object_' + particle.identifier for particle in particles])
        dag.addNeeded('FEISummary.pdf')

    # Finally we add the final particles (normally B+ B0) as needed to the dag
    for finalParticle in finalParticles:
        dag.addNeeded('SignalProbability_' + finalParticle.identifier)
        dag.addNeeded('ParticleList_' + finalParticle.identifier)

    fei_path = create_path()

    is_first_run = args.cache is None or not os.path.isfile(args.cache)
    if args.cache is not None:
        dag.load_cached_resources(args.cache)
    finished_training = dag.run(fei_path)
    if args.cache is not None:
        dag.save_cached_resources(args.cache)

    path = create_path()
    if selection_path is None:
        path.add_module('RootInput')

    if finished_training:
        if selection_path is not None:
            path.add_path(selection_path)
        path.add_path(fei_path)
    else:
        fei_path.add_module("RootOutput")
        if is_first_run and selection_path is not None:
            path.add_path(selection_path)
            path.for_each('RestOfEvent', 'RestOfEvents', fei_path)
        else:
            if selection_path is not None:
                path.add_module('RootInput')
            path.add_path(fei_path)

    if args.cache is not None and not is_first_run:
        # when preloader is used, make sure we also reload the statistics
        for module in path.modules():
            if module.type() == 'RootInput':
                module.param('excludeBranchNamesPersistent', [])

    # with RestOfEvent path, this will be the first module inside for_each
    path.add_module('ProgressBar')
    return FeiState(path, is_trained=finished_training)
