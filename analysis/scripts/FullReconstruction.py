#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Thomas Keck 2014

from basf2 import *
import modularAnalysis
import pdg

import ROOT

import hashlib
import os


class Channel(object):

    """ Represents a decay channel of a particle, contains the particle, list of daughter particles """

    def __init__(self, particle, daughters):
        """
        Creates a new decay channel for a given particle
            @param particle reference to the particle object which contains this channel
            @param daughters list of daughter particles
        """

        ## List of daughter particles
        self.daughters = daughters
        ## Name of the channel
        self.name = particle.name + ' ->' + ' '.join(self.daughters)


class Particle(object):

    """ Contains name and all decay channels of a particle, also the particle object knows which variables and method are used to reconstruct it. """

    def __init__(self, name, variables, method):
        """
        Creates a Particle without any decay channels. To add decay channels use addChannel method.
            @param name is the correct pdg name as a string of the particle
            @param variables is a list of variables which are used to classify the particle signal probability variables of childs are automatically added later.
            @param method is a tuple (name, type, config) of the method used to classify the particle.
        """

        ## The name of the particle as correct pdg name e.g. K+, pi-, D*0
        self.name = name
        ## A list of variables which are used to classify the particle
        self.variables = variables
        ## A tuple (name, type, config) of the  method used to classify the particle.
        self.method = method
        ## Decay channels, added by addChannel method
        self.channels = []

    def addChannel(self, daughters):
        """
        Appends a new decay channel to the Particle object.
        @param daughters is a list of pdg particle names
        """

        self.channels.append(Channel(self, daughters))


class FullReconstruction:

    """
    Contains all particles which are reconstructed by the hierachical full reconstruction algorithm. Steers the training and reconstruction.
    """

    def __init__(self):
        """
        Creates empty FullReconstruction object -> use addParticle to add particles to the FullReoncstruction setup
        """

        ## A list of particle objects, added with addParticle
        self.particles = []

    def addParticle(self, particle):
        """
        Appends a Particle object to the FullReconstruction setup
        @param particle particle which is added to the full reconstruction chain
        """

        self.particles.append(particle)

    def run(self, path):
        """
        @param path  modules are added to this path
        """

        # Add all resources to the sequence
        seq = [Resource('Path', path)]
        for particle in self.particles:
            seq += [Resource('Name_' + name, name) for name in [particle.name] + [channel.name for channel in particle.channels]]
            seq += [Resource('PDG_' + particle.name, pdg.from_name(particle.name))]
            seq += [Resource('Channels_' + particle.name, particle.channels)]
            if particle.channels == []:
                seq += [Resource('Variables_' + particle.name, particle.variables)]
                seq += [Resource('Method_' + particle.name, particle.method)]
                seq += [Resource('Final_' + particle.name, particle.name)]
                seq += [Resource('Final_' + pdg.conjugate(particle.name), pdg.conjugate(particle.name))]
                seq += [Classify(particle, particle.name)]
            else:
                seq += [Resource('Variables_' + channel.name, particle.variables + ['childProb{i}'.format(i=i) for i in range(0,
                        len(channel.daughters))]) for channel in particle.channels]
                seq += [Resource('Method_' + channel.name, particle.method) for channel in particle.channels]
                seq += [Resource('Final_' + particle.name, particle.name, ['MassHistogram_' + particle.name])]
                seq += [Resource('Final_' + pdg.conjugate(particle.name), pdg.conjugate(particle.name), ['MassHistogram_'
                        + particle.name])]
                seq += [Classify(particle, channel.name) for channel in particle.channels]
                seq += [DetermineMassCuts(particle)]
                seq += [MassHistogram(particle)]
                seq += [Combine(particle, channel) for channel in particle.channels]
            seq += [Select(particle)]

        # Resolve dependencies of the resources, by extracting step by step the resources for which
        # all their requirements are provided.
        solved = []
        provided = set()
        while len(seq) != 0:
            current = [item for item in seq if all(r in provided for r in item.requires)]
            seq = [item for item in seq if not all(r in provided for r in item.requires)]
            if current == []:
                break
            provided |= set([feature for item in current for feature in item.provides])
            solved += current
        unsolved = seq

        for x in unsolved:
            print x, 'needs', [r for r in x.requires if r not in provided]

        # Run the resources and store their results in results. These results are then used
        # to provide the required arguments of the following resources.
        results = dict()
        for item in solved:
            arguments = [results[r] for r in item.requires]
            if any([argument is None for argument in arguments]):
                B2WARNING('One or more resources requested are missing. Breaking loop.')
                break
            hash = hashlib.sha1(str([str(argument) for argument in arguments])).hexdigest()
            results.update(zip(item.provides, item(hash, *arguments)))


