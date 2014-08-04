#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
import ROOT
import pdg

import math
import actorFramework
import preCutDetermination
import os
import re
import subprocess
from string import Template


def removeJPsiSlash(filename):
    return filename.replace('/', '')


def purity(x, y):
    if x == 0:
        return 0.0
    return x / float(x + y)


def efficiency(x, y):
    if x == 0:
        return 0.0
    if y == 0:
        return float('inf')
    return x / float(y)


def createTexFile(filename, templateFilename, placeholders):
    """
    Creates tex file from template with the given placeholder values
    @param filename name of the file which is created
    @param templateFilename name of the template file
    @param placeholders dictionary with values for every palceholder in the template
    """
    template = Template(file(ROOT.Belle2.FileSystem.findFile(templateFilename), 'r').read())
    page = template.substitute(placeholders)
    file(filename, 'w').write(page)
    B2INFO("Write tex file " + filename + ".")


def createSummaryTexFile(finalStateParticlePlaceholders, combinedParticlePlaceholders, finalParticlePlaceholders, mcCounts, particles):

    placeholders = {}

    # Prettify config output
    input = '\n\n'.join([str(p) for p in particles])
    output = ''
    count = 0
    first_sep = 0
    for c in input:
        output += c
        count += 1
        if first_sep == 0 and c in ':':
            first_sep = count
        if c == '\n':
            count = 0
            first_sep = 0
        if count >= 75:
            if c in ':, ' or count == 90:
                output += '\n' + ' ' * first_sep
                count = first_sep

    placeholders['particleConfigurations'] = output

    placeholders['finalStateParticleInputs'] = ""
    placeholders['finalStateParticleEPTable'] = ""
    for particlePlaceholder in finalStateParticlePlaceholders:
        placeholders['finalStateParticleInputs'] += '\input{' + particlePlaceholder['texFile'] + '}\n'
        placeholders['finalStateParticleEPTable'] += particlePlaceholder['particleName'] + ' & '
        placeholders['finalStateParticleEPTable'] += '{:.1f}'.format(efficiency(particlePlaceholder['particleNSignal'], particlePlaceholder['particleNTrueSignal']) * 100) + ' & '
        placeholders['finalStateParticleEPTable'] += '{:.1f}'.format(efficiency(particlePlaceholder['particleNSignalAfterPostCut'], particlePlaceholder['particleNTrueSignal']) * 100) + ' & '
        placeholders['finalStateParticleEPTable'] += '{:.3f}'.format(purity(particlePlaceholder['particleNSignal'], particlePlaceholder['particleNBackground']) * 100) + ' & '
        placeholders['finalStateParticleEPTable'] += '{:.3f}'.format(purity(particlePlaceholder['particleNSignalAfterPostCut'], particlePlaceholder['particleNBackgroundAfterPostCut']) * 100) + r'\\' + '\n'

    placeholders['combinedParticleInputs'] = ""
    placeholders['combinedParticleEPTable'] = ""
    for particlePlaceholder in combinedParticlePlaceholders:
        placeholders['combinedParticleInputs'] += '\input{' + particlePlaceholder['texFile'] + '}\n'
        placeholders['combinedParticleEPTable'] += particlePlaceholder['particleName'] + ' & '
        placeholders['combinedParticleEPTable'] += '{:.1f}'.format(efficiency(particlePlaceholder['particleNSignal'], particlePlaceholder['particleNTrueSignal']) * 100) + ' & '
        placeholders['combinedParticleEPTable'] += '{:.1f}'.format(efficiency(particlePlaceholder['particleNSignalAfterPreCut'], particlePlaceholder['particleNTrueSignal']) * 100) + ' & '
        placeholders['combinedParticleEPTable'] += '{:.1f}'.format(efficiency(particlePlaceholder['particleNSignalAfterPostCut'], particlePlaceholder['particleNTrueSignal']) * 100) + ' & '
        placeholders['combinedParticleEPTable'] += '{:.3f}'.format(purity(particlePlaceholder['particleNSignal'], particlePlaceholder['particleNBackground']) * 100) + ' & '
        placeholders['combinedParticleEPTable'] += '{:.3f}'.format(purity(particlePlaceholder['particleNSignalAfterPreCut'], particlePlaceholder['particleNBackground']) * 100) + ' & '
        placeholders['combinedParticleEPTable'] += '{:.3f}'.format(purity(particlePlaceholder['particleNSignalAfterPostCut'], particlePlaceholder['particleNBackgroundAfterPostCut']) * 100) + r'\\' + '\n'

    placeholders['mbcInputs'] = ''
    for particlePlaceholder in finalParticlePlaceholders:
        placeholders['mbcInputs'] += '\input{' + particlePlaceholder['texFile'] + '}\n'

    placeholders['NSignal'] = 0
    placeholders['NBackground'] = 0
    placeholders['NEvents'] = mcCounts['NEvents']

    for bPlaceholder in combinedParticlePlaceholders:
        if bPlaceholder['particleName'] in ['B+', 'B0', 'B-', 'B0bar']:
            placeholders['NSignal'] += int(bPlaceholder['particleNSignal'])
            placeholders['NBackground'] += int(bPlaceholder['particleNBackground'])

    placeholders['overallSignalEfficiencyInPercent'] = efficiency(placeholders['NSignal'], placeholders['NEvents']) * 100

    hash = actorFramework.createHash(placeholders)
    placeholders['texFile'] = 'FEIsummary.tex'
    if not os.path.isfile(placeholders['texFile']):
        createTexFile(placeholders['texFile'], 'analysis/scripts/FullEventInterpretationSummaryTemplate.tex', placeholders)
    return placeholders


