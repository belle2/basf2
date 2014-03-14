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
#   2. Make sure your return value depends on all the used arguments, easiest way to accomplish this is using the createHash function
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
        print 'Called ' + self.name + ' with following arguments'
        print kwargs
        result = self.func(**kwargs)
        print 'Result is:'
        print result
        return result


def createHash(*args):
    """
    Creates a unique has which depends on the given arguments
        @args arguments the hash depends on
    """
    return hashlib.sha1(str([str(v) for v in args])).hexdigest()


############### Now here are the Actors, every Actor implements a part of the FullReconstruction algorithm ################

def ParticleList(path, name, pdgcode, particleLists, criteria=[]):
    """
    Creates a ParticleList gathering up all particles with the given pdgcode.
        @param path the basf2 path
        @param name name of outputList
        @param pdgcode the pdgcode of the particle
        @param particleLists additional requirements before the Particles can be gathered. E.g. All ParticleList of created by the ParticleListFromChannel exist for this particle
        @param criteria filter criteria
    """
    # Select all the reconstructed (or loaded) particles with this pdg into one list.
    list_name = name + createHash(name, pdgcode, particleLists, criteria)
    modularAnalysis.selectParticle(list_name, pdgcode, criteria, path=path)
    return {'ParticleList_' + name: list_name}


def ParticleListFromChannel(path, pdgcode, name, preCut, inputLists, isIgnored):
    """
    Creates a ParticleList by combining other particleLists via the ParticleCombiner module.
        @param path the basf2 path
        @param pdgcode pdgcode of the particle which is reconstructed
        @param name name of the channel or particle which shall be combined
        @param preCut cuts which are applied before the combining of the particles
        @param inputLists the inputs lists which are combined
        @param true if the channel is ignored due to low statistics
    """
    list_name = name + createHash(pdgcode, name, preCut, inputLists, isIgnored)
    pmake = register_module('ParticleCombiner')
    pmake.set_name('ParticleCombiner_' + name)
    pmake.param('PDG', pdgcode)
    pmake.param('ListName', list_name)
    pmake.param('InputListNames', inputLists)
    if isIgnored:
        pmake.param('MassCut', (1000, 1000))
    else:
        pmake.param(preCut['variable'], preCut['range'])

    path.add_module(pmake)
    modularAnalysis.matchMCTruth(list_name, path=path)
    return {'ParticleList_' + name: list_name}


def SignalProbability(path, method, variables, name, particleList, target='SignalProbability', isIgnored=False):
    """
    Calculates the SignalProbability of a ParticleList. If the needed experts aren't available they're created.
        @param path the basf2 path
        @param method method given to the TMVAInterface (see TMVAExpert and TMVATeacher)
        @param variables used for classification (see VariableManager)
        @param name of the particle or channel which is classified
        @param particleList the particleList which is used for training and classification
        @param target name under which the expert stores the result in the extra info of the particle
        @param true if the channel is ignored due to low statistics
    """
    # Create hash with all parameters on which this training depends
    hash = createHash(method, variables, particleList, isIgnored)
    filename = 'weights/{particleList}_{hash}_{method}.weights.xml'.format(particleList=particleList, hash=hash, method=method[0])
    if isIgnored:
        return {target + '_' + name: hash}
    if os.path.isfile(filename):
        expert = register_module('TMVAExpert')
        expert.set_name('TMVAExpert_' + particleList)
        expert.param('identifier', particleList + '_' + hash)
        expert.param('method', method[0])
        expert.param('target', target)
        expert.param('listNames', [particleList])
        path.add_module(expert)
        print target + '_' + name
        return {target + '_' + name: hash}
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


def CombineSignalProbabilities(path, name, targets, fsp_histogram):
    """
    Combines the SignalProbability of multiple trainings
        @param name of the particle
        @param targets the targets which are combined
        @param filename of the fsp histogram with the MC-pdg distributions
    """
    hash = createHash(name, targets, fsp_histogram)
    # TODO Read out fsp_histograms and calculate the correct signal probability
    # p = 1/( f_S + sum( f_Bi * ( 1 - Y_i) ) )
    calculator = register_module('VariableCalculator')
    calculator.param('fractions', [0.01, 0.89, 0.01, 0.01, 0.08])
    calculator.param('extraInfoVars', targets)  # TODO Use correct targets, targets contain hashes!
    calculator.param('newExtraInfoVar', 'SignalProbability')
    return {'SignalProbability_' + name: hash}


