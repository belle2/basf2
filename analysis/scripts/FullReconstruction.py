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
# In this file all the Actors are defined, and added to the sequence of Actors in the FullReconstruction function at the end of the file.
# Afterwards the dependencies between the Actors are solved, and the Actors are called in the correct order, with the required parameters.
# If no further Actors can be called without satisfying all their requirements, the FullReoncstruction function returns.
# Therefore the end user has to run the FullReconstruction several times, until all Distributions, Classifiers, ... are created.
#
# To create an new Actor:
#   1. Write a normal function which takes the needed arguments and returns a dictonary of provided values . E.g. def foo(path, particleList) ... return {'Stuff': x}
#   2. Add the function in the FullReonstruction method (at the end of the file) to the sequence like this: seq.append(Function(foo, path='Path', particleList='K+'))

from basf2 import *
import modularAnalysis
import pdg

import ROOT
import hashlib
import os
import collections


class Particle(object):
    """
    The Particle class is the only class the end-user gets into contact with.
    The user creates an instance of this class for every particle he wants to reconstruct with the FullReconstruction algorithm,
    and provides a method (see TMVA Interface), variables (see VariableManager) and the decay channels of the particle.
    In total this class contains: name, method, variables, efficiency (of preCuts) and all decay channels of a particle.
    """

    # Following line creates a new class called DecayChannel via namedtuple. namedtuples are like a struct in C with two members: name and daughters
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


################ We define some structuring classes, which implement the logic of our functional approach ###################

class Resource(object):
    """
    Resources are provide values if certain requirements are fulfilled.
    To be more specific the Resource class is a generic Functor class, which provides a simple value under a given name.
    It's used to provide things like: Name of a particle, PDG code of a particle, the basf2 path object,...
    """
    def __init__(self, name, x, requires=None):
        """
        Creates a new Resource
            @param name the name of the resource. Other Actors can require this resource using its name.
            @param x the value of the resource. If another Actor requires this resource its value is passed to the Actor.
            @param requires these requirements have to be fulfilled before calling this Actor
        """
        ##  the name of the resource. Other Actors can require this resource using its name.
        self.name = name
        ## the value of the resource. If another Actor requires this resource its value is passed to the Actor.
        self.x = x
        ## these requirements have to be fulfilled before calling this Actor
        self.requires = [] if requires is None else requires

    def __call__(self, *args):
        """
        Returns the given value x under the given name
            @param args additional arguments are ignored
        """
        return {self.name: self.x}


class Function(object):
    """
    This Functor class wraps a normal function into an Actor!
    Imagine you have a function foo, which does some crazy stuff:
        def foo(path, particleList): ...
    You can use this class to add foo to the sequence of Actors in the FullReconstruction algorithm like this:
        seq.append(Function(foo, path='Path', particleList='K+'))
    The first argument is always the wrapped functions, the following keyword arguments correspond to the arguments of your function,
    the value of the keyword arguments are the names of the requirements the system shall bin to this arguments, when the Functor is called.
    You can bind multiple requirements to one parameter of the function by passing a list of requirements as the corresponding keyword argument
        seq.append(Function(foo, path='Path', particleLists=['K+','pi-']))
    """
    def __init__(self, func, **kwargs):
        """
        Creates a new Actor
            @func The function which is called by this Actor
            @kwargs The requirements of the function. The given keyword arguments have to correspond to the arguments of the function!
        """
        ## the name of the actor, for the moment this is simply the name of the function itself
        self.name = func.__name__
        ## the function which is called by this actor
        self.func = func
        ## The kwargs provided by the user.
        self.parameters = kwargs
        ## These requirements have to be fulfilled before calling this Actor
        self.requires = []
        for (key, value) in self.parameters.iteritems():
            if isinstance(value, str):
                self.requires.append(value)
            else:
                self.requires += value

    def __call__(self, *arguments):
        """
        Calls the underlying function of this actor with the required arguments
            @arguments the argument which were required, in the same order as specified in self.requires
        """
        requirement_argument_mapping = dict(zip(self.requires, arguments))
        kwargs = {}
        for (key, value) in self.parameters.iteritems():
            if isinstance(value, str):
                kwargs[key] = requirement_argument_mapping[value]
            else:
                kwargs[key] = [requirement_argument_mapping[v] for v in value]
        return self.func(**kwargs)


############### Now here are the Actors, every Actor implements a part of the FullReconstruction algorithm ################

