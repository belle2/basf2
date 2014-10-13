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
import sys
import subprocess
import copy
from string import Template


def removeJPsiSlash(filename):
    return filename.replace('/', '')


def prettifyDecayString(decayString):
    substitutes = {
        '==>': '$\\to$',
        ':generic': '',
        'gamma': r'$\gamma$',
        'pi+': r'$\pi^+$',
        'pi-': r'$\pi^-$',
        'pi0': r'$\pi^0$',
        'K_S0': r'$K^0_S$',
        'mu+': r'$\mu^+$',
        'mu-': r'$\mu^-$',
        'K+': r'$K^+$',
        'K-': r'$K^-$',
        'e+': r'$e^+$',
        'e-': r'$e^-$',
        'J/psi': r'$J/\psi$',
        'D+': r'$D^+$',
        'D-': r'$D^-$',
        'D0': r'$D^0$',
        'D*+': r'$D^{+*}$',
        'D*-': r'$D^{-*}$',
        'D*0': r'$D^{0*}$',
        'D_s+': r'$D^+_s$',
        'D_s-': r'$D^-_s$',
        'D_s*+': r'$D^{+*}_s$',
        'D_s*-': r'$D^{-*}_s$',
        'anti-D0': r'$\bar{D^0}$',
        'anti-D*0': r'$\bar{D^{0*}}$',
        'B+': r'$B^+$',
        'B-': r'$B^-$',
        'B0': r'$B^0$',
        'anti-B0': r'$\bar{B^0}$'}
    for key, value in substitutes.iteritems():
        decayString = decayString.replace(key, value)
    return decayString


def formatTime(seconds):
    """
    Return string describing a duration in a natural format
    """
    minutes = int(seconds / 60)
    hours = int(minutes / 60)
    minutes %= 60
    ms = int(seconds * 1000) % 1000
    us = int(seconds * 1000 * 1000) % 1000
    seconds = int(seconds % 60)
    string = ''
    if hours != 0:
        string += "%dh" % (hours)
    if minutes != 0:
        string += "%dm" % (minutes)
    if seconds != 0 and hours == 0:
        string += "%ds" % (seconds)
    if ms != 0 and hours == 0 and minutes == 0 and seconds == 0:
        string += "%dms" % (ms)
    if us != 0 and hours == 0 and minutes == 0 and seconds == 0 and ms == 0:
        string += "%d$\mu$s" % (us)

    if hours == 0 and minutes == 0 and seconds == 0 and ms == 0 and us == 0:
        string += '$<1\mu$s'
    return string


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
    placeholders = copy.copy(placeholders)
    if 'particleName' in placeholders:
        placeholders['particleName'] = prettifyDecayString(placeholders['particleName'])
    if 'channelName' in placeholders:
        placeholders['channelName'] = prettifyDecayString(placeholders['channelName'])
    template = Template(file(ROOT.Belle2.FileSystem.findFile(templateFilename), 'r').read())
    page = template.substitute(placeholders)
    file(filename, 'w').write(page)
    B2INFO("Write tex file " + filename + ".")