class Resource(object):

    def __init__(self, name, x, requires=None):

        self.provides = [name]
        self.requires = (requires if requires is not None else [])
        self.x = x

    def __call__(self, hash, *requirements):
        return [self.x]


class Classify(object):

    def __init__(self, particle, name):
        self.provides = ['Classify_' + name]
        self.requires = ['Path', 'Method_' + name, 'Variables_' + name, 'Name_' + name, 'ParticleList_' + name, 'Final_' + particle.name]

    def __call__(self, hash, path, method, variables, name, particleList, final):

        filename = 'weights/{name}_{hash}_{method}.weights.xml'.format(name=name, hash=hash, method=method[0])
        if os.path.isfile(filename):
            expert = register_module('TMVAExpert')
            expert.set_name('TMVAExpert_' + name)
            expert.param('identifier', name)
            expert.param('method', method)
            expert.param('target', 'SignalProbability')
            expert.param('listNames', [particleList])
            path.add_module(expert)
            return [hash]
        else:
            teacher = register_module('TMVATeacher')
            teacher.set_name('TMVATeacher_' + name)
            teacher.param('identifier', name)
            teacher.param('methods', [method])
            teacher.param('factoryOption', '!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification')
            teacher.param('variables', variables)
            teacher.param('target', 'isSignal')
            teacher.param('listNames', [particleList])
            path.add_module(teacher)
            return [None]


class Select(object):

    def __init__(self, particle):
        self.provides = ['ParticleList_' + particle.name, 'ParticleList_' + pdg.conjugate(particle.name)]
        self.requires = ['Path', 'Name_' + particle.name, 'PDG_' + particle.name]
        self.requires += ['ParticleList_' + channel.name for channel in particle.channels]
        self.requires += ['Final_' + daughter for channel in particle.channels for daughter in channel.daughters]

    def __call__(self, hash, path, name, pdgcode, *args):

        # Select all the reconstructed (or loaded) particles with this pdg into one list.
        # Match the MCTruth, so we have the information if the particle was reconstructed correctly
        name = pdg.to_name(pdgcode)
        modularAnalysis.selectParticle(name, pdgcode, [], path=path)
        modularAnalysis.matchMCTruth(name, path=path)
        # Because anti-particles aren't handled properly yet we have to select the charge conjugate particles by hand.
        # For Pi0 and Photons this doesn't make sense, so we exclude them here by hand.
        if not pdgcode == 111 and not pdgcode == 22:
            barname = pdg.to_name(-pdgcode)
            modularAnalysis.selectParticle(barname, -pdgcode, [], path=path)
            modularAnalysis.matchMCTruth(barname, path=path)
        else:
            barname = name
        return [name, barname]


class Combine(object):

    def __init__(self, particle, channel):
        ## Provides ParticleList for the given channel
        self.provides = ['ParticleList_' + channel.name]
        ## Requires ParticleLists of all daughter particles and
        ## Final Flag for all daughters, to ensure that the ParticleLists of the daughter objects
        ## were generated with the correct cuts determined from the MassHistogram.
        self.requires = ['Path', 'Name_' + channel.name, 'PDG_' + particle.name, 'Cut_' + channel.name]
        self.requires += ['ParticleList_' + daughter for daughter in channel.daughters]
        self.requires += ['Final_' + daughter for daughter in channel.daughters]

    def __call__(self, hash, path, name, pdg, cut, *particleLists):

        pmake = register_module('ParticleCombiner')
        pmake.set_name('ParticleCombiner_' + name)
        pmake.param('PDG', pdg)
        pmake.param('ListName', name)
        pmake.param('InputListNames', particleLists[0:len(particleLists) / 2])
        pmake.param(cut['variable'], cut['range'])
        path.add_module(pmake)
        return [name]