def ParticleList(path, pdgcode, particleLists):
    """
    Creates a ParticleList gathering up all particles with the given pdgcode.
        @param path the basf2 path
        @param pdgcode the pdgcode of the particle
        @param particleLists additional requirements before the Particles can be gathered. E.g. All ParticleList of created by the ParticleListFromChannel exist for this particle
    """
    # Select all the reconstructed (or loaded) particles with this pdg into one list.
    name = pdg.to_name(pdgcode)
    name_conj = pdg.conjugate(name)
    modularAnalysis.selectParticle(name, pdgcode, [], path=path)
    # Because anti-particles aren't handled properly yet we have to select the charge conjugate particles by hand.
    if name != name_conj:
        modularAnalysis.selectParticle(name_conj, -pdgcode, [], path=path)
        return {'ParticleList_' + name: name, 'ParticleList_' + name_conj: name_conj}
    return {'ParticleList_' + name: name}


def ParticleListFromChannel(path, pdgcode, outputList, preCut, inputLists):
    """
    Creates a ParticleList by combining other particleLists via the ParticleCombiner module.
        @param path the basf2 path
        @param pdgcode pdgcode of the particle which is reconstructed
        @param outputList name of the list which holds the combined particles
        @param preCut cuts which are applied before the combining of the particles
        @param inputLists the inputs lists which are combined
    """
    pmake = register_module('ParticleCombiner')
    pmake.set_name('ParticleCombiner_' + outputList)
    pmake.param('PDG', pdgcode)
    pmake.param('ListName', outputList)
    pmake.param('InputListNames', inputLists)
    pmake.param(preCut['variable'], preCut['range'])
    path.add_module(pmake)
    modularAnalysis.matchMCTruth(outputList, path=path)
    return {'ParticleList_' + outputList: outputList}


def SignalProbability(path, method, variables, particleList):
    """
    Calculates the SignalProbability of a ParticleList. If the needed experts aren't available they're created.
        @param path the basf2 path
        @param method method given to the TMVAInterface (see TMVAExpert and TMVATeacher)
        @param variables used for classification (see VariableManager)
        @param particleList the particleList which is used for training and classification
    """
    # Create hash with all parameters on which this training depends
    hash = hashlib.sha1(str(method) + str(variables) + particleList).hexdigest()
    filename = 'weights/{particleList}_{hash}_{method}.weights.xml'.format(particleList=particleList, hash=hash, method=method[0])
    if os.path.isfile(filename):
        expert = register_module('TMVAExpert')
        expert.set_name('TMVAExpert_' + particleList)
        expert.param('identifier', particleList + '_' + hash)
        expert.param('method', method[0])
        expert.param('target', 'SignalProbability')
        expert.param('listNames', [particleList])
        path.add_module(expert)
        return {'SignalProbability_' + particleList: hash}
    else:
        teacher = register_module('TMVATeacher')
        teacher.set_name('TMVATeacher_' + particleList)
        teacher.param('identifier', particleList + '_' + hash)
        teacher.param('methods', [method])
        teacher.param('factoryOption', '!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification')
        teacher.param('variables', variables)
        teacher.param('target', 'isSignal')
        teacher.param('listNames', [particleList])
        path.add_module(teacher)
        return {}


def PreCutDistribution(path, particle, daughterLists, daughterSignalProbabilities):
    """
    Creates ROOT file with invariant mass and signal probability product distribution of every channel (signal/background)
    for a given particle, before any intermediate cuts are applied. For now this class uses the ParticleCombiner and HistMaker
    modules to generate these distributions, later we should join these modules together into one fast PreCutDistribution module.
        @param path the basf2 path
        @param particle object for which this distribution is created
        @param daughterLists all particleLists of all the daughter particles in all channels
        @param daughterSignalProbabilities all daughter particles need a SignalProbability
    """
    # Check if the file is available. If the file isn't available yet, create it with
    # the HistMaker Module and return Nothing. If a function is called which depends on
    # the PreCutDistribution, the process will stop, because the PreCutDistribution isn't provided.
    hash = hashlib.sha1(particle.name + str(daughterLists)).hexdigest()
    filename = 'Reconstruction_{name}_{hash}.root'.format(name=particle.name, hash=hash)
    if os.path.isfile(filename):
        return {'PreCutDistribution_' + particle.name: filename}
    else:
        # Combine all the particles according to the decay channels
        mass = pdg.get(pdg.from_name(particle.name)).Mass()
        for channel in particle.channels:
            pmake = register_module('ParticleCombiner')
            pmake.set_name('ParticleCombiner_' + channel.name)
            pmake.param('PDG', pdg.from_name(particle.name))
            pmake.param('ListName', channel.name)
            pmake.param('InputListNames', [daughter for daughter in channel.daughters])
            pmake.param('MassCut', (mass - mass * 1.0 / 20.0, mass + mass * 1.0 / 20.0))
            pmake.param('cutsOnProduct', {'SignalProbability': (0.0, 0.001)})
            path.add_module(pmake)
            modularAnalysis.matchMCTruth(channel.name, path=path)
        histMaker = register_module('HistMaker')
        histMaker.set_name(filename)
        histMaker.param('file', filename)
        histMaker.param('histVariables', [('M', 100, mass - mass * 1.0 / 20.0, mass + mass * 1.0 / 20.0), ('prodChildProb', 1000, 0, 1)])
        histMaker.param('make2dHists', True)
        histMaker.param('truthVariable', 'isSignal')
        histMaker.param('listNames', [channel.name for channel in particle.channels])
        path.add_module(histMaker)
    return {}