def createSummaryTexFile(finalStateParticlePlaceholders, combinedParticlePlaceholders, finalParticlePlaceholders, cpuTimeSummaryPlaceholders, mcCounts, particles):

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
        placeholders['finalStateParticleEPTable'] += '$' + particlePlaceholder['particleName'] + '$ & '
        placeholders['finalStateParticleEPTable'] += '{:.1f}'.format(efficiency(particlePlaceholder['particleNSignal'], particlePlaceholder['particleNTrueSignal']) * 100) + ' & '
        placeholders['finalStateParticleEPTable'] += '{:.1f}'.format(efficiency(particlePlaceholder['particleNSignalAfterPostCut'], particlePlaceholder['particleNTrueSignal']) * 100) + ' & '
        placeholders['finalStateParticleEPTable'] += '{:.3f}'.format(purity(particlePlaceholder['particleNSignal'], particlePlaceholder['particleNBackground']) * 100) + ' & '
        placeholders['finalStateParticleEPTable'] += '{:.3f}'.format(purity(particlePlaceholder['particleNSignalAfterPostCut'], particlePlaceholder['particleNBackgroundAfterPostCut']) * 100) + r'\\' + '\n'

    placeholders['combinedParticleInputs'] = ""
    placeholders['combinedParticleEPTable'] = ""
    for particlePlaceholder in combinedParticlePlaceholders:
        placeholders['combinedParticleInputs'] += '\input{' + particlePlaceholder['texFile'] + '}\n'
        placeholders['combinedParticleEPTable'] += '$' + particlePlaceholder['particleName'] + '$ & '
        placeholders['combinedParticleEPTable'] += '{:.1f}'.format(efficiency(particlePlaceholder['particleNSignal'], particlePlaceholder['particleNTrueSignal']) * 100) + ' & '
        placeholders['combinedParticleEPTable'] += '{:.1f}'.format(efficiency(particlePlaceholder['particleNSignalAfterUserCut'], particlePlaceholder['particleNTrueSignal']) * 100) + ' & '
        placeholders['combinedParticleEPTable'] += '{:.1f}'.format(efficiency(particlePlaceholder['particleNSignalAfterPreCut'], particlePlaceholder['particleNTrueSignal']) * 100) + ' & '
        placeholders['combinedParticleEPTable'] += '{:.1f}'.format(efficiency(particlePlaceholder['particleNSignalAfterPostCut'], particlePlaceholder['particleNTrueSignal']) * 100) + ' & '
        placeholders['combinedParticleEPTable'] += '{:.3f}'.format(purity(particlePlaceholder['particleNSignal'], particlePlaceholder['particleNBackground']) * 100) + ' & '
        placeholders['combinedParticleEPTable'] += '{:.3f}'.format(purity(particlePlaceholder['particleNSignalAfterUserCut'], particlePlaceholder['particleNBackgroundAfterUserCut']) * 100) + ' & '
        placeholders['combinedParticleEPTable'] += '{:.3f}'.format(purity(particlePlaceholder['particleNSignalAfterPreCut'], particlePlaceholder['particleNBackgroundAfterPreCut']) * 100) + ' & '
        placeholders['combinedParticleEPTable'] += '{:.3f}'.format(purity(particlePlaceholder['particleNSignalAfterPostCut'], particlePlaceholder['particleNBackgroundAfterPostCut']) * 100) + r'\\' + '\n'

    placeholders['mbcInputs'] = ''
    for particlePlaceholder in finalParticlePlaceholders:
        placeholders['mbcInputs'] += '\input{' + particlePlaceholder['texFile'] + '}\n'

    placeholders['cpuTimeSummary'] = '\input{' + cpuTimeSummaryPlaceholders['texFile'] + '}\n'

    placeholders['NSignal'] = 0
    placeholders['NBackground'] = 0
    placeholders['NEvents'] = mcCounts['NEvents']

    for bPlaceholder in combinedParticlePlaceholders:
        if bPlaceholder['particleName'] in ['B+', 'B0', 'B-', 'anti-B0']:
            placeholders['NSignal'] += int(bPlaceholder['particleNSignalAfterPostCut'])
            placeholders['NBackground'] += int(bPlaceholder['particleNBackgroundAfterPostCut'])

    placeholders['overallSignalEfficiencyInPercent'] = efficiency(placeholders['NSignal'], placeholders['NEvents']) * 100

    hash = actorFramework.create_hash([placeholders])
    placeholders['texFile'] = 'FEIsummary.tex'
    if not os.path.isfile(placeholders['texFile']):
        createTexFile(placeholders['texFile'], 'analysis/scripts/FEI/templates/SummaryTemplate.tex', placeholders)
    return placeholders


def createMoneyPlotTexFile(ntuple, type):
    """
    Creates a tex document with MBC or CosBDL Plot from the given ntuple
        @param ntuple the ntuple containing the needed information
    """
    placeholders = {}
    moneyFilename = ntuple[:-5] + '_money.pdf'
    if type == 'Mbc':
        template_file = 'analysis/scripts/FEI/templates/MBCTemplate.tex'
        if not os.path.isfile(moneyFilename):
            makeMbcPlot(ntuple, moneyFilename)
    elif type == 'CosBDL':
        template_file = 'analysis/scripts/FEI/templates/CosBDLTemplate.tex'
        if not os.path.isfile(moneyFilename):
            makeCosBDLPlot(ntuple, moneyFilename)
    else:
        raise RuntimeError('Unkown money plot type')
    placeholders['moneyPlot'] = moneyFilename
    placeholders['texFile'] = ntuple[:-5] + '_money.tex'
    placeholders['particleName'] = moneyFilename[4:].split(':', 1)[0]
    if not os.path.isfile(placeholders['texFile']):
        createTexFile(placeholders['texFile'], template_file, placeholders)
    return placeholders


def createFSParticleTexFile(placeholders, nTuple, mcCounts, distribution, mvaConfig):
    """
    Creates a tex document with Training plots
        @param mvaConfig configuration for mva
    """
    placeholders['particleNTrueSignal'] = mcCounts.get(str(abs(pdg.from_name(placeholders['particleName']))), 0)
    placeholders['particleNSignal'] = distribution['nSignal']
    placeholders['particleNBackground'] = distribution['nBackground']
    placeholders['particleNSignalAfterPreCut'] = distribution['nSignal']
    placeholders['particleNBackgroundAfterPreCut'] = distribution['nBackground']

    rootfile = ROOT.TFile(nTuple)
    tree = rootfile.Get('variables')
    placeholders['particleNSignalAfterPostCut'] = int(tree.GetEntries('isSignal'))
    placeholders['particleNBackgroundAfterPostCut'] = int(tree.GetEntries('!isSignal'))

    hash = actorFramework.create_hash([placeholders])
    placeholders['particleDiagPlot'] = removeJPsiSlash('{name}_combined_{hash}_diag.png'.format(name=placeholders['particleName'], hash=hash))
    if not os.path.isfile(placeholders['particleDiagPlot']):
        makeDiagPlotPerParticle(nTuple, placeholders['particleDiagPlot'], mvaConfig)
    placeholders['texFile'] = '{name}_{hash}.tex'.format(name=placeholders['particleName'], hash=hash)
    if not os.path.isfile(placeholders['texFile']):
        createTexFile(placeholders['texFile'], 'analysis/scripts/FEI/templates/FSParticleTemplate.tex', placeholders)

    return placeholders