def CreateFSPHistogram(path, name, particleList):
    """
    Creates ROOT File with MC-pdgcode distributionof the given particleList and returns the filename
    This is used by the CombineSignalProbabilities to calculate the correct SignalProbability
        @param path the basf2 path
        @param name of the particle
        @param particleList for which this histogram is created
        @param daughterSignalProbabilities all daughter particles need a SignalProbability
    """
    # Check if the file is available. If the file isn't available yet, create it with
    # the HistMaker Module and return Nothing. If a function is called which depends on
    # the PreCutHistogram, the process will stop, because the PreCutHistogram isn't provided.
    hash = createHash(name, particleList)
    filename = 'Reconstruction_{name}_{hash}.root'.format(name=name, hash=hash)
    if os.path.isfile(filename):
        return {'FSPHistogram_' + name: filename}
    else:
        histMaker = register_module('HistMaker')
        histMaker.set_name(filename)
        histMaker.param('file', filename)
        histMaker.param('histVariables', [('mcPDG', 1000, 0, 1000)])
        histMaker.param('truthVariable', 'isSignal')
        histMaker.param('listNames', [particleList])
        path.add_module(histMaker)
    return {}


def CreatePreCutMassHistogram(path, particle, daughterLists):
    """
    Creates ROOT file with invariant mass and signal probability product histogram of every channel (signal/background)
    for a given particle, before any intermediate cuts are applied. For now this class uses the ParticleCombiner and HistMaker
    modules to generate these distributions, later we should join these modules together into one fast PreCutHistogram module.
        @param path the basf2 path
        @param particle object for which this histogram is created
        @param daughterLists all particleLists of all the daughter particles in all channels
    """
    # Check if the file is available. If the file isn't available yet, create it with
    # the HistMaker Module and return Nothing. If a function is called which depends on
    # the PreCutHistogram, the process will stop, because the PreCutHistogram isn't provided.
    hash = createHash(particle.name, daughterLists)
    filename = 'Reconstruction_{name}_{hash}.root'.format(name=particle.name, hash=hash)
    if os.path.isfile(filename):
        return {'PreCutHistogram_' + particle.name: filename}
    else:
        # Combine all the particles according to the decay channels
        mass = pdg.get(pdg.from_name(particle.name)).Mass()
        for channel in particle.channels:
            pmake = register_module('ParticleCombiner')
            pmake.set_name('ParticleCombiner_' + channel.name)
            pmake.param('PDG', pdg.from_name(particle.name))
            pmake.param('ListName', channel.name)
            inputListNames = []
            for daughter in channel.daughters:
                listName = filter(lambda x: daughter == x[0:len(daughter)], daughterLists)
                #TODO Raise exception
                if len(listName) > 1:
                    print 'WARNING'
                inputListNames += listName
            pmake.param('InputListNames', inputListNames)
            pmake.param('MassCut', (mass - mass * 1.0 / 10.0, mass + mass * 1.0 / 10.0))
            path.add_module(pmake)
            modularAnalysis.matchMCTruth(channel.name, path=path)
        histMaker = register_module('HistMaker')
        histMaker.set_name(filename)
        histMaker.param('file', filename)
        histMaker.param('histVariables', [('M', 100, mass - mass * 1.0 / 20.0, mass + mass * 1.0 / 20.0)])
        histMaker.param('truthVariable', 'isSignal')
        histMaker.param('listNames', [channel.name for channel in particle.channels])
        path.add_module(histMaker)
    return {}


