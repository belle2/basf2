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
#      CreatePreCutDistribution require among others the SignalProbability for the ParticleLists
#                        provides PreCutDistribution
#      PreCutDetermination requires among others a PreCutDistribution
#                         provides PreCuts
#      ParticleListFromChannel requires among others PreCuts
# ... and so on ...
#
# The actors are added to the Sequence of actors in the FullEventInterpretation function at the end of this file.
# Afterwards the dependencies between the Actors are solved, and the Actors are called in the correct order, with the required parameters.
# If no further Actors can be called without satisfying all their requirements, the FullReoncstruction function returns.
# Therefore the end user has to run the FullEventInterpretation several times, until all Distributions, Classifiers, ... are created.
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
    and provides a method (see TMVA Interface), variables (see VariableManager) and the decay channels of the particle.
    In total this class contains: name, method, variables, efficiency (of preCuts) and all decay channels of a particle.
    """

    ## Create new class called MVAConfiguration via namedtuple. namedtuples are like a struct in C
    MVAConfiguration = collections.namedtuple('MVAConfiguration', 'name, type, config, variables, target, targetCluster')
    ## Create new class called PreCutConfiguration via namedtuple. namedtuples are like a struct in C
    PreCutConfiguration = collections.namedtuple('PreCutConfiguration', 'variable, method, efficiency, purity')
    ## Create new class called PostCutConfiguration via namedtuple. namedtuples are like a struct in C
    PostCutConfiguration = collections.namedtuple('PostCutConfiguration', 'value')
    ## Create new class called DecayChannel via namedtuple. namedtuples are like a struct in C
    DecayChannel = collections.namedtuple('DecayChannel', 'name, daughters, mvaConfig, isIncomplete')

    def __init__(self, name, mvaConfig, preCutConfig=PreCutConfiguration(variable='Mass', method='S/B', efficiency=0.70, purity=0.001),
                 postCutConfig=PostCutConfiguration(value=0.1)):
        """
        Creates a Particle without any decay channels. To add decay channels use addChannel method.
            @param name is the correct pdg name as a string of the particle.
            @param mvaConfig multivariate analysis configuration
            @param preCutConfig intermediate cut configuration
            @param postCutConfig post cut configuration
        """
        ## The name of the particle as correct pdg name e.g. K+, pi-, D*0.
        self.name = name
        ## multivariate analysis configuration
        self.mvaConfig = mvaConfig
        ## Decay channels, added by addChannel method.
        self.channels = []
        ## intermediate cut configuration
        self.preCutConfig = preCutConfig
        ## post cut configuration
        self.postCutConfig = postCutConfig

    @property
    def complete_channels(self):
        """ Property which returns all channels of this particle which don't have missing daughters """
        return [channel for channel in self.channels if not channel.isIncomplete]

    @property
    def incomplete_channels(self):
        """ Property which returns all channels of this particle which have missing daughters """
        return [channel for channel in self.channels if channel.isIncomplete]

    @property
    def daughters(self):
        """ Property which returns all daughter particles of all channels """
        return list(frozenset([daughter for channel in self.channels for daughter in channel.daughters]))

    @property
    def complete_daughters(self):
        """ Property which returns all daughter particles of all complete channels """
        return list(frozenset([daughter for channel in self.complete_channels for daughter in channel.daughters]))

    @property
    def isFSP(self):
        """ Returns true if the particle is a final state particle """
        return self.channels == []

    def addChannel(self, daughters, mvaConfig=None, isIncomplete=False, addExtraVars=True):
        """
        Appends a new decay channel to the Particle object.
        @param daughters is a list of pdg particle names e.g. ['pi+','K-']
        @param mvaConfig multivariate analysis configuration
        @param isIncomplete True if the given channel misses some particles like neutrinos or photons.
        @param addExtraVars True if getExtraInfo(SignalProbability) should be added for all daughters
        """
        mvaConfig = copy.deepcopy(self.mvaConfig if mvaConfig is None else mvaConfig)
        if addExtraVars:
            mvaConfig.variables.extend(['daughter{i}(getExtraInfo(SignalProbability))'.format(i=i) for i in range(0, len(daughters))])
        self.channels.append(Particle.DecayChannel(name=self.name + ' ==> ' + ' '.join(daughters),
                                                   daughters=daughters,
                                                   mvaConfig=mvaConfig, isIncomplete=isIncomplete))
        return self