def createMBCTexFile(ntuple):
    """
    Creates a tex document with MBC Plot frmo the given ntuple
        @param ntuple the ntuple containing the needed information
    """
    placeholders = {}
    mbcFilename = ntuple[:-5] + '_mbc.pdf'
    makeMbcPlot(ntuple, mbcFilename)
    placeholders['mbcPlot'] = mbcFilename
    placeholders['texFile'] = ntuple[:-5] + '_mbc.tex'
    if not os.path.isfile(placeholders['texFile']):
        createTexFile(placeholders['texFile'], 'analysis/scripts/FullEventInterpretationMBCTemplate.tex', placeholders)
    return placeholders


def createFSParticleTexFile(placeholders, mcCounts):
    """
    Creates a tex document with Training plots
        @param mvaConfig configuration for mva
        @param signalProbability config filename for TMVA training
    """
    placeholders['particleNTrueSignal'] = mcCounts.get(str(pdg.from_name(placeholders['particleName'])), 0)
    placeholders['particleNSignal'] = placeholders['mvaNSignal']
    placeholders['particleNBackground'] = placeholders['mvaNBackground']
    placeholders['particleNSignalAfterPreCut'] = placeholders['mvaNSignal']
    placeholders['particleNBackgroundAfterPreCut'] = placeholders['mvaNBackground']
    placeholders['particleNSignalAfterPostCut'] = placeholders['mvaNSignalAfterPostCut']
    placeholders['particleNBackgroundAfterPostCut'] = placeholders['mvaNBackgroundAfterPostCut']

    hash = actorFramework.createHash(placeholders)
    placeholders['texFile'] = '{name}_{hash}.tex'.format(name=placeholders['particleName'], hash=hash)
    if not os.path.isfile(placeholders['texFile']):
        createTexFile(placeholders['texFile'], 'analysis/scripts/FullEventInterpretationFSParticleTemplate.tex', placeholders)

    return placeholders