def CreatePreCutProbHistogram(path, particle, daughterLists, daughterSignalProbabilities):
    """
    Creates ROOT file with invariant mass and signal probability product histogram of every channel (signal/background)
    for a given particle, before any intermediate cuts are applied. For now this class uses the ParticleCombiner and HistMaker
    modules to generate these distributions, later we should join these modules together into one fast PreCutHistogram module.
        @param path the basf2 path
        @param particle object for which this histogram is created
        @param daughterLists all particleLists of all the daughter particles in all channels
        @param daughterSignalProbabilities all daughter particles need a SignalProbability
    """
    # Check if the file is available. If the file isn't available yet, create it with
    # the HistMaker Module and return Nothing. If a function is called which depends on
    # the PreCutHistogram, the process will stop, because the PreCutHistogram isn't provided.
    hash = createHash(particle.name, daughterLists, daughterSignalProbabilities)
    filename = 'Reconstruction_{name}_{hash}.root'.format(name=particle.name, hash=hash)
    if os.path.isfile(filename):
        return {'PreCutHistogram_' + particle.name: filename}
    else:
        # Combine all the particles according to the decay channels
        mass = pdg.get(pdg.from_name(particle.name)).Mass()
        for channel in particle.channels:
            pmake = register_module('ParticleCombiner')
            pmake.set_name('ParticleCombiner_' + channel.name)
            pmake.param('PDG', pdg.from_name(particle.name))
            pmake.param('ListName', channel.name)
            inputListNames = []
            for daughter in channel.daughters:
                listName = filter(lambda x: daughter == x[0:len(daughter)], daughterLists)
                #TODO Raise exception
                if len(listName) > 1:
                    print 'WARNING'
                inputListNames += listName
            pmake.param('InputListNames', inputListNames)
            pmake.param('MassCut', (mass - mass * 1.0 / 10.0, mass + mass * 1.0 / 10.0))
            pmake.param('cutsOnProduct', {'SignalProbability': (0.0, 1.0)})
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


