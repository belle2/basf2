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

import pdg

import actorFramework
from actorFunctions import *

import collections
import argparse
import copy


class Particle(object):
    """
    The Particle class is the only class the end-user gets into contact with.
    The user creates an instance of this class for every particle he wants to reconstruct with the FullReconstruction algorithm,
    and provides MVAConfiguration, PreCutConfiguration and PostCutConfiguration. These configurations can be overwritten per channel.
    """

    ## Create new class called MVAConfiguration via namedtuple. namedtuples are like a struct in C
    MVAConfiguration = collections.namedtuple('MVAConfiguration', 'name, type, config, variables, target, targetCluster')
    ## Create new class called PreCutConfiguration via namedtuple. namedtuples are like a struct in C
    PreCutConfiguration = collections.namedtuple('PreCutConfiguration', 'variable, method, efficiency, purity')
    ## Create new class called PostCutConfiguration via namedtuple. namedtuples are like a struct in C
    PostCutConfiguration = collections.namedtuple('PostCutConfiguration', 'value')
    ## Create new class called DecayChannel via namedtuple. namedtuples are like a struct in C
    DecayChannel = collections.namedtuple('DecayChannel', 'name, daughters, mvaConfig, preCutConfig, postCutConfig')

    def __init__(self, identifier, mvaConfig, preCutConfig=PreCutConfiguration(variable='Mass', method='S/B', efficiency=0.70, purity=0.001),
                 postCutConfig=PostCutConfiguration(value=0.0001)):
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
        ## multivariate analysis configuration
        self.mvaConfig = mvaConfig
        ## Decay channels, added by addChannel method.
        self.channels = []
        ## intermediate cut configuration
        self.preCutConfig = preCutConfig
        ## post cut configuration
        self.postCutConfig = postCutConfig

    @property
    def daughters(self):
        """ Property which returns all daughter particles of all channels """
        return list(frozenset([daughter for channel in self.channels for daughter in channel.daughters]))

    @property
    def isFSP(self):
        """ Returns true if the particle is a final state particle """
        return self.channels == []

    def addChannel(self, daughters, mvaConfig=None, preCutConfig=None, postCutConfig=None):
        """
        Appends a new decay channel to the Particle object.
        @param daughters is a list of pdg particle names e.g. ['pi+','K-']
        @param mvaConfig multivariate analysis configuration
        @param preCutConfig intermediate pre cut configuration
        @param postCutConfig post cut configuration
        """
        daughters = [d + ':generic' if ':' not in d else d for d in daughters]
        preCutConfig = copy.deepcopy(self.preCutConfig if preCutConfig is None else preCutConfig)
        postCutConfig = copy.deepcopy(self.postCutConfig if postCutConfig is None else postCutConfig)
        mvaConfig = copy.deepcopy(self.mvaConfig if mvaConfig is None else mvaConfig)
        mvaConfig.variables.extend(['daughter{i}(getExtraInfo(SignalProbability))'.format(i=i) for i in range(0, len(daughters))])
        self.channels.append(Particle.DecayChannel(name=self.identifier + ' ==> ' + ' '.join(daughters),
                                                   daughters=daughters,
                                                   mvaConfig=mvaConfig,
                                                   preCutConfig=preCutConfig,
                                                   postCutConfig=postCutConfig))
        return self

    def __str__(self):
        """ Convert particle object in a readable string which contains all configuration informations """
        output = '{identifier}\n'.format(identifier=self.identifier)

        def compareMVAConfig(x, y):
            return x.name == y.name and x.type == y.type and x.config == y.config and x.target == y.target and x.targetCluster == y.targetCluster

        def compareCutConfig(x, y):
            return x == y

        if self.isFSP:
            output += '    PreCutConfiguration: variable={p.variable}, method={p.method}, efficiency={p.efficiency}, purity={p.purity}\n'.format(p=self.preCutConfig)
            output += '    PostCutConfiguration: value={p.value}\n'.format(p=self.postCutConfig)
            output += '    MVAConfiguration: name={m.name}, type={m.type}, config={m.config}, target={m.target}, targetCluster={m.targetCluster}\n'.format(m=self.mvaConfig)
            output += '    Variables: ' + ', '.join(self.mvaConfig.variables) + '\n'
        else:
            samePreCutConfig = all(compareCutConfig(channel.preCutConfig, self.preCutConfig) for channel in self.channels)
            samePostCutConfig = all(compareCutConfig(channel.postCutConfig, self.postCutConfig) for channel in self.channels)
            sameMVAConfig = all(compareMVAConfig(channel.mvaConfig, self.mvaConfig) for channel in self.channels)
            commonVariables = reduce(lambda x, y: set(x).intersection(y), [channel.mvaConfig.variables for channel in self.channels])
            if sameMVAConfig:
                output += '    All channels use the same MVA configuration\n'
                output += '    MVAConfiguration: name={m.name}, type={m.type}, target={m.target}, targetCluster={m.targetCluster}, config={m.config}\n'.format(m=self.mvaConfig)
            output += '    Shared Variables: ' + ', '.join(commonVariables) + '\n'

            if samePreCutConfig:
                output += '    All channels use the same PreCut configuration\n'
                output += '    PreCutConfiguration: variable={p.variable}, method={p.method}, efficiency={p.efficiency}, purity={p.purity}\n'.format(p=self.preCutConfig)

            if samePostCutConfig:
                output += '    All channels use the same PostCut configuration\n'
                output += '    PostCutConfiguration: value={p.value}\n'.format(p=self.postCutConfig)

            for channel in self.channels:
                output += '    {name}\n'.format(name=channel.name)
                if not samePreCutConfig:
                    output += '    PreCutConfiguration: variable={p.variable}, method={p.method}, efficiency={p.efficiency}, purity={p.purity}\n'.format(p=channel.preCutConfig)
                if not samePostCutConfig:
                    output += '    PostCutConfiguration: value={p.value}\n'.format(p=channel.postCutConfig)
                if not sameMVAConfig:
                    output += '    MVAConfiguration: name={m.name}, type={m.type}, config={m.config}, target={m.target}, targetCluster={m.targetCluster}\n'.format(m=channel.mvaConfig)
                output += '        Individual Variables: ' + ', '.join(set(channel.mvaConfig.variables).difference(commonVariables)) + '\n'
        return output