def createCombinedParticleTexFile(placeholders, channelPlaceholders, mcCounts):
    """
    Creates a tex document with the PreCut and Training plots
        @param placeholders dictionary with values for every placeholder in the latex-template
        @param channelPlaceholders list of all tex placeholder dictionaries of all channels
    """
    placeholders['NChannels'] = len(channelPlaceholders)
    placeholders['NUsedChannels'] = len([y for y in channelPlaceholders if not y['isIgnored']])

    placeholders['channelListAsItems'] = ""
    for channelPlaceholder in channelPlaceholders:
        if channelPlaceholder['isIgnored'] is None:
            placeholders['channelListAsItems'] += "\\item \\verb|{name}| was ignored\n".format(name=channelPlaceholder['channelName'])
        else:
            placeholders['channelListAsItems'] += "\\item \\verb|{name}|\n".format(name=channelPlaceholder['channelName'])

    placeholders['channelInputs'] = ""
    placeholders['particleNSignal'] = 0
    placeholders['particleNBackground'] = 0
    placeholders['particleNSignalAfterPreCut'] = 0
    placeholders['particleNBackgroundAfterPreCut'] = 0
    placeholders['particleNSignalAfterPostCut'] = 0
    placeholders['particleNBackgroundAfterPostCut'] = 0
    placeholders['particleNTrueSignal'] = mcCounts.get(str(pdg.from_name(placeholders['particleName'])), 0)

    placeholders['postCutRange'] = 'Ignored'
    for channelPlaceholder in channelPlaceholders:
        placeholders['postCutRange'] = channelPlaceholder['postCutRange']
        placeholders['particleNSignal'] += int(channelPlaceholder['channelNSignal'])
        placeholders['particleNBackground'] += int(channelPlaceholder['channelNBackground'])
        placeholders['particleNSignalAfterPreCut'] += int(channelPlaceholder['channelNSignalAfterPreCut'])
        placeholders['particleNBackgroundAfterPreCut'] += int(channelPlaceholder['channelNBackgroundAfterPreCut'])
        placeholders['particleNSignalAfterPostCut'] += int(channelPlaceholder['mvaNSignalAfterPostCut'])
        placeholders['particleNBackgroundAfterPostCut'] += int(channelPlaceholder['mvaNBackgroundAfterPostCut'])
        placeholders['channelInputs'] += '\input{' + channelPlaceholder['texFile'] + '}\n'

    hash = actorFramework.createHash(placeholders)
    placeholders['texFile'] = removeJPsiSlash('{name}_{hash}.tex'.format(name=placeholders['particleName'], hash=hash))
    if not os.path.isfile(placeholders['texFile']):
        createTexFile(placeholders['texFile'], 'analysis/scripts/FullEventInterpretationCombinedParticleTemplate.tex', placeholders)

    return placeholders


