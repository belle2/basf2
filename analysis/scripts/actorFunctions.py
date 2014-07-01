#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Thomas Keck 2014
#
# All the actors functions are defined here.
# To create your own actor:
#   1. Write a normal function which takes the needed arguments and returns a dictonary of provided values.
#      E.g. def foo(path, particleList) ... return {'Stuff': x}
#   2. Make sure your return value depends on all the used arguments, easiest way to accomplish this is using the createHash function.
#   3. Add the function to the sequence object like this (in FullEventInterpretation.py):
#      seq.addFunction(foo, path='Path', particleList='K+')

from basf2 import *
import ROOT
import modularAnalysis
import pdg

import actorFramework
import preCutDetermination
import os
import subprocess
from string import Template


def SelectParticleList(path, particleName, explicitCuts):
    """
    Creates a ParticleList gathering up all particles with the given particleName
        @param path the basf2 path
        @param particleName returned key is named ParticleList_{particleName} corresponding ParticleList is stored as {particleName}:{hash}
    """
    userLabel = actorFramework.createHash(particleName, explicitCuts)
    outputList = particleName + ':' + userLabel
    modularAnalysis.selectParticle(outputList, explicitCuts, path=path)
    return {'ParticleList_' + particleName: outputList,
            'ParticleList_' + pdg.conjugate(particleName): pdg.conjugate(particleName) + ':' + userLabel}


def CopyParticleLists(path, particleName, inputLists, pdf):
    """
    Creates a ParticleList gathering up all particles in the given inputLists
        @param path the basf2 path
        @param particleName returned key is named ParticleList_{particleName} corresponding ParticleList is stored as {particleName}:{hash}
        @param inputLists names of inputLists which are copied to the new list
        @param pdf dependency to particle overview pdf, to trigger the generation of the overview PDF as soon as the particle is available
    """
    inputLists = actorFramework.removeNones(inputLists)
    if inputLists == []:
        return {'ParticleList_' + particleName: None, 'ParticleList_' + pdg.conjugate(particleName): None}

    userLabel = actorFramework.createHash(particleName, inputLists)
    outputList = particleName + ':' + userLabel
    modularAnalysis.copyLists(outputList, inputLists, path=path)
    return {'ParticleList_' + particleName: outputList,
            'ParticleList_' + pdg.conjugate(particleName): pdg.conjugate(particleName) + ':' + userLabel}


def MakeAndMatchParticleList(path, particleName, channelName, inputLists, preCut):
    """
    Creates a ParticleList by combining other particleLists via the ParticleCombiner module and match MC truth for this new list.
        @param path the basf2 path
        @param particleName name of the reconstructed particle, new list is stored as {particleName}:{hash}, where the hash depends on the channel
        @param channelName describes the combined decay, returned key ParticleList_{channelName}
        @param inputLists the inputs lists which are combined
        @param preCut cuts which are applied before the combining of the particles
    """
    if preCut is None:
        return {'ParticleList_' + channelName + '_' + particleName: None, 'ParticleList_' + channelName + '_' + pdg.conjugate(particleName): None}

    userLabel = actorFramework.createHash(particleName, channelName, inputLists, preCut)
    outputList = particleName + ':' + userLabel + ' ==> ' + ' '.join(inputLists)
    listName = particleName + ':' + userLabel
    modularAnalysis.makeParticle(outputList, preCut, path=path)
    modularAnalysis.matchMCTruth(listName, path=path)
    return {'ParticleList_' + channelName + '_' + particleName: listName,
            'ParticleList_' + channelName + '_' + pdg.conjugate(particleName): pdg.conjugate(particleName) + ':' + userLabel}


