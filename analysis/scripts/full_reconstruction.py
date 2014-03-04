#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Thomas Keck 2014

from modularAnalysis import *
from basf2 import *
import pdg
import os
import hashlib
import cut_determination

import argparse


class Particle:

    """
    Contains name, pdg and all decay channels of a particle,
    also the particle object knows which variables and methods are used to
    reconstruct it.
    """

    def __init__(self, name, variables=None, methods=None):
        """
        Creates a Particle without any decay channels. To add
        decay channels use addChannel method.
        name is the correct pdg name as a string of the particle
        variables is a list of variables which are used to classify the
            particle signal probability variables of childs are automatically
            added later.
        methods is a list of tuples (name, type, config) of the
            methods used to classify the particle.
        """

        ## The name of the particle as correct pdg name e.g. K+, pi-, D*0
        self.name = name
        ## PDG code of the particle
        self.pdg = pdg.from_name(name)
        # Set variables and method to empty list, if None were given.
        # We can't use [] directly as default parameter, because this would
        # give every instance of the Particle class the same(!) variable and
        # method list, because default parameters are only evaluated once!
        if variables is None:
            variables = []
        if methods is None:
            methods = []
        ## A list of variables which are used to classify the particle
        self.variables = variables
        ## A list of tuples (name, type, config) of the  methods used to
        ## classify the particle.
        self.methods = methods
        # At this point there are no known decay channels.
        # If a particle has no decay channels by the time you reconstruct it,
        # it is considered as a final state particle.
        ## Decay channels, added by addChannel method, each channel is a list
        ## pdg particle names
        self.channels = []
        ## The hashseed is set by the FullReconstruction class to implement
        ## the dependency of the particle to the previous stages.
        self.hashseed = ''

        ## Ignored channels aren't trained and used, because there isn't enough
        ## statistics for a training
        self.ignored_channels = []

    def addChannel(self, channel):
        """
        Appends a new decay channel to the Particle object. A decay channel is
        a list of pdg particle names
        """

        self.channels.append(channel)

    def dependencies(self):
        """
        Returns a list of abs(pdg_code) of all daughter particle pdgs
        """

        # Create list of pdg names in all the channels, use set to make the
        # names unique and transform them to abs(pdg codes).
        return map(abs, pdg.from_names(list(set(sum([c for c in self.channels], [])))))

    def to_string(self, channel):
        """
        Convert a decay channel (list of particle pdg codes), to a readable
        string
        """

        string = pdg.to_name(self.pdg)
        if len(channel) > 1:
            string += ' ->'
            for d in channel:
                string += ' ' + pdg.to_name(d)
        return string

    def getHistHash(self):
        """
        Determine hash for the cut histograms, the hash depends on the
        previous stages (hashseed) and on the decay channels.
        """

        return hashlib.sha1(self.hashseed + str(self.channels)).hexdigest()

    def getHistFilename(self):
        """
        Returns filename where histograms used for cut determination are
        stored.
        """

        return 'HistMaker_{pdg}_{hash}.root'.format(pdg=self.pdg, hash=self.getHistHash())

    def getWeightHash(self):
        """
        Hash includes the dependencies of the previous stages (hashseed), the
        channels, variables and the method tuples (name, type, config) of the
        used classifiers.
        """

        return hashlib.sha1(self.hashseed + str(self.channels) + str(self.variables) + str(self.methods)).hexdigest()

    def getWeightFilename(self, channel, method):
        """
        Returns filename where weightfile of TMVA Method is stored.
        """

        return 'weights/{channel}_{hash}_{name}.weights.xml'.format(channel=self.to_string(channel), name=method[0], hash=self.getWeightHash())

    def getCuts(self):
        """
        Since the cut determination is a bit lengthy we do this in a
        sepeterate module called cut_determination.py.
        This module gets the filename with the histograms and the decay
        channel identifiers, and returns the optimal cuts for every
        channel as a map.
        """

        cuts = {}
        # If this is not a final state particle and cut determination histogram
        # is available we calculate the correct cuts
        if not self.channels == [] and os.path.isfile(self.getHistFilename()):
            (cuts_m, ignored_channels) = cut_determination.getCutOnMass(0.01, self.getHistFilename(), [self.to_string(c) for c in self.channels])
            cuts['M'] = cuts_m
        return (cuts, ignored_channels)

    def reconstruct(self, path, mcrun=False):
        """
        Reconstruct the particle in all given decay channels. A particle
        without decay channels is considered as a final state particle.
        If the histograms from which the intermediate cuts are determined
        don't exist we create them first.
        @param path  modules are added to this path
        @param mcrun if True, do not apply any cuts (or determine them)

        The function returns:
            False - if not all data is available at the moment to reconstruct
                the particle -> the needed modules to generate this
                information are loaded into the path.
            True - if the particle can be reconstructed -> the needed modules
                to reconstruct the particle are loaded into the path
        """

        mass = pdg.get(self.pdg).Mass()
        width = pdg.get(self.pdg).Width()

        # If this is not a final state particle and no cut determination
        # histograms available we have to gennerate the histograms first
        if not mcrun and not self.channels == [] and not os.path.isfile(self.getHistFilename()):

            # So first we combine the daughter particles to candidates
            # and apply very loose cuts
            for channel in self.channels:
                print self.to_string(channel)
                pmake = register_module('ParticleCombiner')
                pmake.set_name('ParticleCombiner_' + self.to_string(channel))
                pmake.param('PDG', self.pdg)
                pmake.param('ListName', self.to_string(channel))
                pmake.param('InputListNames', pdg.to_names(channel))
                pmake.param('MassCut', (mass - mass * 1.0 / 10.0, mass + mass * 1.0 / 10.0))
                # pmake.param('cutsOnProduct', {'SignalProbability': (0.01,
                #            1.0)})
                path.add_module(pmake)

                # As a last thing we match the MCTruth, so we have the
                # information if the particle was reconstructed correctly
                # available at the next stage.
                matchMCTruth(self.to_string(channel), path=path)

            # Now we can create histograms of the invariant mass distribution,
            # for all the candidates of the different decay channels.
            # From this histograms we determine the optimal cuts per channel
            # later.
            histMaker = register_module('HistMaker')
            histMaker.set_name(self.getHistFilename())
            histMaker.param('file', self.getHistFilename())
            histMaker.param('histVariables', [('M', 100, mass - mass * 1.0 / 10.0, mass + mass * 1.0 / 10.0)])
            histMaker.param('make2dHists', True)
            histMaker.param('truthVariable', 'isSignal')
            histMaker.param('listNames', [self.to_string(c) for c in self.channels])
            path.add_module(histMaker)

            # Obviously we can't proceed until these histograms are generated,
            # so we stop the reconstruction process at this stage by returning
            # False
            return False

        # If not a final state particle we fetch the cuts and reconstruct
        # the particle in all of its decay channels.
        if not self.channels == []:
            (cuts, ignored_channels) = self.getCuts()
            self.ignored_channels = self.ignored_channels + ignored_channels
            print cuts
            print ignored_channels
            for channel in self.channels:
                if self.to_string(channel) in self.ignored_channels:
                    continue
                print channel
                pmake = register_module('ParticleCombiner')
                pmake.set_name('ParticleCombiner_' + self.to_string(channel))
                pmake.param('PDG', self.pdg)
                pmake.param('ListName', self.to_string(channel))
                pmake.param('InputListNames', pdg.to_names(channel))
                if not mcrun:
                    pmake.param('MassCut', cuts['M'][self.to_string(channel)])
                path.add_module(pmake)

        # Now select all the reconstructed (or loaded) particles with this pdg
        # into one list.
        selectParticle(pdg.to_name(self.pdg), self.pdg, [], path=path)

        # As a last thing we match the MCTruth, so we have the information if
        # the particle was reconstructed correctly available at the next stage.
        matchMCTruth(pdg.to_name(self.pdg), path=path)

        # Because anti-particles aren't handled properly yet we have to select
        # the charge conjugate particles by hand. For Pi0 and Photons this
        # doesn't make sense, so we exclude them here by hand.
        if not self.pdg == 111 and not self.pdg == 22:
            selectParticle(pdg.to_name(-self.pdg), -self.pdg, [], path=path)

        # Return True, since the particle can be reconstructed and we can go on
        # with the next stage.
        return True

    def classify(self, path):
        """
        Classify the particle candidates with the given methods and variables
        in all given decay channels.
        If the experts for the different methods (so weight files for
        NeuroBayes or BDTs) aren't available yet, we need to create them
        first.
        The function returns:
            False - if not all data is available at the moment to classify
                the particle -> the needed modules to generate this
                information are loaded into the path.
            True - if the particle can be calssified -> the needed modules to
                classify the particle are loaded into the path
        """

        # This variable will be set to True, if we encounter a classifier
        # which need to be retrained. If so we can't go on with the next stage.
        training_required = False

        # We also have a classifier for final state particles, which don't have
        # any decay channels. So we add an empty one here, so the loop further
        # down is called once.
        if self.channels == []:
            channels = [[]]
        else:
            channels = self.channels

        # So for every channel we train all the classifiers seperatly.
        for channel in channels:
            # Check if channels is ignored
            if self.to_string(channel) in self.ignored_channels:
                continue
            # Check first if all methods are available, by checking if the
            # weight files are present
            if any([not os.path.isfile(self.getWeightFilename(channel, method)) for method in self.methods]):
                # If one or more files are missing we retrain all methods
                # (this has the advantage that we can easily compare the
                # methods in the test phase of the full reconstruction,
                # later in production phase we will have only one method
                # anyway, so this won't make a difference)
                teacher = register_module('TMVATeacher')
                teacher.set_name('TMVATeacher_' + self.to_string(channel))
                teacher.param('identifier', self.to_string(channel) + '_' + self.getWeightHash())
                teacher.param('methods', self.methods)
                teacher.param('factoryOption', '!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification')
                # Add child probability variables!
                teacher.param('variables', self.variables + ['childProb{i}'.format(i=i) for i in range(0, len(channel))])
                teacher.param('target', 'isSignal')
                teacher.param('listNames', [self.to_string(channel)])
                path.add_module(teacher)

                training_required = True

        if training_required:
            # If a training was required, we need to wait until the expert is
            # created and stop the process at this stage by returning False
            return False

        # If all the needed experts are present, we classify alle the particle
        # candidates.
        for channel in channels:
            if self.to_string(channel) in self.ignored_channels:
                continue
            # For every method we add a TMVAExpert module to the path, the
            # target variable has the same name as the method.
            for method in self.methods:
                expert = register_module('TMVAExpert')
                expert.set_name('TMVAEXPERT_' + self.to_string(channel) + '_' + method[0])
                expert.param('identifier', self.to_string(channel) + '_' + self.getWeightHash())
                expert.param('method', method[0])
                # TODO All methods use the same target variable at the moment
                # we want to use different targets for different methods later
                expert.param('target', 'SignalProbability')
                expert.param('listNames', [self.to_string(channel)])
                path.add_module(expert)

        # At this point all modules to classify the particle candidates are in
        # the path, so we can proceed with the next stage.
        return True