def createPreCutTexFile(placeholders, preCutHistogram, preCutConfig, preCut):
    """
    Creates tex file for the PreCuts of this channel. Adds necessary items to the placeholder dictionary
    and returns the modified dictionary.
    @param placeholders dictionary with values for every placeholder in the latex-template
    @param preCutHistogram preCutHistogram (filename, histogram postfix)
    @param preCutConfig configuration for pre cut
    @param preCut used preCuts for this channel
    """
    if preCutHistogram is None:
        hash = actorFramework.createHash(placeholders)
        placeholders['preCutTexFile'] = '{name}_preCut_{hash}.tex'.format(name=placeholders['particleName'], hash=hash)
        placeholders['preCutTemplateFile'] = 'analysis/scripts/FullEventInterpretationMissingPreCutTemplate.tex'
        placeholders['isIgnored'] = True
        placeholders['channelPurity'] = 0
        placeholders['channelNSignal'] = 0
        placeholders['channelNBackground'] = 0
        placeholders['channelNSignalAfterPreCut'] = 0
        placeholders['channelNBackgroundAfterPreCut'] = 0
        placeholders['channelPurityAfterPreCut'] = 0
        placeholders['channelEfficiencyAfterPreCut'] = 0

        if 'ignoreReason' not in placeholders:
            placeholders['ignoreReason'] = """This channel was ignored because one or more daughter particles were ignored."""
    else:
        placeholders['preCutROOTFile'] = preCutHistogram[0]
        rootfile = ROOT.TFile(placeholders['preCutROOTFile'])
        # Calculate purity and efficiency for this channel
        signal_hist = rootfile.Get(rootfile.GetListOfKeys().At(0).GetName())
        background_hist = rootfile.Get(rootfile.GetListOfKeys().At(2).GetName())

        placeholders['channelNSignal'] = preCutDetermination.GetNumberOfEvents(signal_hist)
        placeholders['channelNBackground'] = preCutDetermination.GetNumberOfEvents(background_hist)
        placeholders['channelPurity'] = '{:.5f}'.format(purity(placeholders['channelNSignal'], placeholders['channelNBackground']))

        if preCut is not None:
            lc, uc = preCut['range']
            placeholders['preCutRange'] = '({:.2f},'.format(lc) + ' {:.2f}'.format(uc) + ')'
            placeholders['channelNSignalAfterPreCut'] = preCutDetermination.GetNumberOfEventsInRange(signal_hist, (lc, uc))
            placeholders['channelNBackgroundAfterPreCut'] = preCutDetermination.GetNumberOfEventsInRange(background_hist, (lc, uc))
            placeholders['channelPurityAfterPreCut'] = '{:.5f}'.format(purity(placeholders['channelNSignalAfterPreCut'], placeholders['channelNBackgroundAfterPreCut']))
            placeholders['channelEfficiencyAfterPreCut'] = '{:.5f}'.format(placeholders['channelNSignalAfterPreCut'] / placeholders['channelNSignal'])
        else:
            placeholders['preCutRange'] = '---'
            placeholders['channelNSignalAfterPreCut'] = 0
            placeholders['channelNBackgroundAfterPreCut'] = 0
            placeholders['channelPurityAfterPreCut'] = 0
            placeholders['channelEfficiencyAfterPreCut'] = 0

        placeholders['preCutVariable'] = preCutConfig.variable
        placeholders['preCutEfficiency'] = '{:.5f}'.format(preCutConfig.efficiency)
        placeholders['preCutPurity'] = '{:.5f}'.format(preCutConfig.purity)

        hash = actorFramework.createHash(placeholders)

        ROOT.gROOT.SetBatch(True)

        placeholders['preCutAllPlot'] = removeJPsiSlash('{name}_preCut_{hash}_all.png'.format(name=placeholders['particleName'], hash=hash))
        if not os.path.isfile(placeholders['preCutAllPlot']):
            makePreCutPlot(placeholders['preCutROOTFile'], placeholders['preCutAllPlot'], 0, preCut)

        placeholders['preCutSignalPlot'] = removeJPsiSlash('{name}_preCut_{hash}_signal.png'.format(name=placeholders['particleName'], hash=hash))
        if not os.path.isfile(placeholders['preCutSignalPlot']):
            makePreCutPlot(placeholders['preCutROOTFile'], placeholders['preCutSignalPlot'], 1, preCut)

        placeholders['preCutBackgroundPlot'] = removeJPsiSlash('{name}_preCut_{hash}_background.png'.format(name=placeholders['particleName'], hash=hash))
        if not os.path.isfile(placeholders['preCutBackgroundPlot']):
            makePreCutPlot(placeholders['preCutROOTFile'], placeholders['preCutBackgroundPlot'], 2, preCut)

        placeholders['preCutRatioPlot'] = removeJPsiSlash('{name}_preCut_{hash}_ratio.png'.format(name=placeholders['particleName'], hash=hash))
        if not os.path.isfile(placeholders['preCutRatioPlot']):
            makePreCutPlot(placeholders['preCutROOTFile'], placeholders['preCutRatioPlot'], 3, preCut)

        placeholders['preCutTexFile'] = removeJPsiSlash('{name}_preCut_{hash}.tex'.format(name=placeholders['particleName'], hash=hash))
        placeholders['preCutTemplateFile'] = 'analysis/scripts/FullEventInterpretationPreCutTemplate.tex'

    if not os.path.isfile(placeholders['preCutTexFile']):
        createTexFile(placeholders['preCutTexFile'], placeholders['preCutTemplateFile'], placeholders)

    return placeholders


def makePreCutPlot(rootFilename, plotName, number, preCut):
    rootfile = ROOT.TFile(rootFilename)
    canvas = ROOT.TCanvas(plotName + '_canvas', plotName, 600, 400)
    canvas.cd()
    key = rootfile.GetListOfKeys().At(number)
    hist = rootfile.Get(key.GetName())
    hist.Draw()
    if preCut is not None:
        lc, uc = preCut['range']
        ll = ROOT.TLine(lc, 0, lc, hist.GetMaximum())
        ul = ROOT.TLine(uc, 0, uc, hist.GetMaximum())
        ll.Draw()
        ul.Draw()
    canvas.SaveAs(plotName)


