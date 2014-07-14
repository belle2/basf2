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
from makeDiagPlotForTMVA import makeDiagPlots
from makeMbcPlot import makeMbcPlot
import os
import subprocess
from string import Template


def SelectParticleList(path, particleName):
    """
    Creates a ParticleList gathering up all particles with the given particleName
        @param path the basf2 path
        @param particleName returned key is named ParticleList_{particleName} corresponding ParticleList is stored as {particleName}:{hash}
    """
    userLabel = actorFramework.createHash(particleName)
    outputList = particleName + ':' + userLabel
    modularAnalysis.selectParticle(outputList, path=path)

    B2INFO("Select Particle List " + outputList + " and charged conjugated")
    return {'RawParticleList_' + particleName: outputList,
            'RawParticleList_' + pdg.conjugate(particleName): pdg.conjugate(particleName) + ':' + userLabel}


def CopyParticleLists(path, particleName, inputLists, postCut, pdf):
    """
    Creates a ParticleList gathering up all particles in the given inputLists
        @param path the basf2 path
        @param particleName returned key is named ParticleList_{particleName} corresponding ParticleList is stored as {particleName}:{hash}
        @param inputLists names of inputLists which are copied to the new list
        @param postCut cuts which are applied after the reconstruction of the particle
        @param pdf dependency to particle overview pdf, to trigger the generation of the overview PDF as soon as the particle is available
    """
    inputLists = actorFramework.removeNones(inputLists)
    if inputLists == []:
        B2INFO("Gather Particle List for particle " + particleName + " and charged conjugated. But there are no particles to gather :-(.")
        return {'ParticleList_' + particleName: None, 'ParticleList_' + pdg.conjugate(particleName): None}

    userLabel = actorFramework.createHash(particleName, inputLists)
    outputList = particleName + ':' + userLabel
    modularAnalysis.cutAndCopyLists(outputList, inputLists, postCut['cutstring'], path=path)

    B2INFO("Gather Particle List for particle " + particleName + " and charged conjugated")
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
        B2INFO("Make and Match Particle List for channel " + channelName + " and charged conjugated. But the channel is ignored :-(.")
        return {'ParticleList_' + channelName + '_' + particleName: None, 'ParticleList_' + channelName + '_' + pdg.conjugate(particleName): None}

    userLabel = actorFramework.createHash(particleName, channelName, inputLists, preCut)
    outputList = particleName + ':' + userLabel + ' ==> ' + ' '.join(inputLists)
    listName = particleName + ':' + userLabel
    modularAnalysis.makeParticle(outputList, preCut['cutstring'], path=path)
    modularAnalysis.matchMCTruth(listName, path=path)
    B2INFO("Make and Match Particle List for channel " + channelName + " and charged conjugated.")
    return {'ParticleList_' + channelName + '_' + particleName: listName,
            'ParticleList_' + channelName + '_' + pdg.conjugate(particleName): pdg.conjugate(particleName) + ':' + userLabel}