def addIncompleteChannels(particles, verbose):
    """
    Adds automatically new incomplete channels to all particles determined from the incomplete channels
    of the daughter particles of the complete channels of the particle. e.g. D0 ==> e+ K- (missing neutrino)
    leads to a new incomplete channel in D*+ ==> D0 pi+, called D*+ ==> D0 ==> e+ K-_D0 pi+. This is necessary because
    incomplete channels have different kinematics than complete channels, therefore they need their own networks.
    """
    # Add all FSP as processed
    processed = [particle.name for particle in particles if particle.channels == []]
    processed += [pdg.conjugate(particle.name) for particle in particles if particle.channels == []]

    # Loop over all particles
    while len(processed) < 2 * len(particles):
        for particle in [particle for particle in particles if particle.name not in processed and all([daughter in processed for daughter in particle.complete_daughters])]:
            for name in [particle.name, pdg.conjugate(particle.name)]:
                for mother in [mother for mother in particles if name in mother.complete_daughters]:
                    for incomplete_daughter_channel in particle.incomplete_channels:
                        for complete_mother_channel in mother.complete_channels:
                            if name in complete_mother_channel.daughters:
                                daughters = [daughter for daughter in complete_mother_channel.daughters]
                                daughters[daughters.index(name)] = incomplete_daughter_channel.name + '_' + name
                                mother.addChannel(daughters, complete_mother_channel.mvaConfig, isIncomplete=True, addExtraVars=False)
                                if verbose:
                                    print "Added new incomplete channel to " + mother.name + " " + str(daughters)
                processed.append(name)
    return particles