def createMVATexFile(placeholders, mvaConfig, signalProbability, postCutConfig, postCut):
    """
    Creates tex file for the MVA of a channel. Adds necessary items to the placeholder dictionary
    and returns the modified dictionary.
    @param placeholders dictionary with values for every placeholder in the latex-template
    @param mvaConfig MVAConfiguration object containing the config for the mva
    @param signalProbability config file of the TMVATeacher module
    @param postCut used postCut
    @param postCutConfig configuration of the postCut
    """

    if signalProbability is None:
        hash = actorFramework.createHash(placeholders)
        placeholders['mvaTexFile'] = '{name}_mva_{hash}.tex'.format(name=placeholders['particleName'], hash=hash)
        placeholders['mvaTemplateFile'] = 'analysis/scripts/FullEventInterpretationMissingMVATemplate.tex'
        placeholders['isIgnored'] = True
        placeholders['postCutRange'] = 'Ignored'
        placeholders['mvaNSignal'] = 0
        placeholders['mvaNBackground'] = 0
        placeholders['mvaNSignalAfterPostCut'] = 0
        placeholders['mvaNBackgroundAfterPostCut'] = 0
        if 'ignoreReason' not in placeholders:
            placeholders['ignoreReason'] = """Due too low statistics after the pre cut, we didn\'t perfom a training in this channel.
                                              This means there were less than 100 signal or 100 background events in the given sample.
                                              Or the determined pre cut range is smaller than 1e-4."""
    else:
        ROOT.gROOT.SetBatch(True)

        # Set mva placeholders
        placeholders['mvaROOTFilename'] = signalProbability[:-7] + '.root'  # Strip .config of filename
        placeholders['mvaName'] = mvaConfig.name
        placeholders['mvaType'] = mvaConfig.type
        placeholders['mvaConfig'] = mvaConfig.config
        placeholders['mvaTarget'] = mvaConfig.target
        placeholders['mvaTargetCluster'] = mvaConfig.targetCluster

        from variables import variables
        placeholders['mvaVariables'] = ''
        for v in mvaConfig.variables:
            varName = escapeForLatex(v)
            description = escapeForLatex(variables.getVariable(v).description)
            placeholders['mvaVariables'] += r'\texttt{' + varName + r'} & ' + description + r' \\'

        rootfile = ROOT.TFile(placeholders['mvaROOTFilename'])

        trainTree = rootfile.Get('TrainTree')
        placeholders['mvaNTrainSignal'] = int(trainTree.GetEntries('className == "Signal"'))
        placeholders['mvaNTrainBackground'] = int(trainTree.GetEntries('className == "Background"'))

        testTree = rootfile.Get('TestTree')
        placeholders['mvaNTestSignal'] = int(testTree.GetEntries('className == "Signal"'))
        placeholders['mvaNTestBackground'] = int(testTree.GetEntries('className == "Background"'))

        keys = [key.GetName() for key in rootfile.GetListOfKeys() if re.match('^{name}.*_tree$'.format(name=escapeForRegExp(placeholders['particleName'])), key.GetName()) is not None]
        if len(keys) != 1:
            raise RuntimeError("Couldn't find original tree for particle {name} in root file {f} created by tmva".format(name=placeholders['particleName'], f=placeholders['mvaROOTFilename']))
        originalTree = rootfile.Get(keys[0])
        placeholders['mvaNCandidates'] = originalTree.GetEntries()

        signalSelector = '({isSignal} == {signalCluster})'.format(isSignal=placeholders['mvaTarget'], signalCluster=placeholders['mvaTargetCluster'])
        backgroundSelector = '({isSignal} != {signalCluster})'.format(isSignal=placeholders['mvaTarget'], signalCluster=placeholders['mvaTargetCluster'])
        postCutSelector = '(prob_{name} > {postCut})'.format(name=placeholders['mvaName'], postCut=postCut['range'][0])
        placeholders['mvaNSignal'] = originalTree.GetEntries(signalSelector)
        placeholders['mvaNBackground'] = originalTree.GetEntries(backgroundSelector)

        placeholders['mvaNTestSignalAfterPostCut'] = testTree.GetEntries('className == "Signal"' + ' && ' + postCutSelector)
        placeholders['mvaNTestBackgroundAfterPostCut'] = testTree.GetEntries('className == "Background"' + ' && ' + postCutSelector)

        placeholders['mvaPostCutSignalEfficiency'] = efficiency(placeholders['mvaNTestSignalAfterPostCut'], placeholders['mvaNTestSignal'])
        placeholders['mvaPostCutBackgroundEfficiency'] = efficiency(placeholders['mvaNTestBackgroundAfterPostCut'], placeholders['mvaNTestBackground'])

        placeholders['mvaNSignalAfterPostCut'] = placeholders['mvaPostCutSignalEfficiency'] * placeholders['mvaNSignal']
        placeholders['mvaNBackgroundAfterPostCut'] = placeholders['mvaPostCutBackgroundEfficiency'] * placeholders['mvaNBackground']

        if postCut is not None:
            a, b = postCut['range']
            placeholders['postCutRange'] = '({:.5f},'.format(a) + ' {:.5f}'.format(b) + ')'
        else:
            placeholders['postCutRange'] = 'Ignored'

        # Create plots and texfile if hash changed
        hash = actorFramework.createHash(placeholders)

        placeholders['mvaOvertrainingPlot'] = removeJPsiSlash('{name}_mva_{hash}_overtraining.png'.format(name=placeholders['particleName'], hash=hash))
        if not os.path.isfile(placeholders['mvaOvertrainingPlot']):
            makeOvertrainingPlot(placeholders['mvaROOTFilename'], placeholders['mvaOvertrainingPlot'])

        placeholders['mvaROCPlot'] = removeJPsiSlash('{name}_mva_{hash}_roc.png'.format(name=placeholders['particleName'], hash=hash))
        if not os.path.isfile(placeholders['mvaROCPlot']):
            makeROCPlot(placeholders['mvaROOTFilename'], placeholders['mvaROCPlot'])

        placeholders['mvaDiagPlot'] = removeJPsiSlash('{name}_mva_{hash}_diag.png'.format(name=placeholders['particleName'], hash=hash))
        if not os.path.isfile(placeholders['mvaDiagPlot']):
            makeDiagPlot(placeholders['mvaROOTFilename'], placeholders['mvaDiagPlot'], placeholders['mvaName'])

        placeholders['mvaTexFile'] = removeJPsiSlash('{name}_mva_{hash}.tex'.format(name=placeholders['particleName'], hash=hash))
        placeholders['mvaTemplateFile'] = 'analysis/scripts/FullEventInterpretationMVATemplate.tex'

    if not os.path.isfile(placeholders['mvaTexFile']):
        createTexFile(placeholders['mvaTexFile'], placeholders['mvaTemplateFile'], placeholders)
    return placeholders


