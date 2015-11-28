#!/usr/bin/env python3
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
from modularAnalysis import applyCuts, buildRestOfEvent
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
    parser.add_argument('-nThreads', '--nThreads', dest='nThreads', type=int, default=1, help='Number of threads')
    parser.add_argument('-dump-path', '--dump-path', dest='dumpPath', action='store_true',
                        help='Set if you want to dump the path for analysis usage')
    parser.add_argument('-cache', '--cache', dest='cache', type=str, default=None,
                        help='Use the given file to cache results between multiple executions.'
                             'Data from previous runs has to be provided as input!')
    parser.add_argument('-externTeacher', '--externTeacher', dest='externTeacher', type=str, default='externTeacher',
                        help='Use this command to invoke extern teacher: externTeacher or externClusterTeacher')
    parser.add_argument('-prune', '--prune', dest='prune', action='store_true',
                        help='Prune ParticleStoreArray and ParticleLists')
    parser.add_argument('-rerunCached', '--rerunCached', dest='rerunCachedProviders', action='store_true',
                        help='Runs cached providers again')
    args = parser.parse_args()
    return args


# Define classes at top level to make them pickable
# Creates new classs via namedtuple, which are like a struct in C
# TODO: with python 3, __doc__ is writable for this things, so we can actually set it
MVAConfiguration = collections.namedtuple('MVAConfiguration', 'name, type, config, variables, target, model')
MVAConfiguration.__new__.__defaults__ = ('FastBDT', 'Plugin',
                                         '!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
                                         '', 'isSignal', None)
# Pre-classifier configuration. Note that purity takes precedence over efficiency.
PreCutConfiguration = collections.namedtuple('PreCutConfiguration', 'variable, binning, efficiency, purity')
PreCutConfiguration.__new__.__defaults__ = ('M', (1000, 0, 6), None, None)

UserCutConfiguration = collections.namedtuple('UserCutConfiguration', 'userCut, vertexCut')
UserCutConfiguration.__new__.__defaults__ = ('', -2)

# Adds a 'extraInfo(SignalProbability) > value' cut to a particle.
PostCutConfiguration = collections.namedtuple('PostCutConfiguration', 'value')
PostCutConfiguration.__new__.__defaults__ = (0.0,)

DecayChannel = collections.namedtuple('DecayChannel', 'name, daughters, mvaConfig, userCutConfig, decayModeID')
PlotConfiguration = collections.namedtuple('PlotConfiguration', 'Diagonal, ROC, M, Mbc, CosBDl, Correlation')


