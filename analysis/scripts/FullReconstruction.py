#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Thomas Keck 2014
#
# This FullReconstrucion algorithm uses a functional approach.
# All tasks are implemented in Functions called Actors.
# Each Actor has requirements and provides return values.
# E.g. SignalProbability requires the path, a method, variables and a ParticleList.
#                        provides SignalProbability for the ParticleList.
#      PreCutDistribution require among others the SignalProbability for the ParticleLists
#                        provides PreCutDistribution
#      PreCutDetermination requires among others a PreCutDistribution
#                         provides PreCuts
#      ParticleListFromChannel requires among others PreCuts
# ... and so on ...
#
# In the FR_*.py file all the Actors are defined and they're added to the Sequence of Actors in the FullReconstruction function at the end of this file.
# Afterwards the dependencies between the Actors are solved, and the Actors are called in the correct order, with the required parameters.
# If no further Actors can be called without satisfying all their requirements, the FullReoncstruction function returns.
# Therefore the end user has to run the FullReconstruction several times, until all Distributions, Classifiers, ... are created.
#
# To create a new Actor:
#   1. Write a normal function which takes the needed arguments and returns a dictonary of provided values . E.g. def foo(path, particleList) ... return {'Stuff': x}
#   2. Make sure your return value depends on all the used arguments, easiest way to accomplish this is using the createHash function
#   2. Add the function in the FullReonstruction method (at the end of the file) to the sequence like this: seq.addFunction(foo, path='Path', particleList='K+')


import pdg

import FR_utility
from FR_reconstruction import ParticleList, ParticleListFromChannel
from FR_signalProbability import SignalProbability, SignalProbabilityFSPCluster
from FR_preCut import PreCutMassDetermination, PreCutProbDetermination, CreatePreCutMassHistogram, CreatePreCutProbHistogram, PrintPreCuts

import os
import collections
import argparse


class Particle(object):
    """
    The Particle class is the only class the end-user gets into contact with.
    The user creates an instance of this class for every particle he wants to reconstruct with the FullReconstruction algorithm,
    and provides a method (see TMVA Interface), variables (see VariableManager) and the decay channels of the particle.
    In total this class contains: name, method, variables, efficiency (of preCuts) and all decay channels of a particle.
    """

    ## Create new class called DecayChannel via namedtuple. namedtuples are like a struct in C with two members: name and daughters
    DecayChannel = collections.namedtuple('DecayChannel', 'name, daughters')

    def __init__(self, name, variables, method, efficiency=0.90):
        """
        Creates a Particle without any decay channels. To add decay channels use addChannel method.
            @param name is the correct pdg name as a string of the particle.
            @param variables is a list of variables which are used to classify the particle. In addition to the here given variables,
                   signal probability variables of all daughter particles are automatically added later for every channel.
            @param method is a tuple (name, type, config) of the method used to classify the particle.
            @param efficiency minimum signal efficiency of preCuts
        """
        ## The name of the particle as correct pdg name e.g. K+, pi-, D*0.
        self.name = name
        ## A list of variables which are used to classify the particle.
        self.variables = variables
        ## A tuple (name, type, config) of the  method used to classify the particle.
        self.method = method
        ## Decay channels, added by addChannel method.
        self.channels = []
        ## Minimum signal efficiency of preCuts for this particle
        self.efficiency = efficiency

    def addChannel(self, daughters):
        """
        Appends a new decay channel to the Particle object.
        @param daughters is a list of pdg particle names e.g. ['pi+','K-']
        """
        # Append a new DecayChannel two the channel member
        self.channels.append(Particle.DecayChannel(name=self.name + ' ->' + ' '.join(daughters), daughters=daughters))
        return self