def escapeForLatex(someString):
    """
    Used to escape user strings for LaTex.
    """
    return someString.replace('\\', r'\\').replace('_', r'\_').replace('^', r'\^{}')


def escapeForRegExp(someString):
    """
    Used to escape user strings for regular expressions.
    """
    return someString.replace('*', r'\*').replace('+', r'\+')


def efficiencyError(k, n):
    """
    for an efficiency eps = k/n, this function calculates the
    standard deviation according to http://arxiv.org/abs/physics/0701199 .
    """

    variance = (k + 1) * (k + 2) / ((n + 2) * (n + 3)) - (k + 1) ** 2 / ((n + 2) ** 2)
    return math.sqrt(variance)


def makeOvertrainingPlot(tmvaFilename, plotName):
    subprocess.call(['root -l -q -b "$BELLE2_EXTERNALS_DIR/src/root/tmva/test/mvas.C(\\"{f}\\",3)"'.format(f=tmvaFilename)], shell=True)
    subprocess.call(['cp plots/$(ls -t plots/ | head -1) {name}'.format(name=plotName)], shell=True)


def makeROCPlot(tmvaFilename, plotName):
    subprocess.call(['root -l -q -b "$BELLE2_EXTERNALS_DIR/src/root/tmva/test/efficiencies.C(\\"{f}\\")"'.format(f=tmvaFilename)], shell=True)
    subprocess.call(['cp plots/$(ls -t plots/ | head -1) {name}'.format(name=plotName)], shell=True)


