#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Thomas Keck 2014
#
# This FullEventInterpretation algorithm uses a functional approach (see actorFramework.py).
# All tasks are implemented in Functions called actors.
# Each Actor has requirements and provides return values.
# E.g. SignalProbability requires the path, a method, variables and a ParticleList.
#                        provides SignalProbability for the ParticleList.
#      CreatePreCutHistogram require among others the SignalProbability for the ParticleLists
#                        provides PreCutHistogram
#      PreCutDetermination requires among others a PreCutHistogram
#                         provides PreCuts
#      MakeAndMatchParticleList requires among others PreCuts
# ... and so on ...
#
# The actors are added to the Sequence of actors in the FullEventInterpretation function at the end of this file.
# Afterwards the dependencies between the Actors are solved, and the Actors are called in the correct order, with the required parameters.
# If no further Actors can be called without satisfying all their requirements, the FullReoncstruction function returns.
# Therefore the end user has to run the FullEventInterpretation several times, until all Histograms, Classifiers, ... are created.
#

from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = True
PyConfig.StartGuiThread = False
import ROOT

import pdg

import actorFramework
from basf2 import *
from actorFunctions import *

import collections
import argparse
import copy
import os


class Particle(object):
    """
    The Particle class is the only class the end-user gets into contact with.
    The user creates an instance of this class for every particle he wants to reconstruct with the FullReconstruction algorithm,
    and provides MVAConfiguration, PreCutConfiguration and PostCutConfiguration. These configurations can be overwritten per channel.
    """

    ## Create new class called MVAConfiguration via namedtuple. namedtuples are like a struct in C
    MVAConfiguration = collections.namedtuple('MVAConfiguration', 'name, type, config, variables, target')
    ## Create new class called PreCutConfiguration via namedtuple. namedtuples are like a struct in C
    PreCutConfiguration = collections.namedtuple('PreCutConfiguration', 'variable, binning, efficiency, purity, userCut')
    ## Create new class called PostCutConfiguration via namedtuple. namedtuples are like a struct in C
    PostCutConfiguration = collections.namedtuple('PostCutConfiguration', 'value')
    ## Create new class called DecayChannel via namedtuple. namedtuples are like a struct in C
    DecayChannel = collections.namedtuple('DecayChannel', 'name, daughters, mvaConfig, preCutConfig')

    def __init__(self, identifier, mvaConfig, preCutConfig=None, postCutConfig=None):
        """
        Creates a Particle without any decay channels. To add decay channels use addChannel method.
            @param identifier is the correct pdg name of the particle as a string with an optional additional user label seperated by :
            @param mvaConfig multivariate analysis configuration
            @param preCutConfig intermediate pre cut configuration
            @param postCutConfig post cut configuration
        """
        ## Is the correct pdg name as a string of the particle with an optional additional user label seperated by :
        self.identifier = identifier + ':generic' if len(identifier.split(':')) < 2 else identifier
        v = self.identifier.split(':')
        ## The name of the particle as correct pdg name e.g. K+, pi-, D*0.
        self.name = v[0]
        ## Additional label like hasMissing or has2Daughters
        self.label = v[1]
        ## multivariate analysis configuration (see MVAConfiguration)
        self.mvaConfig = mvaConfig
        ## DecayChannel objects added by addChannel() method.
        self.channels = []
        ## intermediate cut configuration (see PreCutConfiguration)
        self.preCutConfig = preCutConfig
        ## post cut configuration (see PostCutConfiguration)
        self.postCutConfig = postCutConfig

    @property
    def daughters(self):
        """ Property which returns all daughter particles of all channels """
        return list(frozenset([daughter for channel in self.channels for daughter in channel.daughters]))

    @property
    def isFSP(self):
        """ Returns true if the particle is a final state particle """
        return self.channels == []

    def addChannel(self, daughters, mvaConfig=None, preCutConfig=None):
        """
        Appends a new decay channel to the Particle object.
        @param daughters is a list of pdg particle names e.g. ['pi+','K-']
        @param mvaConfig multivariate analysis configuration
        @param preCutConfig intermediate pre cut configuration
        """
        daughters = [d + ':generic' if ':' not in d else d for d in daughters]
        preCutConfig = copy.deepcopy(self.preCutConfig if preCutConfig is None else preCutConfig)
        mvaConfig = copy.deepcopy(self.mvaConfig if mvaConfig is None else mvaConfig)
        mvaConfig.variables.extend(['daughter({i}, getExtraInfo(SignalProbability))'.format(i=i) for i in range(0, len(daughters))])
        self.channels.append(Particle.DecayChannel(name=self.identifier + ' ==> ' + ' '.join(daughters),
                                                   daughters=daughters,
                                                   mvaConfig=mvaConfig,
                                                   preCutConfig=preCutConfig))
        return self

    def __str__(self):
        """ Convert particle object in a readable string which contains all configuration informations """
        output = '{identifier}\n'.format(identifier=self.identifier)

        def compareMVAConfig(x, y):
            return x.name == y.name and x.type == y.type and x.config == y.config and x.target == y.target

        def compareCutConfig(x, y):
            if x is None and y is None:
                return True
            return x == y

        if self.isFSP:
            if self.postCutConfig is None:
                output += '    PostCutConfiguration: None\n'
            else:
                output += '    PostCutConfiguration: value={p.value}\n'.format(p=self.postCutConfig)
            output += '    MVAConfiguration: name={m.name}, type={m.type}, config={m.config}, target={m.target}\n'.format(m=self.mvaConfig)
            output += '    Variables: ' + ', '.join(self.mvaConfig.variables) + '\n'
        else:
            samePreCutConfig = all(compareCutConfig(channel.preCutConfig, self.preCutConfig) for channel in self.channels)
            sameMVAConfig = all(compareMVAConfig(channel.mvaConfig, self.mvaConfig) for channel in self.channels)
            commonVariables = reduce(lambda x, y: set(x).intersection(y), [channel.mvaConfig.variables for channel in self.channels])
            if sameMVAConfig:
                output += '    All channels use the same MVA configuration\n'
                output += '    MVAConfiguration: name={m.name}, type={m.type}, target={m.target}, config={m.config}\n'.format(m=self.mvaConfig)
            output += '    Shared Variables: ' + ', '.join(commonVariables) + '\n'

            if samePreCutConfig:
                output += '    All channels use the same PreCut configuration\n'
                output += '    PreCutConfiguration: variables={p.variable}, efficiency={p.efficiency}, purity={p.purity}\n'.format(p=self.preCutConfig)
                output += '    PreCutConfiguration: binning={p.binning}\n'.format(p=self.preCutConfig)
                output += '    PreCutConfiguration: userCut={p.userCut}\n'.format(p=self.preCutConfig)

            if self.postCutConfig is None:
                output += '    PostCutConfiguration: None\n'
            else:
                output += '    PostCutConfiguration: value={p.value}\n'.format(p=self.postCutConfig)

            for channel in self.channels:
                output += '    {name}\n'.format(name=channel.name)
                if not samePreCutConfig:
                    if self.postCutConfig is None:
                        output += '    PreCutConfiguration: None\n'
                    else:
                        output += '    PreCutConfiguration: variable={p.variable}, efficiency={p.efficiency}, purity={p.purity}\n'.format(p=channel.preCutConfig)
                        output += '    PreCutConfiguration: binning={p.binning}\n'.format(p=channel.preCutConfig)
                        output += '    PreCutConfiguration: userCut={p.userCut}\n'.format(p=channel.preCutConfig)
                if not sameMVAConfig:
                    output += '    MVAConfiguration: name={m.name}, type={m.type}, config={m.config}, target={m.target}\n'.format(m=channel.mvaConfig)
                output += '        Individual Variables: ' + ', '.join(set(channel.mvaConfig.variables).difference(commonVariables)) + '\n'
        return output