def SignalProbability(path, particleName, channelName, mvaConfig, particleList, nBackground=None, daughterSignalProbabilities=[]):
    """
    Calculates the SignalProbability of a ParticleList. If the files required from TMVAExpert aren't available they're created.
        @param path the basf2 path
        @param particleName of the particle which is classified
        @param channelName of channel which is classified
        @param mvaConfig configuration for the multivariate analysis
        @param particleList the particleList which is used for training and classification
        @param nBackground number of background events
        @param daughterSignalProbabilities all daughter particles need a SignalProbability
    """
    if particleList is None or any([daughterSignalProbability is None for daughterSignalProbability in daughterSignalProbabilities]):
        return {'SignalProbability_' + channelName + '_' + particleName: None,
                'SignalProbability_' + channelName + '_' + pdg.conjugate(particleName): None}

    hash = actorFramework.createHash(particleName, channelName, mvaConfig, particleList, nBackground, daughterSignalProbabilities)

    filename = '{particleList}_{hash}.config'.format(particleList=particleList, hash=hash)
    if os.path.isfile(filename):

        expert = register_module('TMVAExpert')
        expert.set_name('TMVAExpert_' + particleList)
        expert.param('prefix', particleList + '_' + hash)
        expert.param('method', mvaConfig.name)
        expert.param('signalProbabilityName', 'SignalProbability')
        expert.param('signalCluster', mvaConfig.targetCluster)
        expert.param('listNames', [particleList])
        path.add_module(expert)

        if particleName == channelName:
            return {'SignalProbability_' + particleName: filename,
                    'SignalProbability_' + pdg.conjugate(particleName): filename}
        else:
            return {'SignalProbability_' + channelName + '_' + particleName: filename,
                    'SignalProbability_' + channelName + '_' + pdg.conjugate(particleName): filename}

    else:
        teacher = register_module('TMVATeacher')
        teacher.set_name('TMVATeacher_' + particleList)
        teacher.param('prefix', particleList + '_' + hash)
        teacher.param('methods', [(mvaConfig.name, mvaConfig.type, mvaConfig.config)])
        teacher.param('factoryOption', '!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification')

        if nBackground is None:
            n = 0
        elif nBackground < 2000000:
            n = 0
        else:
            n = 1000000

        teacher.param('prepareOption', 'SplitMode=random:!V:nTrain_Background={n}:nTest_Background={n}'.format(n=n))
        teacher.param('variables', mvaConfig.variables)
        teacher.param('target', mvaConfig.target)
        teacher.param('listNames', [particleList])
        path.add_module(teacher)
        return {}


def CreatePreCutHistogram(path, particleName, channelName, preCutConfig, daughterLists, additionalDependencies):
    """
    Creates ROOT file with invariant mass and signal probability product histogram of this channel (signal/background)
    for a given particle, before any intermediate cuts are applied.
        @param path the basf2 path
        @param particleName for which this histogram is created
        @param channelName of the channel
        @param daughterLists all particleLists of all the daughter particles
        @param daughterSignalProbabilities all daughter particles need a SignalProbability
    """
    if any([daughterList is None for daughterList in daughterLists]) or any([x is None for x in additionalDependencies]):
        return {'PreCutHistogram_' + channelName: None}

    # Check if the file is available. If the file isn't available yet, create it with
    # the HistMaker Module and return Nothing. If a function is called which depends on
    # the PreCutHistogram, the process will stop, because the PreCutHistogram isn't provided.
    hash = actorFramework.createHash(particleName, channelName, preCutConfig.variable, daughterLists, additionalDependencies)
    filename = 'CutHistograms_{pname}_{cname}_{hash}.root'.format(pname=particleName, cname=channelName, hash=hash)

    if os.path.isfile(filename):
        return {'PreCutHistogram_' + channelName: (filename, particleName + ':' + hash)}
    else:
        # Combine all the particles according to the decay channels
        outputList = particleName + ':' + hash + ' ==> ' + ' '.join(daughterLists)
        pmake = register_module('PreCutHistMaker')
        pmake.set_name('PreCutHistMaker_' + channelName)
        pmake.param('fileName', filename)
        pmake.param('decayString', outputList)
        if preCutConfig.variable == 'Mass':
            mass = pdg.get(pdg.from_name(particleName)).Mass()
            pmake.param('variable', 'M')
            pmake.param('histParams', (200, mass / 2, mass + mass / 2))
        else:
            pmake.param('variable', 'daughterProductOf(getExtraInfo(SignalProbability))')
            pmake.param('customBinning', list(reversed([1.0 / (1.5 ** i) for i in range(0, 20)])))
        path.add_module(pmake)
    return {}