def FullReconstruction(path, particles):
        """
        The FullReconstruction algorithm.
        Alle the Actors defined above are added to the sequence and are executed in an order which fulfills all requirements.
        This function returns if no more Actors can be called without violate some requirements.
            @param path the basf2 module path
            @particles sequence of particle objects which shall be reconstructed by this algorithm
        """

        parser = argparse.ArgumentParser()
        parser.add_argument('-cp', '--cut-probability', dest='cp', action='store_true', help='Use cut on product of signal probability')
        parser.add_argument('-cm', '--cut-mass', dest='cm', action='store_true', help='Use cut on invariant mass')
        parser.add_argument('-fc', '--fsp-cluster', dest='fc', action='store_true', help='Cluster training for charged fsp particles')
        parser.add_argument('-ve', '--verbose', dest='verbose', action='store_true', help='Output additional information')
        parser.add_argument('-nosig', '--no-signal-classifiers', dest='nosig', action='store_true', help='Do not train classifiers')
        args = parser.parse_args()

        # First add the basf2 module path
        seq = FR_utility.Sequence()
        seq.addResource('Path', path)

        # Now loop over all given particles, foreach particle we add some Resources and Actors.
        for particle in particles:
            ########## RESOURCES #############
            # Add all information the user has provided as resources to the sequence, so our function can require them
            seq.addResource('Name_' + particle.name, particle.name)
            seq.addResource('PDG_' + particle.name, pdg.from_name(particle.name))
            seq.addResource('Method_' + particle.name, particle.method)
            seq.addResource('Efficiency_' + particle.name, particle.efficiency)
            seq.addResource('Particle_' + particle.name, particle)
            # If the particle isn't self conjugated we add the name and the pdg also for the antiparticle
            name_conj = pdg.conjugate(particle.name)
            if particle.name != name_conj:
                seq.addResource('Name_' + name_conj, name_conj)
                seq.addResource('PDG_' + name_conj, pdg.from_name(name_conj))
            # Add Variables:
            # If the particle is a FSP, we add the variables for the particle
            # If the particle has decay channels, we add the name and the variables for these decay channels.
            # In addition childProb variables are added for each channel automatically here.
            # The IsIgnored flag is used to deactivate a channel if there's not enough statistics.
            if particle.channels == []:
                seq.addResource('Variables_' + particle.name, particle.variables)
            for channel in particle.channels:
                seq.addResource('Name_' + channel.name, channel.name)
                seq.addResource('Variables_' + channel.name, particle.variables + ['daughter{i}(ExtraInfo(signalProbability))'.format(i=i) for i in range(0, len(channel.daughters))])
                seq.addResource('IsIgnored_' + channel.name, False)

            ########### RECONSTRUCTION ACTORS ##########
            # The Reconstruction part of the FullReconstruction:
            # 1. Load the FSP (via ParticleList),
            # 2. Create a particle list for each channel of the non-FSP (via ParticleListFromChannel), depending an all daughter ParticleLists of this channel
            # 3. Gather up the different ParticleLists for each channel, into a single ParticleList for the particle itself, depending on all channel ParticleLists
            # 4. If the particle isn't self conjugated we gather up the anti-particles as well, because anti-particles aren't handlet properly yet.  FIXME
            channelParticleLists = ['ParticleList_' + channel.name for channel in particle.channels]
            seq.addFunction(ParticleList,
                            path='Path',
                            name='Name_' + particle.name,
                            pdgcode='PDG_' + particle.name,
                            particleLists=channelParticleLists)
            for channel in particle.channels:
                seq.addFunction(ParticleListFromChannel,
                                path='Path',
                                pdgcode='PDG_' + particle.name,
                                name='Name_' + channel.name,
                                preCut='PreCut_' + channel.name,
                                inputLists=['ParticleList_' + daughter for daughter in channel.daughters],
                                isIgnored='IsIgnored_' + channel.name)
            if particle.name != name_conj:
                seq.addFunction(ParticleList,
                                path='Path',
                                name='Name_' + name_conj,
                                pdgcode='PDG_' + name_conj,
                                particleLists=channelParticleLists)

            ############# PRECUT DETERMINATION ############
            # Intermediate PreCut part of FullReconstruction algorithm.
            # To surpress the computational cost due to the combinatoric, we apply efficient PreCuts before combining the daughter particles of each channel in the reconstruction part.
            # The PreCuts are chosen, so that a user-defined total signal efficiency for this particle is met.
            # 1. Create Histograms with the invariant mass or signal probability distribution for each channel, as soon as all daughter ParticleLists and daughter SignalProbabilities are available.
            # 2. If the Histogram is available calculate the PreCuts with the PreCutDetermination function. As soon as the PreCuts are available the particle can be reconstructed.
            if particle.channels != []:
                if args.cp:
                    for channel in particle.channels:
                        seq.addFunction(CreatePreCutProbHistogram,
                                        path='Path',
                                        particle='Particle_' + particle.name,
                                        name='Name_' + channel.name,
                                        daughterLists=['ParticleList_' + daughter for daughter in channel.daughters],
                                        daughterSignalProbabilities=['SignalProbability_' + daughter for daughter in channel.daughters])
                    seq.addFunction(PreCutProbDetermination,
                                    name='Name_' + particle.name,
                                    pdgcode='PDG_' + particle.name,
                                    channels=['Name_' + channel.name for channel in particle.channels],
                                    preCut_Histograms=['PreCutHistogram_' + channel.name for channel in particle.channels],
                                    efficiency='Efficiency_' + particle.name)
                else:
                    for channel in particle.channels:
                        seq.addFunction(CreatePreCutMassHistogram,
                                        path='Path',
                                        particle='Particle_' + particle.name,
                                        name='Name_' + channel.name,
                                        daughterLists=['ParticleList_' + daughter for daughter in channel.daughters])
                    seq.addFunction(PreCutMassDetermination,
                                    name='Name_' + particle.name,
                                    pdgcode='PDG_' + particle.name,
                                    channels=['Name_' + channel.name for channel in particle.channels],
                                    preCut_Histograms=['PreCutHistogram_' + channel.name for channel in particle.channels],
                                    efficiency='Efficiency_' + particle.name)
                if args.verbose:
                    seq.addFunction(PrintPreCuts,
                                    name='Name_' + particle.name,
                                    channels=['Name_' + channel.name for channel in particle.channels],
                                    preCuts=['PreCut_' + channel.name for channel in particle.channels],
                                    ignoredChannels=['IsIgnored_' + channel.name for channel in particle.channels])
            ########### SIGNAL PROBABILITY ACTORS #######
            # The classifier part of the FullReconstruction. Here one has to distinguish between [e+, mu+, pi+, K+, p], other FSPs and non-FSPs.
            # All the classifiers depend on the method, variables and the ParticleList (either for the particle or for the channel).
            # [e+,mu+,pi+,K+,p]: All the tracks seen in the detector are generated by one of these particles.
            #                    To get a better spearation we train these particles against each other and combine the result later
            if not args.nosig:
                if args.fc and particle.channels == [] and particle.name in ['e+', 'e-', 'mu+', 'mu-', 'pi+', 'pi-', 'K+', 'K-', 'p+', 'p-']:
                    # Add charged FSP SignalProbability classifier
                    seq.addFunction(SignalProbabilityFSPCluster,
                                    path='Path',
                                    method='Method_' + particle.name,
                                    variables='Variables_' + particle.name,
                                    name='Name_' + particle.name,
                                    pdg='PDG_' + particle.name,
                                    particleList='ParticleList_' + particle.name)

                # Other FSP: Other FSP like gammas are trained with a single classifier.
                elif particle.channels == []:
                    seq.addFunction(SignalProbability,
                                    path='Path',
                                    method='Method_' + particle.name,
                                    variables='Variables_' + particle.name,
                                    name='Name_' + particle.name,
                                    particleList='ParticleList_' + particle.name)

                # Non FSP:
                #   1. Train classifier for every decay channel of the particle.
                #   2. As soon as all SignalProbabilities for all channels are available the resoure SignalProbability_{particleName} is unlocked.
                elif particle.channels != []:
                    for channel in particle.channels:
                        seq.addFunction(SignalProbability,
                                        path='Path',
                                        method='Method_' + particle.name,
                                        variables='Variables_' + channel.name,
                                        name='Name_' + channel.name,
                                        particleList='ParticleList_' + channel.name,
                                        daughterSignalProbabilities=['SignalProbability_' + daughter for daughter in channel.daughters],
                                        isIgnored='IsIgnored_' + channel.name)
                    seq.addResource('SignalProbability_' + particle.name, 'Dummy', requires=['SignalProbability_' + channel.name for channel in particle.channels])

                # If the classifiers provide the signal probability, they also provide the signal probability for the anti particles, so we add
                # a dummy resource which depends only on the SignalProbability of the particle and provides the SignalProbability for the anti-particle.
                seq.addResource('SignalProbability_' + pdg.conjugate(particle.name), 'Dummy', requires=['SignalProbability_' + particle.name])
        seq.run(args.verbose)