def FullEventInterpretation(user_selection_path, user_analysis_path, particles):
    """
    The Full Event Interpretation algorithm.
    Alle the Actors defined above are added to the playuence and are executed in an order which fulfills all requirements.
    This function returns if no more Actors can be called without violating some requirements.
        @param user_selection_path basf2 module path to execute before any tag-side reconstruction. Should load data, select signal side and create a 'RestOfEvents' list
        @param user_analysis_path basf2 module path to execute after training is finished
        @param particles playuence of particle objects which shall be reconstructed by this algorithm
    """

    parser = argparse.ArgumentParser()
    parser.add_argument('-verbose', '--verbose', dest='verbose', action='store_true', help='Output additional information')
    parser.add_argument('-summary', '--make-summmary', dest='makeSummary', action='store_true', help='Create Summary pdf')
    parser.add_argument('-nosignal', '--no-signal-classifiers', dest='nosig', action='store_true', help='Do not train classifiers')
    parser.add_argument('-nproc', '--nProcesses', dest='nProcesses', type=int, default=1, help='Use n processes to execute actors parallel')
    parser.add_argument('-preload', '--preload', dest='preload', action='store_true', help='Preload stuf from cache file')
    parser.add_argument('-cache', '--cache', dest='cacheFile', type=str, default=None, help='Cache File')
    args = parser.parse_args()

    # Add the basf2 module path
    play = actorFramework.Play()

    # Than add the properties of the particles to the play
    for particle in particles:
        play.addProperty('Name_{i}'.format(i=particle.identifier), particle.name)
        play.addProperty('Label_{i}'.format(i=particle.identifier), particle.label)
        play.addProperty('Identifier_{i}'.format(i=particle.identifier), particle.identifier)
        play.addProperty('Object_{i}'.format(i=particle.identifier), particle)
        play.addProperty('PostCutConfig_{i}'.format(i=particle.identifier), particle.postCutConfig)

        if particle.isFSP:
            play.addProperty('MVAConfig_{i}'.format(i=particle.identifier), particle.mvaConfig)
            play.addProperty('MVAConfigTarget_{i}'.format(i=particle.identifier), particle.mvaConfig.target)
            play.addProperty('VertexFit_{i}'.format(i=particle.identifier), 'TrackFitIsAlreadyDoneForFSPs')
        for channel in particle.channels:
            play.addProperty('Name_{c}'.format(c=channel.name), channel.name)
            play.addProperty('MVAConfig_{c}'.format(c=channel.name), channel.mvaConfig)
            play.addProperty('MVAConfigTarget_{c}'.format(c=channel.name), channel.mvaConfig.target)
            play.addProperty('PreCutConfig_{c}'.format(c=channel.name), channel.preCutConfig)

    # Add top-level actors
    play.addActor(CountMCParticles, names=['Name_{i}'.format(i=particle.identifier) for particle in particles])
    play.addActor(LoadGearbox)
    play.addActor(LoadGeometry)

    if user_selection_path is None:
        play.addActor(LoadParticles)
        play.addProperty('runs_in_ROE', False)
    else:
        play.addProperty('particleLoader', str(serialize_path(user_selection_path)))
        play.addProperty('runs_in_ROE', True)

    # In the first act we reconstruct the particles
    for particle in particles:
        if particle.isFSP:
            play.addActor(SelectParticleList,
                          particleName='Name_{i}'.format(i=particle.identifier),
                          particleLabel='Label_{i}'.format(i=particle.identifier))
            play.addActor(CopyParticleLists,
                          particleName='Name_{i}'.format(i=particle.identifier),
                          particleLabel='Label_{i}'.format(i=particle.identifier),
                          inputLists=['RawParticleList_{i}'.format(i=particle.identifier)],
                          postCut='PostCut_{i}'.format(i=particle.identifier))
        else:
            for channel in particle.channels:
                play.addActor(MakeAndMatchParticleList,
                              particleName='Name_{i}'.format(i=particle.identifier),
                              particleLabel='Label_{i}'.format(i=particle.identifier),
                              channelName='Name_{c}'.format(c=channel.name),
                              daughterParticleLists=['ParticleList_{d}'.format(d=daughter) for daughter in channel.daughters],
                              preCut='PreCut_{c}'.format(c=channel.name))
                play.addActor(FitVertex,
                              channelName='Name_{c}'.format(c=channel.name),
                              particleList='RawParticleList_{c}'.format(c=channel.name),
                              daughterVertices=['VertexFit_{d}'.format(d=daughter) for daughter in channel.daughters])
            play.addCollection('VertexFit_{i}'.format(i=particle.identifier), ['VertexFit_{c}'.format(c=channel.name) for channel in particle.channels])

            play.addActor(CopyParticleLists,
                          particleName='Name_{i}'.format(i=particle.identifier),
                          particleLabel='Label_{i}'.format(i=particle.identifier),
                          inputLists=['RawParticleList_{c}'.format(c=channel.name) for channel in particle.channels],
                          postCut='PostCut_{i}'.format(i=particle.identifier))
        if particle.name != pdg.conjugate(particle.name):
            play.addCollection('VertexFit_{p}:{l}'.format(p=pdg.conjugate(particle.name), l=particle.label), ['VertexFit_{i}'.format(i=particle.identifier)])

    # In the second act pre and post cuts are calculated
    for particle in particles:
        if particle.isFSP:
            play.addActor(FSPDistribution,
                          identifier='Identifier_{i}'.format(i=particle.identifier),
                          inputList='RawParticleList_{i}'.format(i=particle.identifier),
                          mvaConfigTarget='MVAConfigTarget_{i}'.format(i=particle.identifier))
            play.addActor(PostCutDetermination,
                          identifier='Identifier_{i}'.format(i=particle.identifier),
                          postCutConfig='PostCutConfig_{i}'.format(i=particle.identifier),
                          signalProbabilities=['SignalProbability_{i}'.format(i=particle.identifier)])
        else:
            play.addActor(PreCutDetermination,
                          channelNames=['Name_{c}'.format(c=channel.name) for channel in particle.channels],
                          preCutConfigs=['PreCutConfig_{c}'.format(c=channel.name) for channel in particle.channels],
                          preCutHistograms=['PreCutHistogram_{c}'.format(c=channel.name) for channel in particle.channels])
            play.addActor(PostCutDetermination,
                          identifier='Identifier_{i}'.format(i=particle.identifier),
                          postCutConfig='PostCutConfig_{i}'.format(i=particle.identifier),
                          signalProbabilities=['SignalProbability_{c}'.format(c=channel.name) for channel in particle.channels])

            for channel in particle.channels:
                additionalDependencies = []
                if 'SignalProbability' in channel.preCutConfig.variable:
                    additionalDependencies = ['SignalProbability_{d}'.format(d=daughter) for daughter in channel.daughters]

                play.addActor(CreatePreCutHistogram,
                              particleName='Name_{i}'.format(i=particle.identifier),
                              channelName='Name_{c}'.format(c=channel.name),
                              mvaConfigTarget='MVAConfigTarget_{c}'.format(c=channel.name),
                              preCutConfig='PreCutConfig_{c}'.format(c=channel.name),
                              daughterParticleLists=['ParticleList_{d}'.format(d=daughter) for daughter in channel.daughters],
                              additionalDependencies=additionalDependencies)

    # The third act trains multivariate methods and provides signal probabilities
    if not args.nosig:
        for particle in particles:
            if particle.isFSP:
                play.addActor(SignalProbability,
                              identifier='Identifier_{i}'.format(i=particle.identifier),
                              mvaConfig='MVAConfig_{i}'.format(i=particle.identifier),
                              particleList='RawParticleList_{i}'.format(i=particle.identifier),
                              distribution='Distribution_{i}'.format(i=particle.identifier),
                              additionalDependencies='None')
            else:
                for channel in particle.channels:
                    additionalDependencies = []
                    if any('SignalProbability' in variable for variable in channel.mvaConfig.variables):
                        additionalDependencies += ['SignalProbability_{d}'.format(d=daughter) for daughter in channel.daughters]
                    if any(variable in ['dx', 'dy', 'dz', 'dr', 'chiProb', 'significanceOfDistance', 'distance',
                                        'cosAngleBetweenMomentumAndVertexVector'] for variable in channel.mvaConfig.variables):
                        additionalDependencies += ['VertexFit_{c}'.format(c=channel.name)]
                    play.addActor(SignalProbability,
                                  identifier='Name_{c}'.format(c=channel.name),
                                  mvaConfig='MVAConfig_{c}'.format(c=channel.name),
                                  particleList='RawParticleList_{c}'.format(c=channel.name),
                                  distribution='PreCut_{c}'.format(c=channel.name),
                                  additionalDependencies=additionalDependencies)

                play.addCollection('SignalProbability_{i}'.format(i=particle.identifier), ['SignalProbability_{c}'.format(c=channel.name) for channel in particle.channels])

            if particle.name != pdg.conjugate(particle.name):
                play.addCollection('SignalProbability_{p}:{l}'.format(p=pdg.conjugate(particle.name), l=particle.label), ['SignalProbability_{i}'.format(i=particle.identifier)])
            play.addActor(VariablesToNTuple,
                          particleIdentifier='Identifier_{i}'.format(i=particle.identifier),
                          particleList='ParticleList_{i}'.format(i=particle.identifier),
                          signalProbability='SignalProbability_{i}'.format(i=particle.identifier))

    # The last act creates the automatic reporting summary pdf
    finalParticles = [particle for particle in particles if all(particle.identifier not in o.daughters and pdg.conjugate(particle.name) + ':' + particle.label not in o.daughters for o in particles)]
    play.addActor(SaveModuleStatistics,
                  finalParticleSignalProbabilities=['SignalProbability_{i}'.format(i=finalParticle.identifier) for finalParticle in finalParticles])

    if args.makeSummary:
        for particle in particles:
            for channel in particle.channels:
                play.addActor(WriteAnalysisFileForChannel,
                              particleName='Name_{i}'.format(i=particle.identifier),
                              particleLabel='Label_{i}'.format(i=particle.identifier),
                              channelName='Name_{c}'.format(c=channel.name),
                              preCutConfig='PreCutConfig_{c}'.format(c=channel.name),
                              preCut='PreCut_{c}'.format(c=channel.name),
                              preCutHistogram='PreCutHistogram_{c}'.format(c=channel.name),
                              mvaConfig='MVAConfig_{c}'.format(c=channel.name),
                              signalProbability='SignalProbability_{c}'.format(c=channel.name),
                              postCutConfig='PostCutConfig_{i}'.format(i=particle.identifier),
                              postCut='PostCut_{i}'.format(i=particle.identifier))

            if particle.isFSP:
                play.addActor(WriteAnalysisFileForFSParticle,
                              particleName='Name_{i}'.format(i=particle.identifier),
                              particleLabel='Label_{i}'.format(i=particle.identifier),
                              mvaConfig='MVAConfig_{i}'.format(i=particle.identifier),
                              signalProbability='SignalProbability_{i}'.format(i=particle.identifier),
                              postCutConfig='PostCutConfig_{i}'.format(i=particle.identifier),
                              postCut='PostCut_{i}'.format(i=particle.identifier),
                              distribution='Distribution_{i}'.format(i=particle.identifier),
                              nTuple='VariablesToNTuple_{i}'.format(i=particle.identifier))
            else:
                play.addActor(WriteAnalysisFileForCombinedParticle,
                              particleName='Name_{i}'.format(i=particle.identifier),
                              particleLabel='Label_{i}'.format(i=particle.identifier),
                              channelPlaceholders=['Placeholders_{c}'.format(c=channel.name) for channel in particle.channels],
                              nTuple='VariablesToNTuple_{i}'.format(i=particle.identifier))

        #get channelName and corresponding inputList and placeholders (in separate lists, sadly)
        channelNames = ['Name_{i}'.format(i=p.identifier) for p in particles if p.isFSP]
        channelNames += ['Name_{c}'.format(c=channel.name) for p in particles for channel in p.channels]
        inputLists = ['RawParticleList_{i}'.format(i=p.identifier) for p in particles if p.isFSP]
        inputLists += ['RawParticleList_{c}'.format(c=channel.name) for p in particles for channel in p.channels]
        channelPlaceholders = ['Placeholders_{i}'.format(i=p.identifier) for p in particles if p.isFSP]
        channelPlaceholders += ['Placeholders_{c}'.format(c=channel.name) for p in particles for channel in p.channels]
        play.addActor(WriteCPUTimeSummary,
                      channelNames=channelNames,
                      inputLists=inputLists,
                      channelPlaceholders=channelPlaceholders,
                      mcCounts='mcCounts',
                      moduleStatisticsFile='ModuleStatisticsFile')

        play.addActor(WriteAnalysisFileSummary,
                      finalStateParticlePlaceholders=['Placeholders_{i}'.format(i=particle.identifier) for particle in particles if particle.isFSP],
                      combinedParticlePlaceholders=['Placeholders_{i}'.format(i=particle.identifier) for particle in particles if not particle.isFSP],
                      finalParticleNTuples=['VariablesToNTuple_{i}'.format(i=finalParticle.identifier) for finalParticle in finalParticles],
                      cpuTimeSummaryPlaceholders='CPUTimeSummary',
                      mcCounts='mcCounts',
                      particles=['Object_{i}'.format(i=particle.identifier)for particle in particles])
        play.addNeeded('FEIsummary.pdf')

    # Finally we add the final particles (normally B+ B0) as needed to the play
    for finalParticle in finalParticles:
        play.addNeeded('SignalProbability_{i}'.format(i=finalParticle.identifier))
        play.addNeeded('ParticleList_{i}'.format(i=finalParticle.identifier))

    fei_path = create_path()

    is_first_run = args.cacheFile is None or not os.path.isfile(args.cacheFile)
    finished_training = play.run(fei_path, args.verbose, args.cacheFile, args.preload, args.nProcesses)

    path = create_path()
    if user_selection_path is None:
        path.add_module(register_module('RootInput'))

    if finished_training:
        if user_selection_path is not None:
            path.add_path(user_selection_path)
        path.add_path(fei_path)
        if user_analysis_path is not None:
            path.add_path(user_analysis_path)
    else:
        fei_path.add_module(register_module('RootOutput'))
        if is_first_run and user_selection_path is not None:
            path.add_path(user_selection_path)
            path.for_each('RestOfEvent', 'RestOfEvents', fei_path)
        else:
            if user_selection_path is not None:
                path.add_module(register_module('RootInput'))
            path.add_path(fei_path)

    if not is_first_run:
        # when preloader is used, make sure we also reload the statistics
        for module in path.modules():
            if module.type() == 'RootInput':
                module.param('excludeBranchNamesPersistent', [])

    path.add_module(register_module('ProgressBar'))
    return path