def FullEventInterpretation(path, particles):
    """
    The FullReconstruction algorithm.
    Alle the Actors defined above are added to the sequence and are executed in an order which fulfills all requirements.
    This function returns if no more Actors can be called without violate some requirements.
        @param path the basf2 module path
        @param particles sequence of particle objects which shall be reconstructed by this algorithm
    """

    parser = argparse.ArgumentParser()
    parser.add_argument('-verbose', '--verbose', dest='verbose', action='store_true', help='Output additional information')
    parser.add_argument('-nosignal', '--no-signal-classifiers', dest='nosig', action='store_true', help='Do not train classifiers')
    args = parser.parse_args()

    particles = addIncompleteChannels(particles, args.verbose)

    # Add the basf2 module path
    seq = actorFramework.Sequence()

    # Now loop over all given particles, foreach particle we add some Resources and Actors.
    for particle in particles:
        ########## RESOURCES #############
        # Add all information the user has provided as resources to the sequence, so our function can require them
        seq.addResource('Name_' + particle.name, particle.name)
        seq.addResource('PreCutConfig_' + particle.name, particle.preCutConfig)
        seq.addResource('PostCutConfig_' + particle.name, particle.postCutConfig)

        # Add Variables:
        # If the particle is a FSP, we add the variables for the particle
        # If the particle has decay channels, we add the name and the variables for these decay channels.
        if particle.channels == []:
            seq.addResource('MVAConfig_' + particle.name, particle.mvaConfig)
        for channel in particle.channels:
            seq.addResource('Name_' + channel.name, channel.name)
            seq.addResource('MVAConfig_' + channel.name, channel.mvaConfig)

        ########### RECONSTRUCTION ACTORS ##########
        # The Reconstruction part of the FullReconstruction:
        # 1. Load the FSP (via SelectParticleList),
        # 2. Create a particle list for each channel of the non-FSP (via MakeAndMatchParticleList), depending an all daughter ParticleLists of this channel
        # 3. Merge the different ParticleLists for each complete channel into a single ParticleList, depending on all complete channel ParticleLists
        if particle.isFSP:
            seq.addFunction(SelectParticleList,
                            path='Path',
                            particleName='Name_' + particle.name)
            seq.addFunction(CopyParticleLists,
                            path='Path',
                            particleName='Name_' + particle.name,
                            inputLists=['RawParticleList_' + particle.name],
                            postCut='PostCut_' + particle.name,
                            pdf='PDF_' + particle.name)
        else:
            for channel in particle.channels:
                seq.addFunction(MakeAndMatchParticleList,
                                path='Path',
                                particleName='Name_' + particle.name,
                                channelName='Name_' + channel.name,
                                inputLists=['ParticleList_' + daughter for daughter in channel.daughters],
                                preCut='PreCut_' + channel.name)
            seq.addFunction(CopyParticleLists,
                            path='Path',
                            particleName='Name_' + particle.name,
                            inputLists=['ParticleList_' + channel.name + '_' + particle.name for channel in particle.complete_channels],
                            postCut='PostCut_' + particle.name,
                            pdf='PDF_' + particle.name)

        ############# PRECUT DETERMINATION ############
        # Intermediate PreCut part of FullReconstruction algorithm.
        # To surpress the computational cost due to the combinatoric, we apply efficient PreCuts before combining the daughter particles of each channel in the reconstruction part.
        # The PreCuts are chosen, so that a user-defined total signal efficiency for this particle is met.
        # 1. Create Histograms with the invariant mass or signal probability distribution for each channel, as soon as all daughter ParticleLists and daughter SignalProbabilities are available.
        # 2. If the Histogram is available calculate the PreCuts with the PreCutDetermination function. As soon as the PreCuts are available the particle can be reconstructed.
        if particle.channels != []:
            seq.addFunction(PreCutDetermination,
                            particleName='Name_' + particle.name,
                            channelNames=['Name_' + channel.name for channel in particle.channels],
                            preCutConfig='PreCutConfig_' + particle.name,
                            preCutHistograms=['PreCutHistogram_' + channel.name for channel in particle.channels])

            for channel in particle.channels:

                additionalDependencies = []
                if particle.preCutConfig.variable == 'daughterProductOfSignalProbability':
                    additionalDependencies = ['SignalProbability_' + daughter for daughter in channel.daughters]

                seq.addFunction(CreatePreCutHistogram,
                                path='Path',
                                particleName='Name_' + particle.name,
                                channelName='Name_' + channel.name,
                                preCutConfig='PreCutConfig_' + particle.name,
                                daughterLists=['ParticleList_' + daughter for daughter in channel.daughters],
                                additionalDependencies=additionalDependencies)

        seq.addFunction(PostCutDetermination,
                        particleName='Name_' + particle.name,
                        postCutConfig='PostCutConfig_' + particle.name,
                        signalProbability='SignalProbability_' + particle.name)

        ########### SIGNAL PROBABILITY ACTORS #######
        # The classifier part of the FullReconstruction.
        if not args.nosig:
            if particle.isFSP:
                seq.addFunction(SignalProbability,
                                path='Path',
                                particleName='Name_' + particle.name,
                                channelName='Name_' + particle.name,
                                mvaConfig='MVAConfig_' + particle.name,
                                particleList='RawParticleList_' + particle.name)
            else:
                for channel in particle.channels:
                    seq.addFunction(SignalProbability,
                                    path='Path',
                                    particleName='Name_' + particle.name,
                                    channelName='Name_' + channel.name,
                                    mvaConfig='MVAConfig_' + channel.name,
                                    particleList='ParticleList_' + channel.name + '_' + particle.name,
                                    daughterSignalProbabilities=['SignalProbability_' + daughter for daughter in channel.daughters])
                seq.addResource('SignalProbability_' + particle.name, 'Dummy', requires=['SignalProbability_' + channel.name + '_' + particle.name for channel in particle.channels], strict=False)
                seq.addResource('SignalProbability_' + pdg.conjugate(particle.name), 'Dummy', requires=['SignalProbability_' + particle.name])

        ################ Information ACTORS #################
        for channel in particle.channels:
            seq.addFunction(WriteAnalysisFileForChannel,
                            particleName='Name_' + particle.name,
                            channelName='Name_' + channel.name,
                            channelList='ParticleList_' + channel.name + '_' + particle.name,
                            preCutConfig='PreCutConfig_' + particle.name,
                            preCutHistogram='PreCutHistogram_' + channel.name,
                            preCut='PreCut_' + channel.name,
                            mvaConfig='MVAConfig_' + channel.name,
                            signalProbability='SignalProbability_' + channel.name + '_' + particle.name)

        seq.addFunction(WriteAnalysisFileForParticle,
                        particleName='Name_' + particle.name,
                        postCutConfig='PostCutConfig_' + particle.name,
                        postCut='PostCut_' + particle.name,
                        texfiles=['Tex_' + channel.name for channel in particle.channels])

    #TODO: don't hardcode B0/B+ here
    seq.addFunction(VariablesToNTuple,
                    path='Path',
                    particleName='Name_B0',
                    particleList='ParticleList_B0',
                    signalProbability='SignalProbability_B0')

    seq.addFunction(VariablesToNTuple,
                    path='Path',
                    particleName='Name_B+',
                    particleList='ParticleList_B+',
                    signalProbability='SignalProbability_B+')

    seq.addFunction(WriteAnalysisFileSummary,
                    pdffiles=['PDF_' + particle.name for particle in particles],
                    ntuples=['VariablesToNTuple_B0'])  # , 'VariablesToNTuple_B+'])

    seq.addNeeded('SignalProbability_B+')
    seq.addNeeded('SignalProbability_B0')
    seq.addNeeded('ParticleList_B0')
    seq.addNeeded('ParticleList_B+')

    seq.run(path, args.verbose)