def PreCutMassDetermination(name, pdgcode, channels, preCut_Histogram, efficiency):
    """
    Determines the PreCuts for all the channels of a particle. The cuts are chosen as follows:
        1. The maximum of every signal invariant mass preCut_Histogram is isolated
        2. The ratio of signal/background invariant mass preCut_Histogram is calculated and fitted with splines
        3. A cut on the y-axis of S/B is performed, so that a certain signal efficiency is garantueed.
        4. The cut on the y-axis is converted to a minimum and maximum cut on the x-axis for every channel.
        @param name name of the particle
        @param pdgcode pdgcode of the particle
        @param channels list of the names of all the channels
        @param preCut_Histogram filename of the histogram file created by PreCutDistribution
        @param signal efficiency for this particle
    """

    # For every channel we get the signal and background histogram from the Reconstruction file,
    # search the position of the peak of the signal histogram (this peak is used as the starting point for the cut search)
    # and calculate the S/B ratio by cloning signal and dividing it by bckgrd.
    hash = createHash(name, pdgcode, channels, preCut_Histogram, efficiency)
    file = ROOT.TFile(preCut_Histogram, 'UPDATE')
    signal = dict([(channel, file.Get(channel + '_M_signal_histogram')) for channel in channels])
    bckgrd = dict([(channel, file.Get(channel + '_M_background_histogram')) for channel in channels])
    maxpos = dict([(channel, value.GetBinCenter(value.GetMaximumBin())) for (channel, value) in signal.iteritems()])
    ratio = dict([(channel, value.Clone(channel + '_M_ratio_histogram')) for (channel, value) in signal.iteritems()])
    for channel in channels:
        ratio[channel].Divide(bckgrd[channel])
        ratio[channel].Write('', ROOT.TObject.kOverwrite)

    # Create a Spline fit of S/B ratio and wrap in a ROOT TF1 function, so we can perform GetX() on it
    spline = dict([(channel, ROOT.TSpline3(value)) for (channel, value) in ratio.iteritems()])
    spline_ROOT = dict([(channel, ROOT.TF1(hashlib.sha1(channel).hexdigest() + '_M_func', lambda x, channel=channel: spline[channel].Eval(x[0]), 0, 100, 0)) for channel in channels])

    # Calculate a cut on the y-axis, so that only at least nsignal * efficency signal events survive.
    # Therefore create a function which calculates the number of signal events after a cut on the y-axis
    # was performed and wrap it into a ROOT TF1 function, so we can perform GetX() on it.
    nsignals = sum([value.Integral(0, value.GetNbinsX()) for value in signal.values()])

    def ycut_to_xcuts(channel, cut):
        return (spline_ROOT[channel].GetX(cut, 0, maxpos[channel]), spline_ROOT[channel].GetX(cut, maxpos[channel], 100))

    def nsignals_after_cut(cut):
        nevents = 0
        for channel in channels:
            (a, b) = ycut_to_xcuts(channel, cut[0])
            nevents += signal[channel].Integral(signal[channel].FindBin(a), signal[channel].FindBin(b))
        return nevents

    nsignals_after_cut_ROOT = ROOT.TF1(hashlib.sha1(preCut_Histogram).hexdigest() + '_mass_cut_func', nsignals_after_cut, 0, 1, 0)
    ycut = nsignals_after_cut_ROOT.GetX(nsignals * efficiency, 0.0, 1.0)

    # Return the calculates ChannelCuts
    result = {'PreCut_' + channel: {'variable': 'MassCut', 'range': ycut_to_xcuts(channel, ycut)} for channel in channels}
    result.update({'Hash': hash})
    # TODO Add IsIgnored_{channel} = True, so SignalProbability and ParticleCombiner can test this flag
    for channel in channels:
        (a, b) = ycut_to_xcuts(channel, ycut)
        if abs(b - a) < ratio[channel].GetBinWidth(signal[channel].GetMaximumBin()) or signal[channel].Integral(signal[channel].FindBin(a), signal[channel].FindBin(b)) < 100 or bckgrd[channel].Integral(bckgrd[channel].FindBin(a), bckgrd[channel].FindBin(b)) < 100:
            result.update({'IsIgnored_' + channel: True})
    return result


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
            # Add all information the user has provided as resources to the sequence, so our function can require them
            seq.append(Resource('Name_' + particle.name, particle.name))
            seq.append(Resource('PDG_' + particle.name, pdg.from_name(particle.name)))
            seq.append(Resource('Method_' + particle.name, particle.method))
            seq.append(Resource('Efficiency_' + particle.name, particle.efficiency))
            seq.append(Resource('Particle_' + particle.name, particle))
            # If the particle isn't self conjugated we add the name and the pdg also for the antiparticle
            name_conj = pdg.conjugate(particle.name)
            if particle.name != name_conj:
                seq.append(Resource('Name_' + name_conj, name_conj))
                seq.append(Resource('PDG_' + name_conj, pdg.from_name(name_conj)))
            # Add Variables:
            # If the particle is a FSP, we add the variables for the particle
            # If the particle has decay channels, we add the name and the variables for these decay channels.
            # In addition childProb variables are added for each channel automatically here.
            # The IsIgnored flag is used to deactivate a channel if there's not enough statistics.
            if particle.channels == []:
                seq.append(Resource('Variables_' + particle.name, particle.variables))
            for channel in particle.channels:
                seq.append(Resource('Name_' + channel.name, channel.name))
                seq.append(Resource('Variables_' + channel.name, particle.variables + ['childProb{i}'.format(i=i) for i in range(0, len(channel.daughters))]))
                seq.append(Resource('IsIgnored_' + channel.name, False))

            ########### RECONSTRUCTION ACTORS ##########
            # The Reconstruction part of the FullReconstruction:
            # 1. Load the FSP (via ParticleList),
            # 2. Create a particle list for each channel of the non-FSP (via ParticleListFromChannel), depending an all daughter ParticleLists of this channel
            # 3. Gather up the different ParticleLists for each channel, into a single ParticleList for the particle itself, depending on all channel ParticleLists
            # 4. If the particle isn't self conjugated we gather up the anti-particles as well, because anti-particles aren't handlet properly yet.  FIXME
            channelParticleLists = ['ParticleList_' + channel.name for channel in particle.channels]
            seq.append(Function(ParticleList, path='Path', name='Name_' + particle.name, pdgcode='PDG_' + particle.name, particleLists=channelParticleLists))
            for channel in particle.channels:
                seq.append(Function(ParticleListFromChannel, path='Path', pdgcode='PDG_' + particle.name, name='Name_' + channel.name, preCut='PreCut_' + channel.name,
                                    inputLists=['ParticleList_' + daughter for daughter in channel.daughters], isIgnored='IsIgnored_' + channel.name))
            if particle.name != name_conj:
                seq.append(Function(ParticleList, path='Path', name='Name_' + name_conj, pdgcode='PDG_' + name_conj, particleLists=channelParticleLists))

            ############# PRECUT DETERMINATION ############
            # Intermediate PreCut part of FullReconstruction algorithm.
            # To surpress the computational cost due to the combinatoric, we apply efficient PreCuts before combining the daughter particles of each channel in the reconstruction part.
            # The PreCuts are chosen, so that a user-defined total signal efficiency for this particle is met.
            # 1. Create Histograms with the invariant mass or signal probability distribution for each channel, as soon as all daughter ParticleLists and daughter SignalProbabilities are available.
            # 2. If the Histogram is available calculate the PreCuts with the PreCutDetermination function. As soon as the PreCuts are available the particle can be reconstructed.
            # TODO Implement user choice -> Cut on invariant mass or cut on product of signal Probability
            if particle.channels != []:
                daughters = set([daughter for channel in particle.channels for daughter in channel.daughters])
                seq.append(Function(CreatePreCutMassHistogram, path='Path', particle='Particle_' + particle.name,
                                    daughterLists=['ParticleList_' + daughter for daughter in daughters]))
                #seq.append(Function(CreatePreCutProbHistogram, path='Path', particle='Particle_' + particle.name,
                #                    daughterLists=['ParticleList_' + daughter for daughter in daughters],
                #                    daughterSignalProbabilities=['SignalProbability_' + daughter for daughter in daughters]))
                seq.append(Function(PreCutMassDetermination, name='Name_' + particle.name, pdgcode='PDG_' + particle.name,
                                    channels=['Name_' + channel.name for channel in particle.channels], preCut_Histogram='PreCutHistogram_' + particle.name, efficiency='Efficiency_' + particle.name))

            ########### SIGNAL PROBABILITY ACTORS #######
            # The classifier part of the FullReconstruction. Here one has to distinguish between [e+, mu+, pi+, K+, p], other FSPs and non-FSPs.
            # All the classifiers depend on the method, variables and the ParticleList (either for the particle or for the channel).
            # [e+,mu+,pi+,K+,p]: All the tracks seen in the detector are generated by one of these particles.
            #                    To get a better spearation we train these particles against each other and combine the result later (via CombinSignalProbabilities)
            # To accomplish this:
            #   1. Create FSP Histogram with the real distribution of signal and background particles in the ParticleList
            #   2. Create new ParticleList which contains only the the signal component and one background component e.g. e+ vs. mu+
            #   3. Train a classifier for this ParticleList and calculate the SignalProbability with this classifier
            #   4. Combine the different SignalProbabilities to one, using the signal and background fractions of the FSP Histogram.
            # TODO Disabled at the moment
            opponents = []  # ['e+','mu+','pi+','K+']
            if particle.channels == [] and particle.name in opponents:
                seq.append(Function(CreateFSPHistogram, path='Path', name='Name_' + particle.name, particleList='ParticleList_' + particle.name))
                for opponent in opponents:
                    if particle.name == opponent:
                        continue
                    tmp = particle.name + '_against_' + opponent
                    seq.append(Resource('Name_' + tmp, tmp))
                    seq.append(Function(ParticleList, path='Path', name='Name_' + tmp, pdgcode='PDG_' + particle.name, particleLists=['Name_' + particle.name], criteria=""))  # TODO Add criterium
                    seq.append(Function(SignalProbability, path='Path', method='Method_' + particle.name, variables='Variables_' + particle.name,
                                        name='Name_' + particle.name, particleList='ParticleList_' + tmp, target='Name_' + tmp))
                seq.append(Function(CombineSignalProbabilities, path='Path', name='Name_' + particle.name,
                                    targets=[particle.name + '_against_' + opponent + '_' + particle.name for opponent in opponents if particle.name != opponent], fsp_histogram='FSPHistogram_' + name))

            # Other FSP: Other FSP like gammas and pi0, are trained with a single classifier.
            if particle.channels == [] and not particle.name in opponents:
                seq.append(Function(SignalProbability, path='Path', method='Method_' + particle.name, variables='Variables_' + particle.name,
                                    name='Name_' + particle.name, particleList='ParticleList_' + particle.name))

            # Non FSP:
            #   1. Train classifier for every decay channel of the particle.
            #   2. As soon as all SignalProbabilities for all channels are available the resoure SignalProbability_{particleName} is unlocked.
            if particle.channels != []:
                for channel in particle.channels:
                    seq.append(Function(SignalProbability, path='Path', method='Method_' + particle.name, variables='Variables_' + channel.name,
                                        name='Name_' + channel.name, particleList='ParticleList_' + channel.name, isIgnored='IsIgnored_' + channel.name))
                seq.append(Resource('SignalProbability_' + particle.name, 'Dummy', requires=['SignalProbability_' + channel.name for channel in particle.channels]))

            # If the classifiers provide the signal probability, they also provide the signal probability for the anti particles, so we add
            # a dummy resource which depends only on the SignalProbability of the particle and provides the SignalProbability for the anti-particle.
            seq.append(Resource('SignalProbability_' + pdg.conjugate(particle.name), 'Dummy', requires=['SignalProbability_' + particle.name]))

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