def createCombinedParticleTexFile(placeholders, channelPlaceholders, nTuple, mcCounts, mvaConfig):
    """
    Creates a tex document with the PreCut and Training plots
        @param placeholders dictionary with values for every placeholder in the latex-template
        @param channelPlaceholders list of all tex placeholder dictionaries of all channels
    """
    print channelPlaceholders
    placeholders['NChannels'] = len(channelPlaceholders)
    placeholders['NUsedChannels'] = len([y for y in channelPlaceholders if not y['isIgnored']])

    placeholders['channelListAsItems'] = ""
    for channelPlaceholder in channelPlaceholders:
        if channelPlaceholder['isIgnored']:
            placeholders['channelListAsItems'] += "\\item {name} was ignored\n".format(name=prettifyDecayString(channelPlaceholder['channelName']))
        else:
            placeholders['channelListAsItems'] += "\\item {name}\n".format(name=prettifyDecayString(channelPlaceholder['channelName']))

    placeholders['channelInputs'] = ""
    placeholders['particleNSignal'] = 0
    placeholders['particleNBackground'] = 0
    placeholders['particleNSignalAfterUserCut'] = 0
    placeholders['particleNBackgroundAfterUserCut'] = 0
    placeholders['particleNSignalAfterPreCut'] = 0
    placeholders['particleNBackgroundAfterPreCut'] = 0
    placeholders['particleNSignalAfterPostCut'] = 0
    placeholders['particleNBackgroundAfterPostCut'] = 0
    placeholders['particleNTrueSignal'] = mcCounts.get(str(pdg.from_name(placeholders['particleName'])), 0)

    if placeholders['NUsedChannels'] > 0:
        ranges = [channelPlaceholder['postCutRange'] for channelPlaceholder in channelPlaceholders if channelPlaceholder['postCutRange'] != 'Ignored']
        if len(ranges) == 0:
            placeholders['postCutRange'] = 'Deactivated'
        else:
            if not all(ranges[0] == r for r in ranges):
                B2WARNING("Showing different post cuts for channels of the same particle in the summary file, isn't supported at the moment. Show only first cut.")
            placeholders['postCutRange'] = ranges[0]
    else:
        placeholders['postCutRange'] = 'Ignored'

    if nTuple is not None:
        rootfile = ROOT.TFile(nTuple)
        tree = rootfile.Get('variables')
        placeholders['particleNSignalAfterPostCut'] = int(tree.GetEntries(mvaConfig.target))
        placeholders['particleNBackgroundAfterPostCut'] = int(tree.GetEntries('!' + mvaConfig.target))
    else:
        placeholders['particleNSignalAfterPostCut'] = 0
        placeholders['particleNBackgroundAfterPostCut'] = 0

    for channelPlaceholder in channelPlaceholders:
        placeholders['particleNSignal'] += int(channelPlaceholder['channelNSignal'])
        placeholders['particleNBackground'] += int(channelPlaceholder['channelNBackground'])
        if not placeholders['isIgnored']:
            placeholders['particleNSignalAfterPreCut'] += int(channelPlaceholder['channelNSignalAfterPreCut'])
            placeholders['particleNBackgroundAfterPreCut'] += int(channelPlaceholder['channelNBackgroundAfterPreCut'])
            placeholders['particleNSignalAfterUserCut'] += int(channelPlaceholder['channelNSignalAfterUserCut'])
            placeholders['particleNBackgroundAfterUserCut'] += int(channelPlaceholder['channelNBackgroundAfterUserCut'])
        placeholders['channelInputs'] += '\input{' + channelPlaceholder['texFile'] + '}\n'

    hash = actorFramework.create_hash([placeholders])
    placeholders['particleDiagPlot'] = removeJPsiSlash('{name}_combined_{hash}_diag.png'.format(name=placeholders['particleName'], hash=hash))
    if not os.path.isfile(placeholders['particleDiagPlot']):
        makeDiagPlotPerParticle(nTuple, placeholders['particleDiagPlot'], mvaConfig)
    placeholders['texFile'] = removeJPsiSlash('{name}_{hash}.tex'.format(name=placeholders['particleName'], hash=hash))
    if not os.path.isfile(placeholders['texFile']):
        createTexFile(placeholders['texFile'], 'analysis/scripts/FEI/templates/CombinedParticleTemplate.tex', placeholders)

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
        hash = actorFramework.create_hash([placeholders])
        placeholders['preCutTexFile'] = removeJPsiSlash('{name}_preCut_{hash}.tex'.format(name=placeholders['particleName'], hash=hash))
        placeholders['preCutTemplateFile'] = 'analysis/scripts/FEI/templates/MissingPreCutTemplate.tex'
        placeholders['isIgnored'] = True
        placeholders['channelPurity'] = 0
        placeholders['channelNSignal'] = 0
        placeholders['channelNBackground'] = 0
        placeholders['channelNSignalAfterPreCut'] = 0
        placeholders['channelNBackgroundAfterPreCut'] = 0
        placeholders['channelNSignalAfterUserCut'] = 0
        placeholders['channelNBackgroundAfterUserCut'] = 0
        placeholders['channelPurityAfterPreCut'] = 0
        placeholders['channelEfficiencyAfterPreCut'] = 0
        placeholders['channelPurityAfterUserCut'] = 0
        placeholders['channelEfficiencyAfterUserCut'] = 0

        if 'ignoreReason' not in placeholders:
            placeholders['ignoreReason'] = """This channel was ignored because one or more daughter particles were ignored."""
    else:
        placeholders['preCutROOTFile'] = preCutHistogram[0]
        rootfile = ROOT.TFile(placeholders['preCutROOTFile'])
        # Calculate purity and efficiency for this channel
        without_userCut_hist = rootfile.GetListOfKeys().At(0).ReadObj()
        signal_hist = rootfile.GetListOfKeys().At(1).ReadObj()
        background_hist = rootfile.GetListOfKeys().At(3).ReadObj()

        placeholders['channelNSignal'] = without_userCut_hist.GetBinContent(2)
        placeholders['channelNBackground'] = without_userCut_hist.GetBinContent(1) - without_userCut_hist.GetBinContent(2)
        placeholders['channelPurity'] = '{:.5f}'.format(purity(placeholders['channelNSignal'], placeholders['channelNBackground']))

        placeholders['channelNSignalAfterUserCut'] = preCutDetermination.GetNumberOfEvents(signal_hist)
        placeholders['channelNBackgroundAfterUserCut'] = preCutDetermination.GetNumberOfEvents(background_hist)
        placeholders['channelAfterUserPurity'] = '{:.5f}'.format(purity(placeholders['channelNSignalAfterUserCut'], placeholders['channelNBackgroundAfterUserCut']))

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

        hash = actorFramework.create_hash([placeholders])

        ROOT.gROOT.SetBatch(True)

        placeholders['preCutAllPlot'] = removeJPsiSlash('{name}_preCut_{hash}_all.png'.format(name=placeholders['particleName'], hash=hash))
        if not os.path.isfile(placeholders['preCutAllPlot']):
            makePreCutPlot(placeholders['preCutROOTFile'], placeholders['preCutAllPlot'], 'all', preCut)

        placeholders['preCutSignalPlot'] = removeJPsiSlash('{name}_preCut_{hash}_signal.png'.format(name=placeholders['particleName'], hash=hash))
        if not os.path.isfile(placeholders['preCutSignalPlot']):
            makePreCutPlot(placeholders['preCutROOTFile'], placeholders['preCutSignalPlot'], 'signal', preCut)

        placeholders['preCutBackgroundPlot'] = removeJPsiSlash('{name}_preCut_{hash}_background.png'.format(name=placeholders['particleName'], hash=hash))
        if not os.path.isfile(placeholders['preCutBackgroundPlot']):
            makePreCutPlot(placeholders['preCutROOTFile'], placeholders['preCutBackgroundPlot'], 'bckgrd', preCut)

        placeholders['preCutRatioPlot'] = removeJPsiSlash('{name}_preCut_{hash}_ratio.png'.format(name=placeholders['particleName'], hash=hash))
        if not os.path.isfile(placeholders['preCutRatioPlot']):
            makePreCutPlot(placeholders['preCutROOTFile'], placeholders['preCutRatioPlot'], 'ratio', preCut)

        placeholders['preCutTexFile'] = removeJPsiSlash('{name}_preCut_{hash}.tex'.format(name=placeholders['particleName'], hash=hash))
        placeholders['preCutTemplateFile'] = 'analysis/scripts/FEI/templates/PreCutTemplate.tex'

    if not os.path.isfile(placeholders['preCutTexFile']):
        createTexFile(placeholders['preCutTexFile'], placeholders['preCutTemplateFile'], placeholders)

    return placeholders