def FullEventInterpretation(path, particles):
    """
    The Full Event Interpretation algorithm.
    Alle the Actors defined above are added to the sequence and are executed in an order which fulfills all requirements.
    This function returns if no more Actors can be called without violating some requirements.
        @param path the basf2 module path
        @param particles sequence of particle objects which shall be reconstructed by this algorithm
    """

    parser = argparse.ArgumentParser()
    parser.add_argument('-verbose', '--verbose', dest='verbose', action='store_true', help='Output additional information')
    parser.add_argument('-nosignal', '--no-signal-classifiers', dest='nosig', action='store_true', help='Do not train classifiers')
    args = parser.parse_args()

    # Add the basf2 module path
    seq = actorFramework.Sequence()
    seq.addFunction(CountMCParticles, path='Path')

    # Now loop over all given particles, foreach particle we add some Resources and Actors.
    for particle in particles:
        ########## RESOURCES #############
        seq.addResource('Name_{i}'.format(i=particle.identifier), particle.name)
        seq.addResource('Label_{i}'.format(i=particle.identifier), particle.label)
        seq.addResource('Identifier_{i}'.format(i=particle.identifier), particle.identifier)
        seq.addResource('Object_{i}'.format(i=particle.identifier), particle)

        if particle.channels == []:
            seq.addResource('MVAConfig_{i}'.format(i=particle.identifier), particle.mvaConfig)
            seq.addResource('PostCutConfig_{i}'.format(i=particle.identifier), particle.postCutConfig)
        for channel in particle.channels:
            seq.addResource('Name_{c}'.format(c=channel.name), channel.name)
            seq.addResource('MVAConfig_{c}'.format(c=channel.name), channel.mvaConfig)
            seq.addResource('PreCutConfig_{c}'.format(c=channel.name), channel.preCutConfig)
            seq.addResource('PostCutConfig_{c}'.format(c=channel.name), channel.postCutConfig)

        ########### RECONSTRUCTION ACTORS ##########
        if particle.isFSP:
            seq.addFunction(SelectParticleList,
                            path='Path',
                            particleName='Name_{i}'.format(i=particle.identifier),
                            particleLabel='Label_{i}'.format(i=particle.identifier))
            seq.addFunction(CopyParticleLists,
                            path='Path',
                            particleName='Name_{i}'.format(i=particle.identifier),
                            particleLabel='Label_{i}'.format(i=particle.identifier),
                            inputLists=['RawParticleList_{i}'.format(i=particle.identifier)],
                            postCuts=['PostCut_{i}'.format(i=particle.identifier)])
        else:
            for channel in particle.channels:
                seq.addFunction(MakeAndMatchParticleList,
                                path='Path',
                                particleName='Name_{i}'.format(i=particle.identifier),
                                particleLabel='Label_{i}'.format(i=particle.identifier),
                                channelName='Name_{c}'.format(c=channel.name),
                                daughterParticleLists=['ParticleList_{d}'.format(d=daughter) for daughter in channel.daughters],
                                preCut='PreCut_{c}'.format(c=channel.name))
                seq.addFunction(FitVertex,
                                path='Path',
                                channelName='Name_{c}'.format(c=channel.name),
                                particleList='RawParticleList_{c}'.format(c=channel.name))

            seq.addFunction(CopyParticleLists,
                            path='Path',
                            particleName='Name_{i}'.format(i=particle.identifier),
                            particleLabel='Label_{i}'.format(i=particle.identifier),
                            inputLists=['RawParticleList_{c}'.format(c=channel.name) for channel in particle.channels],
                            postCuts=['PostCut_{c}'.format(c=channel.name) for channel in particle.channels])

        ############# PRECUT DETERMINATION ############
        if not particle.isFSP:
            seq.addFunction(PreCutDetermination,
                            channelNames=['Name_{c}'.format(c=channel.name) for channel in particle.channels],
                            preCutConfigs=['PreCutConfig_{c}'.format(c=channel.name) for channel in particle.channels],
                            preCutHistograms=['PreCutHistogram_{c}'.format(c=channel.name) for channel in particle.channels])

            for channel in particle.channels:
                additionalDependencies = []
                if 'SignalProbability' in channel.preCutConfig.variable:
                    additionalDependencies = ['SignalProbability_{d}'.format(d=daughter) for daughter in channel.daughters]

                seq.addFunction(CreatePreCutHistogram,
                                path='Path',
                                particleName='Name_{i}'.format(i=particle.identifier),
                                channelName='Name_{c}'.format(c=channel.name),
                                preCutConfig='PreCutConfig_{c}'.format(c=channel.name),
                                daughterParticleLists=['ParticleList_{d}'.format(d=daughter) for daughter in channel.daughters],
                                additionalDependencies=additionalDependencies)

        ############## POSTCUT DETERMINATION #############
        if particle.isFSP:
            seq.addFunction(PostCutDetermination,
                            identifiers=['Identifier_{i}'.format(i=particle.identifier)],
                            postCutConfigs=['PostCutConfig_{i}'.format(i=particle.identifier)],
                            signalProbabilities=['SignalProbability_{i}'.format(i=particle.identifier)])

        else:
            seq.addFunction(PostCutDetermination,
                            identifiers=['Name_{c}'.format(c=channel.name) for channel in particle.channels],
                            postCutConfigs=['PostCutConfig_{c}'.format(c=channel.name) for channel in particle.channels],
                            signalProbabilities=['SignalProbability_{c}'.format(c=channel.name) for channel in particle.channels])

        ########### SIGNAL PROBABILITY ACTORS #######
        if not args.nosig:
            if particle.isFSP:
                seq.addFunction(SignalProbability,
                                path='Path',
                                identifier='Identifier_{i}'.format(i=particle.identifier),
                                mvaConfig='MVAConfig_{i}'.format(i=particle.identifier),
                                particleList='RawParticleList_{i}'.format(i=particle.identifier))
            else:
                for channel in particle.channels:

                    additionalDependencies = []
                    if any('SignalProbability' in variable for variable in channel.mvaConfig.variables):
                        additionalDependencies += ['SignalProbability_{d}'.format(d=daughter) for daughter in channel.daughters]
                    if any(variable in ['dx', 'dy', 'dz', 'dr'] for variable in channel.mvaConfig.variables):
                        additionalDependencies += ['VertexFit_{c}'.format(c=channel.name)]

                    seq.addFunction(SignalProbability,
                                    path='Path',
                                    identifier='Name_{c}'.format(c=channel.name),
                                    mvaConfig='MVAConfig_{c}'.format(c=channel.name),
                                    particleList='RawParticleList_{c}'.format(c=channel.name),
                                    additionalDependencies=additionalDependencies)

                seq.addResource('SignalProbability_{i}'.format(i=particle.identifier), 'Dummy',
                                requires=['SignalProbability_{c}'.format(c=channel.name) for channel in particle.channels], strict=False)
            if particle.name != pdg.conjugate(particle.name):
                seq.addResource('SignalProbability_{p}:{l}'.format(p=pdg.conjugate(particle.name), l=particle.label), 'Dummy',
                                requires=['SignalProbability_{i}'.format(i=particle.identifier)])

        ################ Information ACTORS #################
        for channel in particle.channels:

            seq.addFunction(WriteAnalysisFileForChannel,
                            particleName='Name_{i}'.format(i=particle.identifier),
                            particleLabel='Label_{i}'.format(i=particle.identifier),
                            channelName='Name_{c}'.format(c=channel.name),
                            preCutConfig='PreCutConfig_{c}'.format(c=channel.name),
                            preCut='PreCut_{c}'.format(c=channel.name),
                            preCutHistogram='PreCutHistogram_{c}'.format(c=channel.name),
                            mvaConfig='MVAConfig_{c}'.format(c=channel.name),
                            signalProbability='SignalProbability_{c}'.format(c=channel.name),
                            postCutConfig='PostCutConfig_{c}'.format(c=channel.name),
                            postCut='PostCut_{c}'.format(c=channel.name))

        if particle.isFSP:
            seq.addFunction(WriteAnalysisFileForFSParticle,
                            particleName='Name_{i}'.format(i=particle.identifier),
                            particleLabel='Label_{i}'.format(i=particle.identifier),
                            mvaConfig='MVAConfig_{i}'.format(i=particle.identifier),
                            signalProbability='SignalProbability_{i}'.format(i=particle.identifier),
                            postCutConfig='PostCutConfig_{i}'.format(i=particle.identifier),
                            postCut='PostCut_{i}'.format(i=particle.identifier),
                            mcCounts='MCParticleCounts')
        else:
            seq.addFunction(WriteAnalysisFileForCombinedParticle,
                            particleName='Name_{i}'.format(i=particle.identifier),
                            particleLabel='Label_{i}'.format(i=particle.identifier),
                            channelPlaceholders=['Placeholders_{c}'.format(c=channel.name) for channel in particle.channels],
                            mcCounts='MCParticleCounts')

    finalParticles = [particle for particle in particles if all(particle.identifier not in o.daughters for o in particles)]
    for finalParticle in finalParticles:
        seq.addFunction(VariablesToNTuple,
                        path='Path',
                        particleIdentifier='Identifier_{i}'.format(i=finalParticle.identifier),
                        particleList='ParticleList_{i}'.format(i=finalParticle.identifier),
                        signalProbability='SignalProbability_{i}'.format(i=finalParticle.identifier))
        seq.addNeeded('SignalProbability_{i}'.format(i=finalParticle.identifier))
        seq.addNeeded('ParticleList_{i}'.format(i=finalParticle.identifier))

    seq.addFunction(WriteAnalysisFileSummary,
                    finalStateParticlePlaceholders=['Placeholders_{i}'.format(i=particle.identifier) for particle in particles if particle.isFSP],
                    combinedParticlePlaceholders=['Placeholders_{i}'.format(i=particle.identifier) for particle in particles if not particle.isFSP],
                    finalParticleNTuples=['VariablesToNTuple_{i}'.format(i=finalParticle.identifier) for finalParticle in finalParticles],
                    mcCounts='MCParticleCounts',
                    particles=['Object_{i}'.format(i=particle.identifier)for particle in particles])
    seq.addNeeded('FEIsummary.pdf')

    seq.run(path, args.verbose)