def PreCutDetermination(name, pdgcode, channels, distribution, efficiency):
    """
    Determines the PreCuts for all the channels of a particle. The cuts are chosen as follows:
        1. The maximum of every signal invariant mass distribution is isolated
        2. The ratio of signal/background invariant mass distribution is calculated and fitted with splines
        3. A cut on the y-axis of S/B is performed, so that a certain signal efficiency is garantueed.
        4. The cut on the y-axis is converted to a minimum and maximum cut on the x-axis for every channel.
        @param name name of the particle
        @param pdgcode pdgcode of the particle
        @param channels list of the names of all the channels
        @param distribution filename of the distribution file created by PreCutDistribution
        @param signal efficiency for this particle
    """

    # For every channel we get the signal and background distribution from the Reconstruction file,
    # search the position of the peak of the signal distribution (this peak is used as the starting point for the cut search)
    # and calculate the S/B ratio by cloning signal and dividing it by bckgrd.
    file = ROOT.TFile(distribution, 'UPDATE')
    signal = dict([(channel, file.Get(channel + '_M_signal_histogram')) for channel in channels])
    bckgrd = dict([(channel, file.Get(channel + '_M_background_histogram')) for channel in channels])
    maxpos = dict([(channel, value.GetBinCenter(value.GetMaximumBin())) for (channel, value) in signal.iteritems()])
    ratio = dict([(channel, value.Clone(channel + '_M_ratio_histogram')) for (channel, value) in signal.iteritems()])
    for channel in channels:
        ratio[channel].Divide(bckgrd[channel])
        ratio[channel].Write('', ROOT.TObject.kOverwrite)

    # Create a Spline fit of S/B ratio and wrap in a ROOT TF1 function, so we can perform GetX() on it
    spline = dict([(channel, ROOT.TSpline3(value)) for (channel, value) in ratio.iteritems()])
    spline_ROOT = dict([(channel, ROOT.TF1(channel + '_M_func', lambda x, channel=channel: spline[channel].Eval(x[0]), 0, 100, 0)) for channel in channels])

    # Calculate a cut on the y-axis, so that only at least nsignal * efficency signal events survive.
    # Therefore create a function which calculates the number of signal events after a cut on the y-axis
    # was performed and wrap it into a ROOT TF1 function, so we can perform GetX() on it.
    nsignals = sum([value.Integral(0, value.GetNbinsX()) for value in signal.values()])

    def ycut_to_xcuts(channel, cut):
        return (spline_ROOT[channel].GetX(cut, 0, maxpos[channel]), splint_ROOT[channel].GetX(cut, maxpos[channel], 100))

    def nsignals_after_cut(cut):
        nevents = 0
        for channel in channels:
            (a, b) = ycut_to_xcuts(channel, cut)
            nevents += signal[channel].Integral(signal[channel].FindBin(a), signal[channel].FindBin(b))
        return nevents

    nsignals_after_cut_ROOT = ROOT.TF1(distribution + '_mass_cut_func', nsignals_after_cut, 0, 1, 0)
    ycut = nsignals_after_cut_ROOT.GetX(nsignals * efficiency, 0.0, 1.0)

    # Return the calculates ChannelCuts
    return {'PreCut_' + channel: {'variable': 'MassCut', 'range': ycut_to_xcuts(channel, cut)} for channel in channels}


##################### Now we work wonders and magically call all the Actors in the correct order to fulfill their requirements ##################