class Particle(object):

    """
    The Particle class is the only class the end-user gets into contact with.
    The user creates an instance of this class for every particle he wants to reconstruct with the FEI algorithm,
    and provides MVAConfiguration, PreCutConfiguration and PostCutConfiguration. These can be overwritten per channel.
    """

    def __init__(self, identifier, mvaConfig, preCutConfig=PreCutConfiguration(), userCutConfig=UserCutConfiguration(),
                 postCutConfig=PostCutConfiguration()):
        """
        Creates a Particle without any decay channels. To add decay channels use addChannel method.
            @param identifier is the pdg name of the particle as a string
                   with an optional additional user label seperated by ':'
            @param mvaConfig multivariate analysis configuration
            @param preCutConfig intermediate pre cut configuration
            @param userCutConfig user defined cuts
            @param postCutConfig post cut configuration
        """
        #: pdg name of the particle with an optional additional user label seperated by :
        self.identifier = identifier + ':generic' if len(identifier.split(':')) < 2 else identifier
        v = self.identifier.split(':')
        #: The name of the particle as correct pdg name e.g. K+, pi-, D*0.
        self.name = v[0]
        #: Additional label like hasMissing or has2Daughters
        self.label = v[1]
        #: multivariate analysis configuration (see MVAConfiguration)
        self.mvaConfig = mvaConfig
        #: DecayChannel objects added by addChannel() method.
        self.channels = []
        #: intermediate cut configuration (see PreCutConfiguration)
        self.preCutConfig = preCutConfig
        #: user defined cut configuration (see UserCutConfiguration)
        self.userCutConfig = userCutConfig
        #: post cut configuration (see PostCutConfiguration)
        self.postCutConfig = postCutConfig

    @property
    def charge_conjugated_identifier(self):
        """ Property returning the identifier of the charge conjugated identifier """
        return pdg.conjugate(self.name) + ':' + self.label

    @property
    def daughters(self):
        """ Property returning list of unique daughter particles of all channels """
        return list(frozenset([daughter for channel in self.channels for daughter in channel.daughters]))

    def addChannel(self, daughters, mvaConfig=None, userCutConfig=None):
        """
        Appends a new decay channel to the Particle object.
            @param daughters is a list of pdg particle names e.g. ['pi+','K-']
            @param mvaConfig multivariate analysis configuration
        """
        # Append generic label to all defined daughters if no label was set yet
        daughters = [d + ':generic' if ':' not in d else d for d in daughters]
        # Use default mvaConfig of this particle if no channel-specific config is given
        mvaConfig = copy.deepcopy(self.mvaConfig if mvaConfig is None else mvaConfig)
        # Use default userCutConfig of this particle if no channel-specific config is given
        userCutConfig = copy.deepcopy(self.userCutConfig if userCutConfig is None else userCutConfig)
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
                mvaVars += [v.format(*c) for c in itertools.combinations(list(range(0, len(daughters))), v.count('{}'))]
        mvaConfig = mvaConfig._replace(variables=mvaVars)
        # Add new channel
        self.channels.append(DecayChannel(name=self.identifier + ' ==> ' + ' '.join(daughters),
                                          daughters=daughters,
                                          mvaConfig=mvaConfig,
                                          userCutConfig=userCutConfig,
                                          decayModeID=len(self.channels)))
        return self


# Simple object containing the output of fei
FeiState = collections.namedtuple('FeiState', 'path, is_trained')