def PreCutDetermination(particleName, channelNames, preCutConfig, preCutHistograms):
    """
    Determines the PreCuts for all the channels of a particle.
        @param particleName name of the particle
        @param channelNames list of the names of all the channels
        @param preCutConfig configuration for PreCut determination e.g. signal efficiency for this particle
        @param preCutHistograms filenames of the histogram files created for every channel by PreCutDistribution
    """

    results = {'PreCut_' + channel: None for channel, _ in zip(*actorFramework.getNones(channelNames, preCutHistograms))}
    channelNames, preCutHistograms = actorFramework.removeNones(channelNames, preCutHistograms)

    cuts = preCutDetermination.CalculatePreCuts(preCutConfig, channelNames, preCutHistograms)

    for (channel, cut) in cuts.iteritems():
        results['PreCut_' + channel] = None if cut['isIgnored'] else {cut['variable']: cut['range']}
        results['nSignal_' + channel] = None if cut['isIgnored'] else cut['nSignal']
        results['nBackground_' + channel] = None if cut['isIgnored'] else cut['nBackground']
    return results


def WriteAnalysisFileForChannel(particleName, channelName, channelList, preCutConfig, preCutHistogram, preCut, mvaConfig, signalProbability):
    """
    Creates a pdf document with the PreCut and Training plots
        @param particleName name of the particle
        @param channelName name of the channel
        @param channelList ParticleList name of the channel
        @param preCutConfig configuration for pre cut
        @param preCutHistogram preCutHistogram (filename, histogram postfix)
        @param preCut used preCuts for this channel
        @param mvaConfig configuration for mva
        @param signalProbability config filename for TMVA training
    """

    if signalProbability is None or preCut is None:
        return {'Tex_' + channelName: None}

    ROOT.gROOT.SetBatch(True)

    # Create TMVA Plots
    tmva_filename = signalProbability[:-7] + '.root'  # Strip .config of filename
    subprocess.call(['root -l -q -b "$BELLE2_EXTERNALS_DIR/src/root/tmva/test/mvas.C(\\"{f}\\",3)"'.format(f=tmva_filename)], shell=True)
    subprocess.call(['cp plots/$(ls -t plots/ | head -1) {c}_overtraining.png'.format(c=channelList)], shell=True)
    subprocess.call(['root -l -q -b "$BELLE2_EXTERNALS_DIR/src/root/tmva/test/efficiencies.C(\\"{f}\\")"'.format(f=tmva_filename)], shell=True)
    subprocess.call(['cp plots/$(ls -t plots/ | head -1) {c}_roc.png'.format(c=channelList)], shell=True)

    # Create PreCut Hist Plots
    rootfile = ROOT.TFile(preCutHistogram[0])
    names = ['signal', 'all', 'background', 'ratio']
    keys = rootfile.GetListOfKeys()
    lc, uc = preCut.values()[0]
    for (name, key) in zip(names, keys):
        canvas = ROOT.TCanvas(name + channelList + '_canvas', name, 600, 400)
        canvas.cd()
        hist = rootfile.Get(key.GetName())
        hist.Draw()
        ll = ROOT.TLine(lc, 0, lc, hist.GetMaximum())
        ul = ROOT.TLine(uc, 0, uc, hist.GetMaximum())
        ll.Draw()
        ul.Draw()
        canvas.SaveAs(channelList + '_' + name + '.png')

    # Calculate purity and efficiency for this channel
    # ...
    signal_hist = rootfile.Get(keys.At(0).GetName())
    background_hist = rootfile.Get(keys.At(2).GetName())

    ntotalsignal = signal_hist.Integral()
    nsignal = preCutDetermination.GetNumberOfEventsInRange(signal_hist, (lc, uc))
    nbckgrd = preCutDetermination.GetNumberOfEventsInRange(background_hist, (lc, uc))

    purity = nsignal / (nsignal + nbckgrd)
    efficiency = nsignal / ntotalsignal

    placeholders = {}

    placeholders['particleName'] = particleName
    placeholders['channelName'] = channelName

    placeholders['channelNSignal'] = int(nsignal)
    placeholders['channelNBackground'] = int(nbckgrd)
    placeholders['channelPurity'] = '{:.3f}'.format(purity)
    placeholders['channelEfficiency'] = '{:.3f}'.format(efficiency)

    placeholders['preCutVariable'] = preCutConfig.variable
    placeholders['preCutEfficiency'] = '{:.2f}'.format(preCutConfig.efficiency)
    a, b = preCut.values()[0]
    placeholders['preCutRange'] = 'Ignored' if preCut is None else '({:.2f},'.format(a) + ' {:.2f}'.format(b) + ')'

    placeholders['preCutAllPlot'] = channelList + '_all.png'
    placeholders['preCutSignalPlot'] = channelList + '_signal.png'
    placeholders['preCutBackgroundPlot'] = channelList + '_background.png'
    placeholders['preCutRatioPlot'] = channelList + '_ratio.png'

    placeholders['mvaName'] = mvaConfig.name
    placeholders['mvaType'] = mvaConfig.type
    placeholders['mvaConfig'] = mvaConfig.config
    placeholders['mvaVariables'] = ', '.join(mvaConfig.variables)
    placeholders['mvaTarget'] = mvaConfig.target
    placeholders['mvaTargetCluster'] = mvaConfig.targetCluster

    placeholders['mvaROCPlot'] = channelList + '_roc.png'
    placeholders['mvaOvertrainingPlot'] = channelList + '_overtraining.png'

    template = Template(file(ROOT.Belle2.FileSystem.findFile('analysis/scripts/FullEventInterpretationChannelTemplate.tex'), 'r').read())
    page = template.substitute(placeholders)
    filename = channelList + '.tex'
    file(filename, 'w').write(page)

    return {'Tex_' + channelName: (filename, placeholders)}


