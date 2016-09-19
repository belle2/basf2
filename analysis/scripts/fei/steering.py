#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2016

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
import fei.dag
# The different tasks themselves are defined in the provider module
from fei import provider
# Import utility classes
import fei.config

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
    parser.add_argument('-externTeacher', '--externTeacher', dest='externTeacher', type=str, default='basf2_mva_teacher',
                        help='Use this command to invoke extern teacher: basf2_mva_teacher or externClusterTeacher')
    parser.add_argument('-monitor', '--monitor', dest='monitor', action='store_true',
                        help='Create monitor NTuples/Histograms used for Reporting system')
    parser.add_argument('-prune', '--prune', dest='prune', action='store_true',
                        help='Prune particles which are not in lists.')
    parser.add_argument('-rerunCached', '--rerunCached', dest='rerunCachedProviders', action='store_true',
                        help='Runs cached providers again')
    parser.add_argument('-noSelection', '--noSelection', dest='noSelection', action='store_true',
                        help='Set whether the selection path should be removed from the to be dumped pickle-file')
    args = parser.parse_args()
    return args


# Simple object containing the output of fei
FeiState = collections.namedtuple('FeiState', 'path, fei_path, selection_path, is_trained')


def charge_conjugated_identifier(particle):
    """ Property returning the identifier of the charge conjugated identifier """
    return pdg.conjugate(particle.name) + ':' + particle.label