def makeDiagPlot(tmvaFilename, plotName, methodName):

    tmvaFile = ROOT.TFile(tmvaFilename)
    testTree = tmvaFile.Get('TestTree')

    if testTree.GetEntries() == 0:
        raise RuntimeError('TestTree is empty')

    nbins = 100
    varPrefix = ''  # there's also a prob_MethodName variable, but not sure what it is. it definitely looks odd.
    probabilityVar = varPrefix + methodName
    bgHist = ROOT.TH1F('background' + probabilityVar, 'background', nbins, 0.0, 1.0)
    testTree.Project('background' + probabilityVar, probabilityVar, 'className == "Background"')
    signalHist = ROOT.TH1F('signal' + probabilityVar, 'signal', nbins, 0.0, 1.0)
    testTree.Project('signal' + probabilityVar, probabilityVar, 'className == "Signal"')

    import array

    x = array.array('d')
    y = array.array('d')
    xerr = array.array('d')
    yerr = array.array('d')

    for i in range(1, nbins + 1):  # no under/overflow bins
        nSig = 1.0 * signalHist.GetBinContent(i)
        nBg = 1.0 * bgHist.GetBinContent(i)

        try:
            purity = nSig / (nSig + nBg)
            purityErr = efficiencyError(nSig, nSig + nBg)
        except ZeroDivisionError:
            purity = 0
            purityErr = 0
        binCenter = signalHist.GetXaxis().GetBinCenter(i)
        x.append(binCenter)
        y.append(purity)
        xerr.append(signalHist.GetXaxis().GetBinWidth(i) / 2)
        yerr.append(purityErr)

    purityPerBin = ROOT.TGraphErrors(len(x), x, y, xerr, yerr)

    plotTitle = 'Diagonal plot for ' + methodName
    canvas = ROOT.TCanvas(plotTitle + plotName, plotTitle, 600, 400)
    canvas.cd()

    purityPerBin.SetTitle(';' + probabilityVar + ' output;'
                          + 'purity per bin')
    purityPerBin.GetXaxis().SetRangeUser(0.0, 1.0)
    purityPerBin.GetYaxis().SetRangeUser(0.0, 1.0)
    purityPerBin.Draw('APZ')
    diagonal = ROOT.TLine(0.0, 0.0, 1.0, 1.0)
    diagonal.SetLineColor(ROOT.kAzure)
    diagonal.Draw()
    canvas.SaveAs(plotName)


def makeMbcPlot(fileName, outputFileName):
    """
    Using the TNTuple in 'fileName', save M_bc plot in 'outputFileName'.
    Shows effect of different cuts on SignalProbability, plus signal distribution.
    """
    ntupleFile = ROOT.TFile(fileName)
    ntupleName = 'variables'

    testTree = ntupleFile.Get(ntupleName)
    if testTree.GetEntries() == 0:
        raise RuntimeError('Couldn\'t find TNtuple "' + ntupleName + '" in file ' + ntupleFile)

    plotTitle = 'Mbc plot'
    ROOT.gStyle.SetOptStat(0)
    canvas = ROOT.TCanvas(plotTitle, plotTitle, 600, 400)
    canvas.cd()

    testTree.SetLineColor(ROOT.kBlack)
    testTree.Draw('Mbc', 'Mbc > 5.23', '')
    testTree.SetLineStyle(ROOT.kDotted)
    testTree.Draw('Mbc', '!isSignal', 'same')
    color = ROOT.kRed + 4
    for cut in [0.0001, 0.001, 0.01, 0.1, 0.5]:
        testTree.SetLineColor(int(color))
        testTree.SetLineStyle(ROOT.kSolid)
        testTree.Draw('Mbc', 'getExtraInfoSignalProbability > ' + str(cut), 'same')

        testTree.SetLineStyle(ROOT.kDotted)
        testTree.Draw('Mbc', 'getExtraInfoSignalProbability > ' + str(cut) + ' && !isSignal', 'same')
        color -= 1

    l = canvas.GetListOfPrimitives()
    for i in range(l.GetEntries()):
        hist = l[i]
        if isinstance(hist, ROOT.TH1F):
            hist.GetXaxis().SetRangeUser(5.24, 5.29)
            break

    canvas.BuildLegend(0.1, 0.65, 0.6, 0.9)

    canvas.SaveAs(outputFileName)