def WriteAnalysisFileForParticle(particleName, texfiles):
    """
    Creates a pdf document with the PreCut and Training plots
        @param particleName name of the particle
        @param texfiles list of tex filenames
    """
    placeholders = {}
    placeholders['NChannels'] = len(texfiles)
    texfiles = actorFramework.removeNones(texfiles)
    placeholders['NUsedChannels'] = len(texfiles)

    placeholders['particleName'] = particleName
    placeholders['channelInputs'] = ""
    placeholders['particleNSignal'] = 0
    placeholders['particleNBackground'] = 0
    for texfile, channelPlaceholders in texfiles:
        placeholders['particleNSignal'] += channelPlaceholders['channelNSignal']
        placeholders['particleNBackground'] += channelPlaceholders['channelNBackground']
        placeholders['channelInputs'] += '\include{' + texfile[:-4] + '}\n'

    template = Template(file(ROOT.Belle2.FileSystem.findFile('analysis/scripts/FullEventInterpretationParticleTemplate.tex'), 'r').read())
    page = template.substitute(placeholders)
    filename = particleName + '.tex'
    file(filename, 'w').write(page)

    subprocess.call(['pdflatex', filename])

    # Return None - Therefore Particle List depends not on TMVAExpert directly
    return {'PDF_' + particleName: None}  # filename[:-4] + '.pdf'}