def SignalProbability(path, particleName, channelName, mvaConfig, particleList, daughterSignalProbabilities=[]):
    """
    Calculates the SignalProbability of a ParticleList. If the files required from TMVAExpert aren't available they're created.
        @param path the basf2 path
        @param particleName of the particle which is classified
        @param channelName of channel which is classified
        @param mvaConfig configuration for the multivariate analysis
        @param particleList the particleList which is used for training and classification
        @param daughterSignalProbabilities all daughter particles need a SignalProbability
    """
    if particleList is None or any([daughterSignalProbability is None for daughterSignalProbability in daughterSignalProbabilities]):
        B2INFO("Calculate SignalProbability for channel " + channelName + " and charged conjugated. But the channel is ignored :-(.")
        return {'SignalProbability_' + channelName + '_' + particleName: None,
                'SignalProbability_' + channelName + '_' + pdg.conjugate(particleName): None}

    hash = actorFramework.createHash(particleName, channelName, mvaConfig, particleList, daughterSignalProbabilities)

    rootFilename = '{particleList}_{hash}.root'.format(particleList=particleList, hash=hash)
    configFilename = '{particleList}_{hash}.config'.format(particleList=particleList, hash=hash)

    if not os.path.isfile(rootFilename):
        teacher = register_module('TMVATeacher')
        teacher.set_name('TMVATeacher_' + particleList)
        teacher.param('prefix', particleList + '_' + hash)
        teacher.param('methods', [(mvaConfig.name, mvaConfig.type, mvaConfig.config)])
        teacher.param('factoryOption', '!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification')
        teacher.param('prepareOption', 'SplitMode=random:!V')
        teacher.param('variables', mvaConfig.variables)
        teacher.param('target', mvaConfig.target)
        teacher.param('listNames', [particleList])
        teacher.param('maxEventsPerClass', 1000000)
        teacher.param('doNotTrain', True)
        path.add_module(teacher)
        B2INFO("Calculate SignalProbability for channel " + channelName + " and charged conjugated. Create root file with variables first.")
        return {}

    if not os.path.isfile(configFilename):
        B2INFO("Calculate SignalProbability for channel " + channelName + " and charged conjugated. Run Teacher in extern process.")
        subprocess.call("externTeacher --methodName '{name}' --methodType '{type}' --methodConfig '{config}' --target '{target}'"
                        " --variables '{variables}' --factoryOption '{foption}' --prepareOption '{poption}' --prefix '{prefix}'"
                        " --maxEventsPerClass {maxEvents}".format(name=mvaConfig.name, type=mvaConfig.type, config=mvaConfig.config,
                                                                  target=mvaConfig.target, variables="' '".join(mvaConfig.variables),
                                                                  foption='!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification',
                                                                  poption='SplitMode=random:!V', maxEvents=1000000,
                                                                  prefix=particleList + '_' + hash), shell=True)

    if os.path.isfile(configFilename):

        expert = register_module('TMVAExpert')
        expert.set_name('TMVAExpert_' + particleList)
        expert.param('prefix', particleList + '_' + hash)
        expert.param('method', mvaConfig.name)
        expert.param('signalFraction', -2)  # Use signalFraction from training
        expert.param('signalProbabilityName', 'SignalProbability')
        expert.param('signalCluster', mvaConfig.targetCluster)
        expert.param('listNames', [particleList])
        path.add_module(expert)

        B2INFO("Calculate SignalProbability for channel " + channelName + " and charged conjugated.")
        if particleName == channelName:
            return {'SignalProbability_' + particleName: configFilename,
                    'SignalProbability_' + pdg.conjugate(particleName): configFilename}
        else:
            return {'SignalProbability_' + channelName + '_' + particleName: configFilename,
                    'SignalProbability_' + channelName + '_' + pdg.conjugate(particleName): configFilename}

    B2ERROR("Training of channel " + channelName + " failed")
    return {}