def getKey(rootFile, regexp):
    """
    Return TKey in given TFile that matches regexp. If not exactly one matching key is found, throw exception.
    """
    keys = [key for key in rootFile.GetListOfKeys() if re.match(regexp, key.GetName()) is not None]
    if len(keys) != 1:
        raise RuntimeError("Couldn't find key matching {regexp} in root file {f}".format(regexp=regexp, f=rootFile.GetName()))
    return keys[0]


def makePreCutPlot(rootFilename, plotName, prefix, preCut):
    rootfile = ROOT.TFile(rootFilename)
    canvas = ROOT.TCanvas(plotName + '_canvas', plotName, 600, 400)
    canvas.cd()
    hist = getKey(rootfile, '^{name}.*$'.format(name=prefix)).ReadObj()
    if preCut is not None:
        lc, uc = preCut['range']
        d = uc - lc
        lr = lc - 4 * d
        ur = uc + 4 * d
        lm = hist.GetXaxis().GetXmin()
        um = hist.GetXaxis().GetXmax()
        hist.GetXaxis().SetRangeUser(lr if lr > lm else lm, ur if ur < um else um)
        hist.Draw()
        ll = ROOT.TLine(lc if lc > lm else lm, 0, lc if lc > lm else lm, hist.GetMaximum())
        ul = ROOT.TLine(uc if uc < um else um, 0, uc if uc < um else um, hist.GetMaximum())
        ll.Draw()
        ul.Draw()
    else:
        hist.Draw()
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
        hash = actorFramework.create_hash([placeholders])
        placeholders['mvaTexFile'] = removeJPsiSlash('{name}_mva_{hash}.tex'.format(name=placeholders['particleName'], hash=hash))
        placeholders['mvaTemplateFile'] = 'analysis/scripts/FEI/templates/MissingMVATemplate.tex'
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
        placeholders['mvaTMVAFilename'] = signalProbability[:-7] + '_1.root'  # Strip .config of filename
        placeholders['mvaLogFilename'] = signalProbability[:-7] + '.log'  # Strip .config of filename
        placeholders['mvaName'] = mvaConfig.name
        placeholders['mvaType'] = mvaConfig.type
        placeholders['mvaConfig'] = addHyphenations(mvaConfig.config)
        placeholders['mvaTarget'] = mvaConfig.target

        from variables import variables
        ranking = getMVARankingFromLogfile(placeholders['mvaLogFilename'])
        lines_by_rank = dict()
        for v in mvaConfig.variables:
            rootName = ROOT.Belle2.Variable.makeROOTCompatible(v)
            if rootName in ranking:
                rank, value = ranking[rootName]
                rankStr = str(rank)
                value = '{:.2f}'.format(value)
            else:
                rank = 900 + len(lines_by_rank)  # nonsensical value to add it at the end
                rankStr = ''
                value = ''
            varName = addHyphenations(escapeForLatex(v))
            description = escapeForLatex(variables.getVariable(v).description)
            if rank in lines_by_rank:
                raise runtime_error("Rank %d occurs more than once! Something is wrong in TMVA or in our parsing of the variable ranking." % (rank))
            lines_by_rank[rank] = varName + ' & ' + description + ' & ' + rankStr + '& ' + value + r' \\'

        placeholders['mvaVariables'] = ''
        for key in sorted(lines_by_rank):
            placeholders['mvaVariables'] += lines_by_rank[key]

        rootfile = ROOT.TFile(placeholders['mvaTMVAFilename'])

        trainTree = rootfile.Get('TrainTree')
        placeholders['mvaNTrainSignal'] = int(trainTree.GetEntries('className == "Signal"'))
        placeholders['mvaNTrainBackground'] = int(trainTree.GetEntries('className == "Background"'))

        testTree = rootfile.Get('TestTree')
        placeholders['mvaNTestSignal'] = int(testTree.GetEntries('className == "Signal"'))
        placeholders['mvaNTestBackground'] = int(testTree.GetEntries('className == "Background"'))

        variablefile = ROOT.TFile(placeholders['mvaROOTFilename'])
        keys = [key for key in variablefile.GetListOfKeys() if re.match('^{name}.*_tree$'.format(name=removeJPsiSlash(escapeForRegExp(placeholders['particleName']))), key.GetName()) is not None]
        if len(keys) != 1:
            print [k for k in variablefile.GetListOfKeys()]
            raise RuntimeError("Couldn't find original tree for particle {name} in root file {f} created by tmva".format(name=placeholders['particleName'], f=placeholders['mvaROOTFilename']))
        originalTree = keys[0].ReadObj()
        placeholders['mvaNCandidates'] = originalTree.GetEntries()

        signalSelector = '({isSignal} == 1)'.format(isSignal=placeholders['mvaTarget'])
        backgroundSelector = '({isSignal} != 1)'.format(isSignal=placeholders['mvaTarget'])
        placeholders['mvaNSignal'] = originalTree.GetEntries(signalSelector)
        placeholders['mvaNBackground'] = originalTree.GetEntries(backgroundSelector)

        if postCut is not None:
            a, b = postCut['range']
            placeholders['postCutRange'] = '({:.5f},'.format(a) + ' {:.5f}'.format(b) + ')'
            postCutSelector = '(prob_{name} > {postCut})'.format(name=placeholders['mvaName'], postCut=postCut['range'][0])
            placeholders['mvaNTestSignalAfterPostCut'] = testTree.GetEntries('className == "Signal"' + ' && ' + postCutSelector)
            placeholders['mvaNTestBackgroundAfterPostCut'] = testTree.GetEntries('className == "Background"' + ' && ' + postCutSelector)
        else:
            placeholders['postCutRange'] = 'Ignored'
            placeholders['mvaNTestSignalAfterPostCut'] = testTree.GetEntries('className == "Signal"')
            placeholders['mvaNTestBackgroundAfterPostCut'] = testTree.GetEntries('className == "Background"')

        placeholders['mvaPostCutSignalEfficiency'] = efficiency(placeholders['mvaNTestSignalAfterPostCut'], placeholders['mvaNTestSignal'])
        placeholders['mvaPostCutBackgroundEfficiency'] = efficiency(placeholders['mvaNTestBackgroundAfterPostCut'], placeholders['mvaNTestBackground'])

        placeholders['mvaNSignalAfterPostCut'] = placeholders['mvaPostCutSignalEfficiency'] * placeholders['mvaNSignal']
        placeholders['mvaNBackgroundAfterPostCut'] = placeholders['mvaPostCutBackgroundEfficiency'] * placeholders['mvaNBackground']

        # Create plots and texfile if hash changed
        hash = actorFramework.create_hash([placeholders])

        placeholders['mvaOvertrainingPlot'] = removeJPsiSlash('{name}_mva_{hash}_overtraining.png'.format(name=placeholders['particleName'], hash=hash))
        if not os.path.isfile(placeholders['mvaOvertrainingPlot']):
            makeOvertrainingPlot(placeholders['mvaTMVAFilename'], placeholders['mvaOvertrainingPlot'])

        placeholders['mvaROCPlot'] = removeJPsiSlash('{name}_mva_{hash}_roc.png'.format(name=placeholders['particleName'], hash=hash))
        if not os.path.isfile(placeholders['mvaROCPlot']):
            makeROCPlot(placeholders['mvaTMVAFilename'], placeholders['mvaROCPlot'])

        placeholders['mvaDiagPlot'] = removeJPsiSlash('{name}_mva_{hash}_diag.png'.format(name=placeholders['particleName'], hash=hash))
        if not os.path.isfile(placeholders['mvaDiagPlot']):
            makeDiagPlotPerChannel(placeholders['mvaTMVAFilename'], placeholders['mvaDiagPlot'], placeholders['mvaName'])

        placeholders['mvaTexFile'] = removeJPsiSlash('{name}_mva_{hash}.tex'.format(name=placeholders['particleName'], hash=hash))
        placeholders['mvaTemplateFile'] = 'analysis/scripts/FEI/templates/MVATemplate.tex'

    if not os.path.isfile(placeholders['mvaTexFile']):
        createTexFile(placeholders['mvaTexFile'], placeholders['mvaTemplateFile'], placeholders)
    return placeholders