def fullEventInterpretation(signalParticleList, selection_path, particles):
    """
    The Full Event Interpretation algorithm has to be executed multiple times, because of the dependencies between
    the MVCs among each other and PreCuts on Histograms.
    These dependencies are automatically solved and a basf2 path is returned containing all needed modules for this stage.
        @param signalParticleList name of ParticleList containing the signal-candidates of the signal-side,
               Use None to perform independent tag-side reconstruction (equivalent to old Belle I Full Reconstruction).
        @param selection_path basf2 module path to execute before any tag-side reconstruction.
               The path should load the data and perform skimming and belle-I conversion if needed
               In addition it should select a signal-side B and create a 'RestOfEvents' list if signalParticleList is not None
        @param particles list of particle objects which shall be reconstructed by this algorithm
        @return FeiState object containing basf2 path to execute, plus status information
    """
    args = getCommandLineOptions()

    # Create a new directed acyclic graph
    dag = dagFramework.DAG()

    # Set environment variables
    dag.env['ROE'] = str(signalParticleList) if signalParticleList is not None and not args.dumpPath else False
    dag.env['prune'] = args.prune
    dag.env['verbose'] = args.verbose
    dag.env['nThreads'] = args.nThreads
    dag.env['rerunCachedProviders'] = args.rerunCachedProviders
    dag.env['externTeacher'] = args.externTeacher

    # Add FSP
    dag.add('ParticleList_e+:FSP', 'e+:FSP')
    dag.add('ParticleList_mu+:FSP', 'mu+:FSP')
    dag.add('ParticleList_pi+:FSP', 'pi+:FSP')
    dag.add('ParticleList_K+:FSP', 'K+:FSP')
    dag.add('ParticleList_K_S0:V0', 'K_S0:V0')
    dag.add('ParticleList_K_L0:FSP', 'K_L0:FSP')
    dag.add('ParticleList_p+:FSP', 'p+:FSP')
    dag.add('ParticleList_gamma:FSP', 'gamma:FSP')
    dag.add('ParticleList_gamma:V0', 'gamma:V0')
    dag.add('VertexFit_e+:FSP', 'e+:FSP')
    dag.add('VertexFit_mu+:FSP', 'mu+:FSP')
    dag.add('VertexFit_pi+:FSP', 'pi+:FSP')
    dag.add('VertexFit_K+:FSP', 'K+:FSP')
    dag.add('VertexFit_K_S0:V0', 'K_S0:V0')
    dag.add('VertexFit_K_L0:FSP', 'K_L0:FSP')
    dag.add('VertexFit_p+:FSP', 'p+:FSP')
    dag.add('VertexFit_gamma:FSP', 'gamma:FSP')
    dag.add('VertexFit_gamma:V0', 'gamma:V0')

    # Add basic properties defined by the user of all Particles as Resources into the graph
    for particle in particles:
        dag.add('Name_' + particle.identifier, particle.name)
        dag.add('Label_' + particle.identifier, particle.label)
        dag.add('Identifier_' + particle.identifier, particle.identifier)
        dag.add('Object_' + particle.identifier, particle)
        dag.add('MVAConfig_' + particle.identifier, particle.mvaConfig)
        dag.add('MVATarget_' + particle.identifier, particle.mvaConfig.target)
        dag.add('PreCutConfig_' + particle.identifier, particle.preCutConfig)
        dag.add('UserCutConfig_' + particle.identifier, particle.userCutConfig)
        dag.add('UserCut_' + particle.identifier, particle.userCutConfig.userCut)
        dag.add('PostCutConfig_' + particle.identifier, particle.postCutConfig)

        for channel in particle.channels:
            dag.add('Name_' + channel.name, channel.name)
            dag.add('Mother_' + channel.name, particle.identifier)
            dag.add('MVAConfig_' + channel.name, channel.mvaConfig)
            dag.add('MVATarget_' + channel.name, channel.mvaConfig.target)
            dag.add('UserCutConfig_' + channel.name, channel.userCutConfig)
            dag.add('UserCut_' + channel.name, channel.userCutConfig.userCut)
            dag.add('VertexCut_' + channel.name, channel.userCutConfig.vertexCut)
            dag.add('DecayModeID_' + channel.name, channel.decayModeID)

    # Add Modules which are always needed
    dag.add('gearbox', provider.LoadGearbox)
    dag.add('geometry', provider.LoadGeometry, 'gearbox')
    dag.add('particles', provider.LoadParticles)

    # Reconstruct given particle topology
    for particle in particles:
        for channel in particle.channels:
            dag.add('RawParticleList_' + channel.name, provider.MakeParticleList,
                    'particles',
                    'Label_' + particle.identifier,
                    particleName='Name_' + particle.identifier,
                    daughterParticleLists=['ParticleList_' + daughter for daughter in channel.daughters],
                    preCut='PreCut_' + channel.name,
                    userCut='UserCut_' + channel.name,
                    decayModeID='DecayModeID_' + channel.name)
            dag.add('MatchedParticleList_' + channel.name, provider.MatchParticleList,
                    particleList='RawParticleList_' + channel.name,
                    mvaTarget='MVATarget_' + channel.name)

        # Copy all channel lists into a single particle list
        dag.add('ParticleList_' + particle.identifier, provider.CopyParticleLists,
                particleName='Name_' + particle.identifier,
                particleLabel='Label_' + particle.identifier,
                particleLists=['RawParticleList_' + channel.name for channel in particle.channels],
                postCut='PostCut_' + particle.identifier,
                signalProbabilities=['SignalProbability_' + channel.name for channel in particle.channels])

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
        for channel in particle.channels:
            if len(channel.daughters) == 1:
                dag.add('VertexFit_' + channel.name, 'TrackFitIsAlreadyDoneForFSPs')
            else:
                dag.add('VertexFit_' + channel.name, provider.FitVertex,
                        'geometry',
                        ['VertexFit_' + daughter for daughter in channel.daughters],
                        channelName='Name_' + channel.name,
                        particleList='RawParticleList_' + channel.name,
                        vertexCut='VertexCut_' + channel.name)
        # Create common VertexFit Resource for this particle, thereby its easy to state the dependency
        # on the VertexFit of a daughter particle in the line above!
        dag.add('VertexFit_' + particle.identifier, provider.HashRequirements,
                ['VertexFit_' + channel.name for channel in particle.channels])

        # Add the same Resource for the charge conjugated particle
        if particle.name != pdg.conjugate(particle.name):
            dag.add('VertexFit_' + particle.charge_conjugated_identifier, 'VertexFit_' + particle.identifier)

    # Determine Intermediate Cuts using the config objects provided by the user
    for particle in particles:
        # Determine PreCut
        for channel in particle.channels:
            additionalDependencies = []
            if particle.preCutConfig is not None and 'SignalProbability' in particle.preCutConfig.variable:
                additionalDependencies = ['SignalProbability_' + daughter for daughter in channel.daughters]
            dag.add('PreCutHistogram_' + channel.name, provider.CreatePreCutHistogram,
                    'particles',
                    particleName='Name_' + particle.identifier,
                    channelName='Name_' + channel.name,
                    mvaTarget='MVATarget_' + channel.name,
                    preCutConfig='PreCutConfig_' + particle.identifier,
                    userCut='UserCut_' + channel.name,
                    daughterParticleLists=['ParticleList_' + daughter for daughter in channel.daughters],
                    additionalDependencies=additionalDependencies)

            dag.add('PreCut_' + channel.name, provider.PreCutDeterminationPerChannel,
                    channelName='Name_' + channel.name,
                    preCut='PreCut_' + particle.identifier)

        dag.add('PreCut_' + particle.identifier, provider.PreCutDetermination,
                channelNames=['Name_' + channel.name for channel in particle.channels],
                preCutConfig='PreCutConfig_' + particle.identifier,
                preCutHistograms=['PreCutHistogram_' + channel.name for channel in particle.channels])

        # Determine PostCut
        dag.add('PostCut_' + particle.identifier, provider.PostCutDetermination,
                'Identifier_' + particle.identifier,
                ['SignalProbability_' + channel.name for channel in particle.channels],
                postCutConfig='PostCutConfig_' + particle.identifier)

    # Trains multivariate classifiers (MVC) methods and provides signal probabilities
    for particle in particles:
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
                    additionalDependencies=additionalDependencies)

            # Train and apply MVC on raw ParticleList
            dag.add('TrainedMVC_' + channel.name, provider.TrainMultivariateClassifier,
                    mvaConfig='MVAConfig_' + channel.name,
                    Nbins='Nbins_' + channel.name,
                    trainingData='TrainingData_' + channel.name)
            dag.add('SignalProbability_' + channel.name, provider.SignalProbability,
                    mvaConfig='MVAConfig_' + channel.name,
                    particleList='RawParticleList_' + channel.name,
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
                        trainingData='SPlotTrainingData_' + channel.name,
                        inverseSamplingRates='InverseSamplingRate_' + channel.name)
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

    dag.add('listCounts', provider.CountParticleLists,
            targets=['MVATarget_' + channel.name for particle in particles for channel in particle.channels],
            lists=['MatchedParticleList_' + channel.name for particle in particles for channel in particle.channels])

    dag.add('ModuleStatisticsFile', provider.SaveModuleStatistics,
            ['SignalProbability_' + finalParticle.identifier for finalParticle in finalParticles])

    for particle in particles:
        # Tag unique signal candidates, to avoid double-counting
        # e.g. B->D* pi and B->D pi pi can contain the same correctly reconstructed candidate
        dag.add('TagUniqueSignal_' + particle.identifier, provider.TagUniqueSignal,
                'gearbox',
                particleList='ParticleList_' + particle.identifier,
                signalProbability='SignalProbability_' + particle.identifier,
                target='MVATarget_' + particle.identifier)
        dag.add('VariablesToNTuple_' + particle.identifier, provider.VariablesToNTuple,
                'gearbox',
                'TagUniqueSignal_' + particle.identifier,
                particleList='ParticleList_' + particle.identifier,
                signalProbability='SignalProbability_' + particle.identifier,
                target='MVATarget_' + particle.identifier)
        dag.addNeeded('VariablesToNTuple_' + particle.identifier)

    dag.add('SaveSummary', provider.SaveSummary,
            mcCounts='mcCounts',
            listCounts='listCounts',
            moduleStatistics='ModuleStatisticsFile',
            particles=['Object_' + particle.identifier for particle in particles],
            ntuples=['VariablesToNTuple_' + particle.identifier for particle in particles],
            preCuts=['PreCut_' + particle.identifier for particle in particles],
            postCuts=['PostCut_' + particle.identifier for particle in particles],
            plists=['ParticleList_' + particle.identifier for particle in particles],
            clists=['RawParticleList_' + channel.name for particle in particles for channel in particle.channels],
            mlists=['MatchedParticleList_' + channel.name for particle in particles for channel in particle.channels],
            mothers=['Mother_' + channel.name for particle in particles for channel in particle.channels],
            cnames=['Name_' + channel.name for particle in particles for channel in particle.channels],
            preCutHistograms=['PreCutHistogram_' + channel.name for particle in particles for channel in particle.channels],
            inverseSamplingRates=['InverseSamplingRate_' + channel.name for particle in particles for channel in particle.channels],
            trainingData=['TrainingData_' + channel.name for particle in particles for channel in particle.channels])

    # Finally we add the final particles (normally B+ B0) as needed to the dag
    for finalParticle in finalParticles:
        dag.addNeeded('SignalProbability_' + finalParticle.identifier)
        dag.addNeeded('ParticleList_' + finalParticle.identifier)
        dag.addNeeded('HumanReadableParticleList_' + finalParticle.identifier)
    dag.addNeeded('SaveSummary')

    fei_path = create_path()

    is_first_run = args.cache is None or not os.path.isfile(args.cache)
    if args.cache is not None:
        dag.load_cached_resources(args.cache)
    finished_training = dag.run(fei_path)

    if args.cache is not None:
        dag.save_cached_resources(args.cache)

    path = create_path()

    if finished_training:
        path.add_path(selection_path)
        path.add_path(fei_path)
        return FeiState(path, is_trained=True)

    fei_path.add_module("RootOutput")
    if is_first_run:
        path.add_path(selection_path)
        if signalParticleList:
            # TODO SignalMC with no correct signal-candidate will be still used as background component
            # We have to fix this cut! (Other FEI-UserCuts should be fine, as soon as this one is fixed)
            # Although the cut is not 100% correct at the moment, it still a big step in the right direction.
            cut = 'isSignalAcceptMissingNeutrino == 1'
            cut += ' or eventCached(countInList(' + dag.env['ROE'] + ', isSignalAcceptMissingNeutrino == 1)) == 0'
            applyCuts(signalParticleList, cut, path=path)
            buildRestOfEvent(signalParticleList, path=path)
            path.for_each('RestOfEvent', 'RestOfEvents', fei_path)
        else:
            path.add_path(fei_path)
    else:
        path.add_module('RootInput', excludeBranchNamesPersistent=[])
        path.add_path(fei_path)

    # with RestOfEvent path, this will be the first module inside for_each
    path.add_module('ProgressBar')
    return FeiState(path, is_trained=False)