def VariablesToNTuple(path, particleName, particleList, signalProbability):
    """
    Saves the calculated signal probability for this particle list
        @param path the basf2 path
        @param particleName particleName
        @param particleList the particleList
        @param signalProbability signalProbability as additional dependency
    """

    if particleList is None or signalProbability is None:
        B2INFO("Write variables to ntuple for " + particleName + " and charged conjugated. But list is ignored.")
        return {'VariablesToNTuple': None}

    hash = actorFramework.createHash(particleName, particleList, signalProbability)
    filename = 'var_{particleName}_{hash}.root'.format(particleName=particleName, hash=hash)

    if not os.path.isfile(filename):
        output = register_module('VariablesToNtuple')
        output.param('particleList', particleList)
        output.param('variables', ['getExtraInfo(SignalProbability)', 'isSignal', 'Mbc', 'mcStatus'])
        output.param('fileName', filename)
        output.param('treeName', 'variables')
        path.add_module(output)
        B2INFO("Write variables to ntuple for " + particleName + " and charged conjugated.")
        return {}

    B2INFO("Write variables to ntuple for " + particleList + " and charged conjugated. But file already exists, so nothing to do here.")
    particleName = particleList.split(':')[0]
    return {'VariablesToNTuple_' + particleName: filename}


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
        B2INFO("Create pre cut histogram for channel " + channelName + " and charged conjugated. But channel is ignored.")
        return {'PreCutHistogram_' + channelName: None}

    # Check if the file is available. If the file isn't available yet, create it with
    # the HistMaker Module and return Nothing. If a function is called which depends on
    # the PreCutHistogram, the process will stop, because the PreCutHistogram isn't provided.
    hash = actorFramework.createHash(particleName, channelName, preCutConfig.variable, daughterLists, additionalDependencies)
    filename = 'CutHistograms_{pname}_{cname}_{hash}.root'.format(pname=particleName, cname=channelName, hash=hash)

    if os.path.isfile(filename):
        B2INFO("Create pre cut histogram for channel " + channelName + " and charged conjugated. But file already exists, so nothing to do here.")
        return {'PreCutHistogram_' + channelName: (filename, particleName + ':' + hash)}
    else:
        # Combine all the particles according to the decay channels
        outputList = particleName + ':' + hash + ' ==> ' + ' '.join(daughterLists)
        pmake = register_module('PreCutHistMaker')
        pmake.set_name('PreCutHistMaker_' + channelName)
        pmake.param('fileName', filename)
        pmake.param('decayString', outputList)
        if preCutConfig.variable in ['M']:
            mass = pdg.get(pdg.from_name(particleName)).Mass()
            pmake.param('variable', 'M')
            pmake.param('histParams', (200, mass / 2, mass + mass / 2))
        elif preCutConfig.variable in ['Q']:
            pmake.param('variable', 'Q')
            pmake.param('histParams', (200, -1, 1))
        else:
            pmake.param('variable', 'daughterProductOf(getExtraInfo(SignalProbability))')
            pmake.param('customBinning', list(reversed([1.0 / (1.5 ** i) for i in range(0, 20)])))
        path.add_module(pmake)

    B2INFO("Create pre cut histogram for channel " + channelName + " and charged conjugated.")
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
        results['PreCut_' + channel] = None if cut['isIgnored'] else cut

    B2INFO("Calculate pre cut for particle " + particleName + " and charged conjugated.")
    return results