def escapeForLatex(someString):
    """
    Used to escape user strings for LaTex.
    """
    return someString.replace('\\', r'\\').replace('_', r'\_').replace('^', r'\^{}')


def addHyphenations(someString):
    """
    Adds hyphenations after brackets, and for common variables.
    """
    substitutes = {
        '=': r'=\allowbreak ',
        ':': r':\allowbreak ',
        '(': r'(\allowbreak ',
        'getExtraInfo': r'get\-Ex\-tra\-In\-fo',
        'SignalProbability': r'Sig\-nal\-Prob\-a\-bil\-i\-ty',
        'cosAngleBetweenMomentumAndVertexVector': r'cosAngle\-Between\-Momentum\-And\-Vertex\-Vector'}
    for key, value in substitutes.iteritems():
        someString = someString.replace(key, value)
    return someString


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


def makeDiagPlotPerParticle(nTuple, plotName, mvaConfig):

    nbins = 100
    probabilityVar = 'getExtraInfoSignalProbability'  # ROOT.Belle2.Variable.makeROOTCompatible('getExtraInfo(SignalProbability)')
    bgHist = ROOT.TH1D('background' + probabilityVar, 'background', nbins, 0.0, 1.0)
    signalHist = ROOT.TH1D('signal' + probabilityVar, 'signal', nbins, 0.0, 1.0)

    if nTuple is not None:
        nTupleFile = ROOT.TFile(nTuple)
        variables = nTupleFile.Get('variables')

        if variables.GetEntries() == 0:
            raise RuntimeError('variables is empty')
        variables.Project('background' + probabilityVar, probabilityVar, '!' + mvaConfig.target)
        variables.Project('signal' + probabilityVar, probabilityVar, mvaConfig.target)
    makeDiagPlot(signalHist, bgHist, plotName)