def FullReconstruction(path, particles):
        """
        The FullReconstruction algorithm.
        Alle the Actors defined above are added to the sequence and are executed in an order which fulfills all requirements.
        This function returns if no more Actors can be called without violate some requirements.
            @param path the basf2 module path
            @particles sequence of particle objects which shall be reconstructed by this algorithm
        """

        # First add the basf2 module path
        seq = [Resource('Path', path)]

        # Now loop over all given particles, foreach particle we add some Resources and Actors.
        for particle in particles:
            ########## RESOURCES #############
            # First we add the resources of the particle
            seq.append(Resource('Name_' + particle.name, particle.name))
            seq.append(Resource('PDG_' + particle.name, pdg.from_name(particle.name)))
            seq.append(Resource('Method_' + particle.name, particle.method))
            seq.append(Resource('Efficiency_' + particle.name, particle.efficiency))
            seq.append(Resource('Particle_' + particle.name, particle))
            # Add some resources if FSP
            if particle.channels == []:
                seq.append(Resource('Variables_' + particle.name, particle.variables))
            # Add some resources for every channel
            for channel in particle.channels:
                seq.append(Resource('Name_' + channel.name, channel.name))
                # Here we add the additional childProb variables to each channel!
                seq.append(Resource('Variables_' + channel.name, particle.variables + ['childProb{i}'.format(i=i) for i in range(0, len(channel.daughters))]))

            ########### RECONSTRUCTION ACTORS ##########
            # ParticleList Actor provides the ParticleList for every particle
            seq.append(Function(ParticleList, path='Path', pdgcode='PDG_' + particle.name, particleLists=['ParticleList_' + channel.name for channel in particle.channels]))
            # ParticleListFromChannel Actor provides ParticleLists for every channel
            for channel in particle.channels:
                seq.append(Function(ParticleListFromChannel, path='Path', pdgcode='PDG_' + particle.name, outputList='Name_' + channel.name, preCut='PreCut_' + channel.name,
                                    inputLists=['ParticleList_' + daughter for daughter in channel.daughters]))

            ########### SIGNAL PROBABILITY ACTORS #######
            # For FSP we add SignalProbability Actor for the particle itself, and a Resource which provides the conjugated SignalProbability
            if particle.channels == []:
                seq.append(Function(SignalProbability, path='Path', method='Method_' + particle.name, variables='Variables_' + particle.name, particleList='ParticleList_' + particle.name))
                seq.append(Resource('SignalProbability_' + pdg.conjugate(particle.name), 'Dummy', requires=['SignalProbability_' + particle.name]))
            # For non FSP we add SignalProbability Actor for every channel and resources to provide SignalProbability for the whole particle and its conjugated particle as soon as all channels have SignalProbability
            else:
                seq.append(Resource('SignalProbability_' + particle.name, 'Dummy', requires=['SignalProbability_' + channel.name for channel in particle.channels]))
                seq.append(Resource('SignalProbability_' + pdg.conjugate(particle.name), 'Dummy', requires=['SignalProbability_' + channel.name for channel in particle.channels]))
                for channel in particle.channels:
                    seq.append(Function(SignalProbability, path='Path', method='Method_' + particle.name, variables='Variables_' + channel.name, particleList='ParticleList_' + channel.name))

            ############# PRECUT DETERMINATION ############
            # For non FSP Particles we determine the PreCuts usgin the PreCutDistribution and PreCutDetermination Actor
            if particle.channels != []:
                seq.append(Function(PreCutDetermination, name='Name_' + particle.name, pdgcode='PDG_' + particle.name,
                                    channels=['Name_' + channel.name for channel in particle.channels], distribution='PreCutDistribution_' + particle.name, efficiency='Efficiency_' + particle.name))

                daughters = set([daughter for channel in particle.channels for daughter in channel.daughters])
                seq.append(Function(PreCutDistribution, path='Path', particle='Particle_' + particle.name,
                                    daughterLists=['ParticleList_' + daughter for daughter in daughters],
                                    daughterSignalProbabilities=['SignalProbability_' + daughter for daughter in daughters]))

        # We've arrived at the climax of our algorithm. The great finale!
        # Resolve dependencies of the Actors, by extracting step by step the Actors for which
        # all their requirements are provided.
        # Execute these Actors and store their results in results. These results are then used
        # to provide the required arguments of the following resources.
        results = dict()
        while len(seq) != 0:
            current = [item for item in seq if all(r in results for r in item.requires)]
            seq = [item for item in seq if not all(r in results for r in item.requires)]
            for item in current:
                arguments = [results[r] for r in item.requires]
                results.update(item(*arguments))
                #print 'Called ', item.name, ' with arguments ', arguments
            if current == []:
                break

        # Print unresolved Actors for debugging perposes
        for x in seq:
            print x.name, 'needs', [r for r in x.requires if r not in results]