def fullEventInterpretation(signalParticleList, selection_path, particles, databasePrefix, BtoBII=False):
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
        @param databasePrefix used to store the generated weightfiles in the database
        @param BtoBII Boolian to set geometry and gearbox for B2BII converted Belle Data/MC. Default is False.
        @return FeiState object containing basf2 path to execute, plus status information
    """
    args = getCommandLineOptions()

    # Create a new directed acyclic graph
    dag = fei.dag.DAG()

    # Set environment variables
    dag.env['ROE'] = str(signalParticleList) if signalParticleList is not None else False
    dag.env['monitor'] = args.monitor
    dag.env['verbose'] = args.verbose
    dag.env['nThreads'] = args.nThreads
    dag.env['rerunCachedProviders'] = args.rerunCachedProviders
    dag.env['externTeacher'] = args.externTeacher

    dag.add('DatabasePrefix', databasePrefix)

    # Add some fake resources for the FSPs
    for fsp in ['e+:FSP', 'mu+:FSP', 'pi+:FSP', 'K+:FSP', 'K_S0:V0', 'K_L0:FSP', 'p+:FSP', 'gamma:FSP', 'gamma:V0']:
        for resource in ['ParticleList_', 'VertexFit_', 'SignalProbability_']:
            dag.add(resource + fsp, fsp)

    # Add some fake resources for FSPs which are only available in b2bii
    if BtoBII:
        for resource in ['ParticleList_', 'VertexFit_', 'SignalProbability_']:
            dag.add(resource + 'pi0:FSP', 'pi0:FSP')
        # Add Modules which are always needed
        dag.add('particles', provider.LoadParticlesB2BII,
                names=['Name_' + particle.identifier for particle in particles])
    else:
        # Add Modules which are always needed
        dag.add('particles', provider.LoadParticles,
                names=['Name_' + particle.identifier for particle in particles])

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
            dag.add('PreCutConfig_' + channel.name, channel.preCutConfig)
            dag.add('DecayModeID_' + channel.name, channel.decayModeID)

    # Reconstruct given particle topology
    for particle in particles:
        for channel in particle.channels:
            dag.add('RawParticleList_' + channel.name, provider.MakeParticleList,
                    'particles',
                    'Label_' + particle.identifier,
                    particleName='Name_' + particle.identifier,
                    daughterParticleLists=['ParticleList_' + daughter for daughter in channel.daughters],
                    preCutConfig='PreCutConfig_' + channel.name,
                    mvaConfig='MVAConfig_' + channel.name,
                    decayModeID='DecayModeID_' + channel.name)
            dag.add('MatchedParticleList_' + channel.name, provider.MatchParticleList,
                    particleList='RawParticleList_' + channel.name,
                    mvaConfig='MVAConfig_' + channel.name)

        # Copy all channel lists into a single particle list
        dag.add('ParticleList_' + particle.identifier, provider.CopyParticleLists,
                particleName='Name_' + particle.identifier,
                particleLabel='Label_' + particle.identifier,
                particleLists=['RawParticleList_' + channel.name for channel in particle.channels],
                postCutConfig='PostCutConfig_' + particle.identifier,
                mvaConfig='MVAConfig_' + channel.name,
                signalProbabilities=['SignalProbability_' + channel.name for channel in particle.channels])

        # Copy particle list into a list with a human readable name
        dag.add('HumanReadableParticleList_' + particle.identifier, provider.CopyIntoHumanReadableParticleList,
                'TagUniqueSignal_' + particle.identifier,
                particleName='Name_' + particle.identifier,
                particleLabel='Label_' + particle.identifier,
                mvaConfig='MVAConfig_' + channel.name,
                particleList='ParticleList_' + particle.identifier)

        # Add the same Resource for the charge conjugated particle
        if particle.name != pdg.conjugate(particle.name):
            dag.add('ParticleList_' + charge_conjugated_identifier(particle), provider.PDGConjugate,
                    particleList='ParticleList_' + particle.identifier)

    # Vertex fit
    for particle in particles:
        for channel in particle.channels:
            if len(channel.daughters) == 1:
                dag.add('VertexFit_' + channel.name, 'TrackFitIsAlreadyDoneForFSPs')
            else:
                dag.add('VertexFit_' + channel.name, provider.FitVertex,
                        ['VertexFit_' + daughter for daughter in channel.daughters],
                        channelName='Name_' + channel.name,
                        particleList='RawParticleList_' + channel.name,
                        mvaConfig='MVAConfig_' + channel.name,
                        preCutConfig='PreCutConfig_' + channel.name)
        # Create common VertexFit Resource for this particle, thereby its easy to state the dependency
        # on the VertexFit of a daughter particle in the line above!
        dag.add('VertexFit_' + particle.identifier, provider.HashRequirements,
                ['VertexFit_' + channel.name for channel in particle.channels])

        # Add the same Resource for the charge conjugated particle
        if particle.name != pdg.conjugate(particle.name):
            dag.add('VertexFit_' + charge_conjugated_identifier(particle), 'VertexFit_' + particle.identifier)

    # Trains multivariate classifiers (MVC) methods and provides signal probabilities
    for particle in particles:
        for channel in particle.channels:
            # Generate training data from MC-matched ParticleList
            dag.add('TrainingData_' + channel.name, provider.GenerateTrainingData,
                    ['SignalProbability_' + daughter for daughter in channel.daughters],
                    ['VertexFit_' + channel.name],
                    particleName='Name_' + particle.identifier,
                    mcCounts='mcCounts',
                    preCutConfig='PreCutConfig_' + channel.name,
                    mvaConfig='MVAConfig_' + channel.name,
                    particleList='MatchedParticleList_' + channel.name)

            # Train and apply MVC on raw ParticleList
            dag.add('TrainedMVC_' + channel.name, provider.TrainMultivariateClassifier,
                    databasePrefix='DatabasePrefix',
                    mvaConfig='MVAConfig_' + channel.name,
                    trainingData='TrainingData_' + channel.name)
            dag.add('SignalProbability_' + channel.name, provider.SignalProbability,
                    databasePrefix='DatabasePrefix',
                    mvaConfig='MVAConfig_' + channel.name,
                    particleList='RawParticleList_' + channel.name,
                    tmvaPrefix='TrainedMVC_' + channel.name)

        # Create common SignalProbability Resource for this particle, thereby its easy to state the dependency
        # on the SignalProbability of a daughter particle in the line above!
        dag.add('SignalProbability_' + particle.identifier, provider.HashRequirements,
                ['SignalProbability_' + channel.name for channel in particle.channels])

        # Add the same Resource for the charge conjugated particle
        if particle.name != pdg.conjugate(particle.name):
            dag.add('SignalProbability_' + charge_conjugated_identifier(particle), 'SignalProbability_' + particle.identifier)

    # Search all final particles (particles which don't appear as a daughter particle in a channel of any other particle)
    finalParticles = [particle for particle in particles if all(
                      particle.identifier not in o.daughters and
                      pdg.conjugate(particle.name) + ':' + particle.label not in o.daughters for o in particles)]

    # Write out additional information, histograms, ...
    dag.add('mcCounts', provider.CountMCParticles,
            'particles',
            names=['Name_' + particle.identifier for particle in particles])

    dag.add('ModuleStatisticsFile', provider.SaveModuleStatistics,
            ['SignalProbability_' + finalParticle.identifier for finalParticle in finalParticles])

    for particle in particles:
        # Tag unique signal candidates, to avoid double-counting
        # e.g. B->D* pi and B->D pi pi can contain the same correctly reconstructed candidate
        dag.add('TagUniqueSignal_' + particle.identifier, provider.TagUniqueSignal,
                particleList='ParticleList_' + particle.identifier,
                signalProbability='SignalProbability_' + particle.identifier,
                mvaConfig='MVAConfig_' + particle.identifier)
        dag.addNeeded('TagUniqueSignal_' + particle.identifier)
        dag.addNeeded('SignalProbability_' + particle.identifier)
        dag.addNeeded('ParticleList_' + particle.identifier)
        dag.addNeeded('HumanReadableParticleList_' + particle.identifier)

    dag.add('SaveSummary', provider.SaveSummary,
            mcCounts='mcCounts',
            moduleStatistics='ModuleStatisticsFile',
            particles=['Object_' + particle.identifier for particle in particles],
            plists=['ParticleList_' + particle.identifier for particle in particles],
            cnames=['Name_' + channel.name for particle in particles for channel in particle.channels],
            clists=['RawParticleList_' + channel.name for particle in particles for channel in particle.channels],
            mlists=['MatchedParticleList_' + channel.name for particle in particles for channel in particle.channels],
            trainingData=['TrainingData_' + channel.name for particle in particles for channel in particle.channels])

    dag.addNeeded('SaveSummary')

    fei_path = create_path()

    is_first_run = args.cache is None or not os.path.isfile(args.cache)
    if args.cache is not None:
        dag.load_cached_resources(args.cache)
    finished_training = dag.run(fei_path)

    if args.cache is not None:
        dag.save_cached_resources(args.cache)

    if args.prune:
        listNames = []
        for particle in particles:
            sig = 'SignalProbability_' + particle.identifier
            if sig in dag.resources and dag.resources[sig].value is not None:
                listNames += ['ParticleList_' + particle.identifier]
            else:
                listNames += ['RawParticleList_' + channel.name for channel in particle.channels]
        particleLists = [dag.resources[i].value for i in listNames if i in dag.resources and dag.resources[i].value is not None]
        if signalParticleList:
            particleLists += [signalParticleList]  # , 'B+:FEIMC'] FEIMC uses different particle array
        if args.verbose:
            print("Removing particles which aren't in the following lists")
            print(listNames)
            print(particleLists)
        fei_path.add_module("RemoveParticlesNotInLists", particleLists=particleLists)

    path = create_path()

    if finished_training:
        if not args.noSelection:
            path.add_path(selection_path)
        path.add_path(fei_path)
        return FeiState(path, fei_path, selection_path, is_trained=True)

    fei_path.add_module("RootOutput")
    if is_first_run:
        path.add_path(selection_path)
        if signalParticleList:
            isSignal = 'isSignalAcceptMissingNeutrino'
            signalMC = 'eventCached(countInList(B+:FEIMC))'
            cut = '[[{mc} > 0 and {sig} == 1] or [{mc} == 0 and {sig} != 1]]'.format(mc=signalMC, sig=isSignal)
            applyCuts(signalParticleList, cut, path=path)
            roe_path = create_path()
            cond_module = register_module('SignalSideParticleFilter')
            cond_module.param('particleListName', signalParticleList)
            cond_module.if_true(fei_path, AfterConditionPath.END)
            roe_path.add_module(cond_module)
            path.for_each('RestOfEvent', 'RestOfEvents', roe_path)
        else:
            path.add_path(fei_path)
    else:
        path.add_module('RootInput', excludeBranchNamesPersistent=[])

        if BtoBII:
            gearbox = register_module('Gearbox')
            path.add_module(gearbox)
            path.add_module('Geometry', ignoreIfPresent=False, components=['MagneticFieldConstantBelle'])
            make_code_pickable('import ROOT\n'
                               'from ROOT import Belle2\n'
                               'ROOT.Belle2.BFieldManager.getInstance().setConstantOverride(0, 0, 1.5 * ROOT.Belle2.Unit.T)')
        else:
            path.add_module('Gearbox')
            path.add_module('Geometry', ignoreIfPresent=True, components=['MagneticField'])
        path.add_path(fei_path)

    # with RestOfEvent path, this will be the first module inside for_each
    path.add_module('ProgressBar')
    if args.noSelection:
        path = fei_path
    return FeiState(path, fei_path, selection_path, is_trained=False)