def makeDiagPlotPerChannel(tmvaFilename, plotName, methodName):

    tmvaFile = ROOT.TFile(tmvaFilename)
    testTree = tmvaFile.Get('TestTree')

    if testTree.GetEntries() == 0:
        raise RuntimeError('TestTree is empty')

    nbins = 100
    varPrefix = ''  # there's also a prob_MethodName variable, but not sure what it is. it definitely looks odd.
    probabilityVar = varPrefix + methodName
    bgHist = ROOT.TH1D('background' + probabilityVar, 'background', nbins, 0.0, 1.0)
    testTree.Project('background' + probabilityVar, probabilityVar, 'className == "Background"')
    signalHist = ROOT.TH1D('signal' + probabilityVar, 'signal', nbins, 0.0, 1.0)
    testTree.Project('signal' + probabilityVar, probabilityVar, 'className == "Signal"')
    makeDiagPlot(signalHist, bgHist, plotName)


def makeDiagPlot(signalHist, bgHist, plotName):
    nbins = 100
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

    plotTitle = 'Diagonal plot'
    canvas = ROOT.TCanvas(plotTitle + plotName, plotTitle, 600, 400)
    canvas.cd()

    purityPerBin.SetTitle(';classifier output;'
                          + 'purity per bin')
    purityPerBin.GetXaxis().SetRangeUser(0.0, 1.0)
    purityPerBin.GetYaxis().SetRangeUser(0.0, 1.0)
    purityPerBin.Draw('APZ')
    diagonal = ROOT.TLine(0.0, 0.0, 1.0, 1.0)
    diagonal.SetLineColor(ROOT.kAzure)
    diagonal.Draw()
    canvas.SaveAs(plotName)