class FullReconstruction:

    """
    Steers the training and reconstruction of all decay channels.
    Create ad instances of this class and add all particles you
    want to reconstruct.
    """

    def __init__(self):
        """
        Creates empty FullReconstruction object -> use addParticle to add
        particles to the FullReoncstruction setup
        """

        ## A list of particle objects, added with addParticle
        self.particles = []

        parser = argparse.ArgumentParser()
        parser.add_argument('-t', '--train', dest='train', action='store_true', help='Train stages')
        parser.add_argument('-n', '--retrain-stage', dest='retrain_stage', type=int, help='Retrain networks in given stage')
        parser.add_argument('-m', '--recreate-stage', dest='recreate_stage', type=int, help='Recreate histograms in given stage')
        parser.add_argument('-c', '--print-cuts', dest='print_cuts', action='store_true', help='Output cuts for given stage only'
                            )
        parser.add_argument('-a', '--recreate-all', dest='recreate_all', action='store_true',
                            help="""
                            Retrain all networs and recreate all histograms
                            """)

        ## Stores arguments passed via command line and parsed by argparse of
        ## python
        self.args = parser.parse_args()

    def addParticle(self, particle):
        """
        Appends a Particle object to the FullReconstruction setup
        """

        self.particles.append(particle)

    def run(self, path, mcrun=False):
        """
        All the added Particle objects are arranged in stages according to
        their dependencies. Every stage is then reconstructed. The
        reconstruction stops if a stage is missing some information
        (cut histograms or classifiers) to be successfully reconstructed.
        The needed modules to create this information are added to the path.
        So you need to run the FullReconstruction several times, to train
        all stages.
        @param path  modules are added to this path
        @param mcrun if True, only do reconstruction, with no cuts, trainings,
                     or classifications.
        """

        # Create a map from abs(pdg code) to the Particle object
        # abs is necessary because charge conjugated particles are handlet
        # automatically inside basf2.
        # We use the old dict comprehension syntax because fixstyle crashs
        # with the new one :-(
        pdg_to_particle = dict((abs(p.pdg), p) for p in self.particles)

        # Arrange given particles into stages according to their dependencies
        # First create a map from abs(pdg code) -> list(daughter particle pdg
        # codes) for all the particles
        # We use the old dict comprehension syntax because fixstyle crashs
        # with the new one :-(
        dependencies = dict((abs(p.pdg), p.dependencies()) for p in self.particles)
        stages = []
        # Loop over the dependencies as long as their are still particles
        # which aren't added into a stage.
        while len(dependencies) > 0:
            # Add  pdg codes to the next stage which don't have any
            # dependencies left
            stages.append([pdg for (pdg, d) in dependencies.iteritems() if len(d) == 0])
            # Keep only dependencies to pdg codes which weren't added to the
            # stage in the previous line.
            # We use the old dict comprehension syntax because fixstyle crashs
            # with the new one :-(
            dependencies = dict((pdg, filter(lambda x: x not in stages[-1], d)) for (pdg, d) in dependencies.iteritems()
                                if len(d) > 0)
            # If no pdg codes were added to the current stagem but the
            # dependencies are still not empty, something is wring with the
            # definition of the channels, e.g. cyclic dependencies
            if len(stages[-1]) == 0:
                raise Exception('Error in Decay Channels Definition')

        # Print calculated stages
        for (i, stage) in enumerate(stages):
            print i, stage

        # Create hashseed for all the particles.
        # A particle should depend on all the particles in the lower stages.
        # So if one added for example a new variable to the training of a
        # kaon, all particles in the higher stages, which depend on the
        # reconstruction of the kaon are retrained too.
        # For final state particles the hashseed is empty.
        hashseed = ''
        for (i, stage) in enumerate(stages):
            # Set current hashseed, which depends on the previous stages to
            # all the particles in the current stage
            for pdg in stage:
                pdg_to_particle[pdg].hashseed = hashseed
            # Update hashseed, append information about the used methods,
            # channels and variables for every particle in the current stage
            for pdg in stage:
                p = pdg_to_particle[pdg]
                hashseed += str(p.methods) + str(p.channels) + str(p.variables)
            hashseed = hashlib.sha1(hashseed).hexdigest()

        # Process options
        for (i, stage) in enumerate(stages):
            if self.args.retrain_stage and self.args.retrain_stage == i or self.args.recreate_stage and self.args.recreate_stage == i or self.args.recreate_all:
                for pdg in stage:
                    p = pdg_to_particle[pdg]
                    for method in p.methods:
                        for channel in p.channels:
                            if os.path.isfile(p.getWeightFilename(channel, method)):
                                print 'Remove ' + p.getWeightFilename(channel, method)
                                os.remove(p.getWeightFilename(channel, method))
            if self.args.recreate_stage and self.args.recreate_stage == i or self.args.recreate_all:
                for pdg in stage:
                    p = pdg_to_particle[pdg]
                    if os.path.isfile(p.getHistFilename()):
                        print 'Remove ' + p.getHistFilename()
                        os.remove(p.getHistFilename())

        # Run every stage until a stage stops
        for (i, stage) in enumerate(stages):
            # Output only the cuts for the different channels
            if self.args.print_cuts:
                for pdg in stage:
                    p = pdg_to_particle[pdg]
                    print p.getCuts()
                continue

            # First we reconstruct all the particles in the current stage, so
            # we create a list of particle candidates from the given decay
            # channels. If not all particles can be reconstructed, then we
            # missing information about the intermediate cuts we want to
            # apply. So some histograms have to be generated first.
            if not all([pdg_to_particle[pdg].reconstruct(path, mcrun) for pdg in stage]):
                if not self.args.train:
                    B2ERROR('Missing cuts for a particle at stage' + ' {i} abort process.'.format(i=i)
                            + 'iPleas use --train to train the fullrecon.')
                B2WARNING('Missing cuts for a particle at stage' + ' {i} generating cut'.format(i=i)
                          + ' determination histograms.')
                break

            # Secondly we classify the reconstructed particle candidates with
            # the given methods (e.g. BDTs or NeuroBayes). These multivariate
            # methods have to be trained first. So if not all training files
            # (experts) are present yet, we stop the evaluation at this stage
            # and generate these experts first.
            if not mcrun:
                if not all([pdg_to_particle[pdg].classify(path) for pdg in stage]):
                    if not self.args.train:
                        B2ERROR('Missing experts for a particle at stage' + ' {i} abort process.'.format(i=i)
                                + 'iPleas use --train to train the fullrecon.')
                    B2WARNING('Missing experts for a particle at stage' + ' {i} generating experts.'.format(i=i))
                    break