def PostCutDetermination(particleName, postCutConfig, signalProbability):
    """
    Determines the PostCut of a particle.
        @param particleName name of the particle
        @param postCutConfig configuration for post cut determination
        @param signalProbability of the particle
    """
    B2INFO("Calculate post cut for particle " + particleName + " and charged conjugated.")
    return {'PostCut_' + particleName: {'cutstring': '0.1 < getExtraInfo(SignalProbability)', 'range': (0.1, 1)}}


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
        B2INFO("Write analysis tex file for channel " + channelName + " and charged conjugated. But channel is ignored.")
        return {'Tex_' + channelName: None}

    ROOT.gROOT.SetBatch(True)

    # Create TMVA Plots
    tmva_filename = signalProbability[:-7] + '.root'  # Strip .config of filename

    overtrainingPlot = channelList + '_overtraining.png'
    if not os.path.isfile(overtrainingPlot):
        subprocess.call(['root -l -q -b "$BELLE2_EXTERNALS_DIR/src/root/tmva/test/mvas.C(\\"{f}\\",3)"'.format(f=tmva_filename)], shell=True)
        subprocess.call(['cp plots/$(ls -t plots/ | head -1) {name}'.format(name=overtrainingPlot)], shell=True)

    rocPlot = channelList + '_roc.png'
    if not os.path.isfile(rocPlot):
        subprocess.call(['root -l -q -b "$BELLE2_EXTERNALS_DIR/src/root/tmva/test/efficiencies.C(\\"{f}\\")"'.format(f=tmva_filename)], shell=True)
        subprocess.call(['cp plots/$(ls -t plots/ | head -1) {name}'.format(name=rocPlot)], shell=True)

    # Create PreCut Hist Plots
    rootfile = ROOT.TFile(preCutHistogram[0])
    names = ['signal', 'all', 'background', 'ratio']
    keys = rootfile.GetListOfKeys()
    lc, uc = preCut['range']
    for (name, key) in zip(names, keys):
        plotName = channelList + '_' + name + '.png'
        if not os.path.isfile(plotName):
            canvas = ROOT.TCanvas(name + channelList + '_canvas', name, 600, 400)
            canvas.cd()
            hist = rootfile.Get(key.GetName())
            hist.Draw()
            ll = ROOT.TLine(lc, 0, lc, hist.GetMaximum())
            ul = ROOT.TLine(uc, 0, uc, hist.GetMaximum())
            ll.Draw()
            ul.Draw()
            canvas.SaveAs(plotName)

    # Create diag plots
    diagPlotFile = channelList + '_diag_' + mvaConfig.name + '.pdf'
    if not os.path.isfile(diagPlotFile):
        B2WARNING("plot " + diagPlotFile + " doesn't exist, creating it")
        makeDiagPlots(tmva_filename, channelList)

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
    placeholders['channelPurity'] = '{:.5f}'.format(purity)
    placeholders['channelEfficiency'] = '{:.3f}'.format(efficiency)

    placeholders['preCutVariable'] = preCutConfig.variable
    placeholders['preCutEfficiency'] = '{:.2f}'.format(preCutConfig.efficiency)
    a, b = preCut['range']
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

    placeholders['mvaROCPlot'] = rocPlot
    placeholders['mvaOvertrainingPlot'] = overtrainingPlot
    placeholders['mvaDiagPlot'] = diagPlotFile

    filename = channelList + '.tex'
    if not os.path.isfile(filename):
        template = Template(file(ROOT.Belle2.FileSystem.findFile('analysis/scripts/FullEventInterpretationChannelTemplate.tex'), 'r').read())
        page = template.substitute(placeholders)
        file(filename, 'w').write(page)
        B2INFO("Write analysis tex file for channel " + channelName + " and charged conjugated.")
    else:
        B2INFO("Write analysis tex file for channel " + channelName + " and charged conjugated. But file already exists, nothing to do here.")
    return {'Tex_' + channelName: (filename, placeholders)}


def WriteAnalysisFileForParticle(particleName, postCutConfig, postCut, texfiles):
    """
    Creates a pdf document with the PreCut and Training plots
        @param particleName name of the particle
        @param preCutConfig configuration for post cut
        @param preCut used postCut for this particle
        @param texfiles list of tex filenames
    """
    filename = particleName + '.tex'
    if not os.path.isfile(filename):

        placeholders = {}
        placeholders['NChannels'] = len(texfiles)
        texfiles = actorFramework.removeNones(texfiles)
        placeholders['NUsedChannels'] = len(texfiles)

        a, b = postCut['range']
        placeholders['postCutRange'] = 'Ignored' if postCut is None else '({:.2f},'.format(a) + ' {:.2f}'.format(b) + ')'

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

        file(filename, 'w').write(page)

        subprocess.call(['pdflatex', filename])
        B2INFO("Write analysis tex file and create pdf for particle " + particleName + " and charged conjugated.")
    else:
        B2INFO("Write analysis tex file and create pdf for particle " + particleName + " and charged conjugated. But file already exists, nothing to do here.")

    return {'PDF_' + particleName: filename[:-4] + '.pdf'}


def WriteAnalysisFileSummary(pdffiles, ntuples):
    """
    Creates a pdf summarizing all networks trained.
        @param texfiles list of tex filenames
    """

    #gather the previously produced PDFs
    fileList = []
    for fileName in pdffiles:
        #fileName = f[4:] + '.pdf'
        if os.path.isfile(fileName):
            fileList.append(fileName)

    #create Mbc plots
    B2WARNING("i got these ntuples: " + str(ntuples))
    for ntupleFile in ntuples:
        outputFile = ntupleFile[:-5] + '_mbc.pdf'
        makeMbcPlot(ntupleFile, outputFile)
        fileList.append(outputFile)

    #TODO: overall efficiencies in our channels

    subprocess.call(['pdfunite'] + fileList + ['FEIsummary.pdf'])

    # Return None - Therefore Particle List depends not on TMVAExpert directly
    return {'PDF_FEIsummary': None}