def makeCosBDLPlot(fileName, outputFileName):
    """
    Using the TNTuple in 'fileName', save CosThetaBDL plot in 'outputFileName'.
    Shows effect of different cuts on SignalProbability, plus signal distribution.
    """
    ntupleFile = ROOT.TFile(fileName)
    ntupleName = 'variables'

    testTree = ntupleFile.Get(ntupleName)
    if testTree.GetEntries() == 0:
        raise RuntimeError('Couldn\'t find TNtuple "' + ntupleName + '" in file ' + ntupleFile)

    plotTitle = 'CosThetaBDL plot'
    ROOT.gStyle.SetOptStat(0)
    canvas = ROOT.TCanvas(plotTitle, plotTitle, 600, 400)
    canvas.cd()

    #testTree.SetLineColor(ROOT.kBlack)
    #testTree.Draw('Mbc', 'Mbc > 5.23', '')
    #testTree.SetLineStyle(ROOT.kDotted)
    #testTree.Draw('Mbc', '!isSignal', 'same')
    color = ROOT.kRed + 4
    first_plot = True
    for cut in [0.01, 0.1, 0.5]:
        testTree.SetLineColor(int(color))
        testTree.SetLineStyle(ROOT.kSolid)
        testTree.Draw('cosThetaBetweenParticleAndTrueB', 'abs(cosThetaBetweenParticleAndTrueB) < 10 && getExtraInfoSignalProbability > ' + str(cut), '' if first_plot else 'same')
        first_plot = False

        testTree.SetLineStyle(ROOT.kDotted)
        testTree.Draw('cosThetaBetweenParticleAndTrueB', 'abs(cosThetaBetweenParticleAndTrueB) < 10 && getExtraInfoSignalProbability > ' + str(cut) + ' && !isSignalAcceptMissingNeutrino', 'same')
        color -= 1

    l = canvas.GetListOfPrimitives()
    for i in range(l.GetEntries()):
        hist = l[i]
        if isinstance(hist, ROOT.TH1D):
            hist.GetXaxis().SetRangeUser(-10, 10)
            break

    legend = canvas.BuildLegend(0.1, 0.65, 0.6, 0.9)
    legend.SetFillStyle(0)

    canvas.SaveAs(outputFileName)


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

    #testTree.SetLineColor(ROOT.kBlack)
    #testTree.Draw('Mbc', 'Mbc > 5.23', '')
    #testTree.SetLineStyle(ROOT.kDotted)
    #testTree.Draw('Mbc', '!isSignal', 'same')
    color = ROOT.kRed + 4
    first_plot = True
    for cut in [0.01, 0.1, 0.5]:
        testTree.SetLineColor(int(color))
        testTree.SetLineStyle(ROOT.kSolid)
        testTree.Draw('Mbc', 'Mbc > 5.23 && getExtraInfoSignalProbability > ' + str(cut), '' if first_plot else 'same')
        first_plot = False

        testTree.SetLineStyle(ROOT.kDotted)
        testTree.Draw('Mbc', 'Mbc > 5.23 && getExtraInfoSignalProbability > ' + str(cut) + ' && !isSignal', 'same')
        color -= 1

    l = canvas.GetListOfPrimitives()
    for i in range(l.GetEntries()):
        hist = l[i]
        if isinstance(hist, ROOT.TH1D):
            hist.GetXaxis().SetRangeUser(5.24, 5.29)
            break

    legend = canvas.BuildLegend(0.1, 0.65, 0.6, 0.9)
    legend.SetFillStyle(0)
    canvas.SaveAs(outputFileName)


def getMVARankingFromLogfile(logfile):
    """
    Extracts ranking from TMVA logfile and returns dictionary with variable:(rank, value)
    """
    ranking = {}
    ranking_mode = 0
    with open(logfile, 'r') as f:
        for line in f:
            if 'Variable Importance' in line:
                ranking_mode = 1
            elif ranking_mode == 1:
                ranking_mode = 2
            elif ranking_mode == 2 and '-------' in line:
                ranking_mode = 0
            elif ranking_mode == 2:
                v = line.split(':')
                ranking[v[2].strip()] = (int(v[1]), float(v[3]))
    return ranking


def sendMail():
    import smtplib
    from email.MIMEMultipart import MIMEMultipart
    from email.MIMEBase import MIMEBase
    from email.MIMEText import MIMEText
    from email.Utils import COMMASPACE, formatdate
    from email import Encoders
    import random

    fromMail = 'nordbert@kit.edu'
    toMails = ['t.keck@online.de', 'christian.pulvermacher@kit.edu']

    ClicheBegin = ['Hello', 'Congratulations!', 'Whats up?', 'Dear Sir/Madam', 'Howdy!']
    Person = ['I\'m', 'Nordbert is']
    StateOfMind = ['happy', 'cheerful', 'joyful', 'delighted', 'lucky']
    VerbAnnounce = ['announce', 'declare', 'make public', 'make known', 'report', 'publicize', 'broadcast', 'publish']
    PossesivePronoun = ['our', 'my', 'your']
    Adjective = ['awesome', 'revolutionary', 'gorgeous', 'beautiful', 'spectacular', 'splendid', 'superb', 'wonderful', 'impressive', 'amazing', 'stunning', 'breathtaking', 'incredible']
    VerbFinished = ('has', ['finished', 'completed', 'terminated', 'concluded'])
    Motivation = [('Keep on your', Adjective, 'work!'), (Person, 'so proud of you!'), (['Enjoy this', 'Relax a'], 'moment and have a', ['cookie', '\bn ice', 'bath', 'day off'], '\b.')]
    ClicheEnd = ['Best regards', 'Yours sincerely', 'Kind regards', 'Yours faithfully', 'See you', 'cu']

    def generate_sentence(term):
        if isinstance(term, str):
            return term
        if isinstance(term, list):
            return generate_sentence(term[random.randint(0, len(term) - 1)])
        if isinstance(term, tuple):
            return " ".join([generate_sentence(subterm) for subterm in term])
        raise RuntimeError('Invalid type received in sentence generator')

    sentence = (ClicheBegin, '\n\n', Person, StateOfMind, 'to', VerbAnnounce, 'that', PossesivePronoun, Adjective, 'Full Event Interpretation', VerbFinished,
                '\b.\n\nThat\'s', Adjective, '\b!\n', Motivation, '\n\n', ClicheEnd, '\b,\n', ['', ('the', [Adjective, StateOfMind])], 'Nordbert')

    text = MIMEText(re.sub('.\b', '', generate_sentence(sentence)))

    filename = 'FEIsummary.pdf'
    pdf = MIMEBase('application', "octet-stream")
    pdf.set_payload(open(filename, "rb").read())
    Encoders.encode_base64(pdf)
    pdf.add_header('Content-Disposition', 'attachment; filename="%s"' % os.path.basename(filename))

    msg = MIMEMultipart()
    msg['From'] = fromMail
    msg['To'] = COMMASPACE.join(toMails)
    msg['Date'] = formatdate(localtime=True)
    msg['Subject'] = 'Congratulations!'
    msg.attach(text)
    msg.attach(pdf)

    server = smtplib.SMTP('smtp.kit.edu')
    #server.login(fromMail, 'Not necessary for kit.edu :-)')
    server.sendmail(fromMail, toMails, msg.as_string())
    server.quit()