class DetermineMassCuts(object):

    def __init__(self, particle):
        self.provides = ['Cut_' + channel.name for channel in particle.channels]
        self.requires = ['Name_' + particle.name, 'PDG_' + particle.name, 'Channels_' + particle.name]

    def __call__(self, hash, name, pdgcode, channels):

        filename = 'Reconstruction_{name}_{hash}.root'.format(name=name, hash=hash)
        if not os.path.isfile(filename):
            mass = pdg.get(pdgcode).Mass()
            return [{'variable': 'MassCut', 'range': (mass - mass * 1.0 / 10.0, mass + mass * 1.0 / 10.0)} for channel in
                    channels]

        # For every channel we get the signal and background distribution from the Reconstruction file,
        # search the position of the peak of the signal distribution (this peak is used as the starting point for the cut search)
        # and calculate the S/B ratio by cloning signal and dividing it by bckgrd.
        keys = [channel.name for channel in channels]
        file = ROOT.TFile(filename, 'UPDATE')
        signal = dict([(key, file.Get(key + '_M_signal_histogram')) for key in keys])
        bckgrd = dict([(key, file.Get(key + '_M_background_histogram')) for key in keys])
        maxpos = dict([(key, value.GetBinCenter(value.GetMaximumBin())) for (key, value) in signal.iteritems()])
        ratio = dict([(key, value.Clone(key + '_M_ratio_histogram')) for (key, value) in signal.iteritems()])
        for key in keys:
            ratio[key].Divide(bckgrd[key])
            ratio[key].Write('', ROOT.TObject.kOverwrite)

        # Create a Spline fit of S/B ratio and wrap in a ROOT TF1 function, so we can perform GetX() on it
        spline = dict([(key, ROOT.TSpline3(value)) for (key, value) in ratio.iteritems()])
        spline_ROOT = dict([(key, ROOT.TF1(key + '_M_func', lambda x, key=key: spline[key].Eval(x[0]), 0, 100, 0)) for key in
                           keys])

        # Calculate a cut on the y-axis, so that only ncandidates * percentage candidates survive.
        # Where ncandidates is the total number of candidates.
        # Therefore create a function which calculates the number of candidates after a cut on the y-axis
        # was performed and wrap it into a ROOT TF1 function, so we can perform GetX() on it.
        ncandidates = sum([value.Integral(0, value.GetNbinsX()) for value in signal.values() + bckgrd.values()])

        def ycut_to_xcuts(key, cut):
            return (spline_ROOT[key].GetX(cut, 0, maxpos[key]), splint_ROOT[key].GetX(cut, maxpos[key], 100))

        def ncandidates_after_cut(cut):
            nevents = 0
            for key in keys:
                (a, b) = ycut_to_xcuts(key, cut)
                nevents += signal[key].Integral(signal[key].FindBin(a), signal[key].FindBin(b))
                nevents += bckgrd[key].Integral(bckgrd[key].FindBin(a), bckgrd[key].FindBin(b))
            return nevents

        ncandidates_after_cut_ROOT = ROOT.TF1(filename + '_mass_cut_func', ncandidates_after_cut, 0, 1, 0)
        ycut = ncandidates_after_cut_ROOT.GetX(ncandidates * percentage, 0.0, 1.0)

        # Return the calculates ChannelCuts
        return [{'variable': 'MassCut', 'range': ycut_to_xcuts(key, cut), 'file': filename} for key in keys]


class MassHistogram(object):

    """
    Creates ROOT file with invariant mass distribution of every channel (signal/background)
    for a given particle.
    """

    def __init__(self, particle):
        ## Provides MassHistogram for given particle
        self.provides = ['MassHistogram_' + particle.name]
        ## Requires the ParticleList of the particle itself (to ensure that Select already matched the MCTruth of the channels
        ## and ParticleList of all channels.
        self.requires = ['Path', 'Name_' + particle.name, 'ParticleList_' + particle.name]
        self.requires += ['ParticleList_' + channel.name for channel in particle.channels]

    def __call__(self, hash, path, name, *args):

        # Check if the file is available. If the file isn't available yet, create it with
        # the HistMaker Module and return None. If a function is called which depends on
        # the MassHistogram, the process will stop, because the MassHistogram isn't provided (None)
        filename = 'Reconstruction_{name}_{hash}.root'.format(name=name, hash=hash)
        if os.path.isfile(filename):
            return [filename]
        else:
            histMaker = register_module('HistMaker')
            histMaker.set_name(filename)
            histMaker.param('file', filename)
            histMaker.param('histVariables', [('M', 1000, 0, 10)])
            histMaker.param('truthVariable', 'isSignal')
            histMaker.param('listNames', args)
            path.add_module(histMaker)
        return [None]