def getModuleStatsFromFile(filename):
    """
    Gets a vector of ModuleStatistics objects from given file.
    """

    tfile = ROOT.TFile(filename)
    persistentTree = tfile.Get('persistent')
    persistentTree.GetEntry(0)
    # Clone() needed so we actually own the object (original dies when tfile is deleted)
    stats = persistentTree.ProcessStatistics.Clone()

    # merge statistics from all persistent trees into 'stats'
    numEntries = persistentTree.GetEntriesFast()
    for i in range(1, numEntries):
        persistentTree.GetEntry(i)
        stats.merge(persistentTree.ProcessStatistics)

    return stats.getAll()


def createCPUTimeTexFile(channelNames, inputLists, channelPlaceholders, mcCounts, moduleStatisticsFile, stats):
    """
    Creates CPU time summary .tex file
        @param stats ProcessStatistics object to interpret
        @return placeholders
    """

    sum_time_seconds = 0
    sum_trueCandidates = 0
    sum_allCandidates = 0
    statTable = []
    moduleTypes = ('ParticleSelector', 'ParticleCombiner', 'ParticleVertexFitter', 'MCMatching', 'TMVAExpert', 'Other')
    for name, plist, currentPlaceholders in zip(channelNames, inputLists, channelPlaceholders):
        if plist is None:
            continue
        cpuPerModuleType = {}
        for mType in moduleTypes:
            cpuPerModuleType[mType] = 0.0

        matchingModules = [m for m in stats if plist in m.getName()]
        time_total_seconds = 0
        for m in matchingModules:
            moduleName = m.getName().split('_')[0]
            time_seconds = m.getTimeSum(m.c_Event) / 1e9
            if moduleName in cpuPerModuleType:
                cpuPerModuleType[moduleName] += time_seconds
            else:
                cpuPerModuleType['Other'] += time_seconds
            time_total_seconds += time_seconds

        for m in cpuPerModuleType:
            cpuPerModuleType[m] /= time_total_seconds
            cpuPerModuleType[m] *= 100

        shortName = prettifyDecayString(name)
        trueCandidates = currentPlaceholders['mvaNSignal']
        allCandidates = trueCandidates + currentPlaceholders['mvaNBackground']
        statTable.append([shortName, time_total_seconds, cpuPerModuleType, trueCandidates, allCandidates])
        sum_time_seconds += time_total_seconds
        sum_trueCandidates += trueCandidates
        sum_allCandidates += allCandidates

    # fill cpuTimeStatistics placeholder
    placeholders = {}
    colours = ["orange", "blue", "red", "green", "cyan", "purple"]
    placeholders['colourList'] = ', '.join('"%s"' % (c) for c in colours)
    placeholders['numColoursMinusOne'] = len(colours) - 1
    placeholders['barLegend'] = '' + ', '.join('\\textcolor{%s}{%s}' % (c, m) for c, m in zip(colours, moduleTypes))
    placeholders['cpuTimeStatistics'] = ''
    rowString = '{name} & {time} & {bargraph} & {timePerCandidate} & {timePercent:.2f}\\% \\\\\n'
    for row in statTable:
        cpuPerModuleType = row[2]
        bargraph = '\plotbar{ %g/, %g/, %g/, %g/, %g/, %g/, }' % tuple(cpuPerModuleType[key] for key in moduleTypes)
        timePerCandidate = formatTime(row[1] * 1.0 / row[4]) + ' (' + formatTime(row[1] * 1.0 / row[3]) + ')'
        timePercent = row[1] / sum_time_seconds * 100
        placeholders['cpuTimeStatistics'] += rowString.format(name=row[0], bargraph=bargraph, time=formatTime(row[1]), timePerCandidate=timePerCandidate, timePercent=timePercent)
    placeholders['cpuTimeStatistics'] += '\\bottomrule\n'
    placeholders['cpuTimeStatistics'] += rowString.format(name='Total', bargraph='', time=formatTime(sum_time_seconds), timePerCandidate='', timePercent=100)

    placeholders['texFile'] = 'CPUTimeSummary_' + moduleStatisticsFile + '.tex'
    if not os.path.isfile(placeholders['texFile']):
        createTexFile(placeholders['texFile'], 'analysis/scripts/FEI/templates/CPUTimeTemplate.tex', placeholders)

    return placeholders
