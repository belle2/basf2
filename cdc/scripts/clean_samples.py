#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
from reconstruction import prepare_cdst_analysis
import modularAnalysis as mA


def make_electron_collection(path_electron):
    prepare_cdst_analysis(path=path_electron)
    trg_bhabhaskim = path_electron.add_module("TriggerSkim", triggerLines=["software_trigger_cut&skim&accept_radee"])
    trg_bhabhaskim.if_value("==0", basf2.Path(), basf2.AfterConditionPath.END)

    path_electron.add_module(
        'CDCDedxCorrection',
        relativeCorrections=False,
        scaleCor=True,
        runGain=True,
        timeGain=True,
        cosineCor=True,
        wireGain=True,
        twoDCell=True,
        oneDCell=True)

    mA.fillParticleList('e+:calib', ' ', path=path_electron)
    return ['e+:calib']


# radiative muon selection
def make_muon_collection(path_muon):

    prepare_cdst_analysis(path=path_muon)

    path_muon.add_module(
        'CDCDedxCorrection',
        relativeCorrections=False,
        scaleCor=True,
        runGain=True,
        timeGain=True,
        cosineCor=True,
        wireGain=True,
        twoDCell=True,
        oneDCell=True)
    # Creating a particle list with basic cuts

    goodTrack = 'abs(dr) < 0.5 and abs(dz) < 0.5 and nCDCHits > 0'
    goodTrack += ' and clusterE < 0.40 and clusterEoP < 0.40 and inCDCAcceptance==1'
    goodTrack += ' and [[isPIDAvailableFrom(KLM) == 0 and clusterE < 0.25] or isPIDAvailableFrom(KLM) == 1]'

    mA.fillParticleList('mu+:calib', goodTrack, path=path_muon)

    track_cuts = ''
    track_cuts += 'formula(daughter(0,charge) * daughter(1,charge)) < 0'
    track_cuts += ' and [daughter(0,clusterE) <= 0.25 or daughter(1,clusterE) <= 0.25]'

    # one of them is valid in KLM
    track_cuts += ' and [daughter(0,isPIDAvailableFrom(KLM)) == 1 or daughter(1,isPIDAvailableFrom(KLM))  == 1]'

    mA.reconstructDecay('vpho:mumu -> mu+:calib mu-:calib', track_cuts, path=path_muon)
    event_cuts = '[nCleanedTracks('+goodTrack+') == 2] and nTracks < 3'

    mA.applyCuts('vpho:mumu', event_cuts, path=path_muon)
    radmumulist = ['vpho:mumu']

    skimfilter = basf2.register_module('SkimFilter')
    skimfilter.param('particleLists', radmumulist)
    path_muon.add_module(skimfilter)

    filter_path = basf2.create_path()
    skimfilter.if_value('=1', filter_path, basf2.AfterConditionPath.CONTINUE)

    # ------------------------Widget File
    cutonPairs = ''
    mA.cutAndCopyList('mu+:cal', 'mu+:calib', cutonPairs, path=filter_path)
    event_cutscopy = '[nCleanedTracks('+goodTrack+') == 2] and nTracks < 3'
    mA.applyCuts('mu+:cal', event_cutscopy, path=filter_path)

    return ['mu+:cal']


# proton selection
def make_proton_collection(path_hadrons):
    trg_skim = path_hadrons.add_module("TriggerSkim", triggerLines=["software_trigger_cut&skim&accept_lambda"])
    trg_skim.if_value("==0", basf2.Path(), basf2.AfterConditionPath.END)

    prepare_cdst_analysis(path=path_hadrons)
    path_hadrons.add_module(
        'CDCDedxCorrection',
        relativeCorrections=False,
        scaleCor=True,
        runGain=True,
        timeGain=True,
        cosineCor=True,
        wireGain=True,
        twoDCell=True,
        oneDCell=True)

    goodProtonTrack = 'dr< 0.50 and abs(dz)<0.50'
    mA.fillParticleList("p+:calib", goodProtonTrack, path=path_hadrons)
    return "p+:calib"


# pion kaon selection
def make_pion_kaon_collection(path_hadrons):

    trg_skim = path_hadrons.add_module(
        "TriggerSkim",
        triggerLines=[
            "software_trigger_cut&skim&accept_dstar_1",
            "software_trigger_cut&skim&accept_dstar_3"])
    trg_skim.if_value("==0", basf2.Path(), basf2.AfterConditionPath.END)

    prepare_cdst_analysis(path=path_hadrons)

    path_hadrons.add_module(
        'CDCDedxCorrection',
        relativeCorrections=False,
        scaleCor=True,
        runGain=True,
        timeGain=True,
        cosineCor=True,
        wireGain=True,
        twoDCell=True,
        oneDCell=True)

    clean_K = 'dr < 1.00 and abs(dz) < 1.0 and inCDCAcceptance==1 '
    clean_K += ' and [[cosTheta<-0.45 or cosTheta >= 0.80] or [[cosTheta>=-0.45 and cosTheta <= 0.80 and pt <= 0.50]'
    clean_K += ' or [cosTheta>=-0.45 and cosTheta <= 0.80 and pt > 0.50 and pidProbabilityExpert(321, TOP) > 0.015 ]]]'
    mA.fillParticleList("K+:calib", clean_K, path=path_hadrons)

    clean_Pi = 'dr < 1.0 and abs(dz) < 1.0 and inCDCAcceptance==1 '
    clean_Pi += ' and [[cosTheta<-0.45 or cosTheta > 0.80] or [[cosTheta>=-0.45 and cosTheta <= 0.80 and pt <= 0.50] '
    clean_Pi += ' or [cosTheta>=-0.45 and cosTheta <= 0.80 and pt > 0.50 and pidProbabilityExpert(321, TOP) < 0.60 ]]]'
    mA.fillParticleList("pi+:calib", clean_Pi, path=path_hadrons)

    # -----------------------------------------------------------
    clean_Dz1 = 'abs(formula(M-1.864)) < 0.020'
    clean_Dz1 += ' and [daughter(0,nCDCHits) > 30 or daughter(1, nCDCHits) > 30] '
    mA.reconstructDecay('D0:cal1 -> K-:calib pi+:calib', clean_Dz1, path=path_hadrons)

    clean_Dz2 = 'abs(formula(M-1.864)) < 0.020'
    clean_Dz2 += ' and [daughter(0,nCDCHits) > 25 or daughter(1, nCDCHits) > 25 or daughter(2, nCDCHits) > 25 '
    clean_Dz2 += ' or daughter(3, nCDCHits) > 25] '
    mA.reconstructDecay('D0:cal2 -> K-:calib pi+:calib pi+:calib pi-:calib', clean_Dz2, path=path_hadrons)

    # ------------------------------------------------------------
    clean_sPi = 'dr < 1.00 and abs(dz) < 1.00 and p < 1.5 and nCDCHits > 10 and inCDCAcceptance==1'
    mA.fillParticleList("pi+:slow", clean_sPi, path=path_hadrons)

    # ------------------------------------------------------------
    # tights are 0.0017 and 0.015
    clean_Ds = 'useCMSFrame(p) > 1.5 and abs(formula(massDifference(0) - 0.14542)) < 0.00070 '
    clean_Ds += ' and abs(formula(M - 2.0102)) < 0.020'
    mA.reconstructDecay('D*+:cal1 -> D0:cal1 pi+:slow', clean_Ds, path=path_hadrons)
    list_Ds1 = ['D*+:cal1']
    skimf_Ds1 = basf2.register_module('SkimFilter')
    skimf_Ds1.param('particleLists', list_Ds1)
    path_hadrons.add_module(skimf_Ds1)
    fpath_Ds1 = basf2.create_path()
    skimf_Ds1.if_value('=1', fpath_Ds1, basf2.AfterConditionPath.CONTINUE)

    mA.reconstructDecay('D*+:cal2 -> D0:cal2 pi+:slow', clean_Ds, path=path_hadrons)

    list_Ds2 = ['D*+:cal2']
    skimf_Ds2 = basf2.register_module('SkimFilter')
    skimf_Ds2.param('particleLists', list_Ds2)
    path_hadrons.add_module(skimf_Ds2)
    fpath_Ds2 = basf2.create_path()
    skimf_Ds2.if_value('=1', fpath_Ds2, basf2.AfterConditionPath.CONTINUE)

    # ------------------------------------------------------------

    mA.variablesToDaughterExtraInfo('D*+:cal1', 'D*+ -> [D0 -> ^K- ^pi+ ] ^pi+', {'M': 'l_Ds1M'}, path=fpath_Ds1)
    mA.variablesToDaughterExtraInfo('D*+:cal1', 'D*+ -> [D0 -> ^K- ^pi+ ] ^pi+', {'useCMSFrame(p)': 'l_Ds1p'}, path=fpath_Ds1)
    mA.variablesToDaughterExtraInfo('D*+:cal1', 'D*+ -> [D0 -> ^K- ^pi+ ] ^pi+',
                                    {'massDifference(0)': 'l_Ds1mDiff'}, path=fpath_Ds1)
    mA.variablesToDaughterExtraInfo('D*+:cal1', 'D*+ -> [D0 -> ^K- ^pi+ ] ^pi+', {'daughter(0,M)': 'l_Ds1_DzM'}, path=fpath_Ds1)
    mA.variablesToDaughterExtraInfo('D*+:cal1',
                                    'D*+ -> [D0 -> ^K- ^pi+ ] ^pi+',
                                    {'daughter(0,daughter(0,nCDCHits))': 'l_Ds1_Dz_Khits'},
                                    path=fpath_Ds1)
    mA.variablesToDaughterExtraInfo('D*+:cal1',
                                    'D*+ -> [D0 -> ^K- ^pi+ ] ^pi+',
                                    {'daughter(0,daughter(1,nCDCHits))': 'l_Ds1_Dz_Pihits'},
                                    path=fpath_Ds1)
    cutonD1var = "extraInfo(l_Ds1M) > 1.99 and extraInfo(l_Ds1M) < 2.03"
    cutonD1var += " and extraInfo(l_Ds1p) > 1.5 "
    cutonD1var += " and extraInfo(l_Ds1mDiff) > 0.14442 and extraInfo(l_Ds1mDiff) < 0.14612"
    cutonD1var += " and extraInfo(l_Ds1_DzM) > 1.844 and extraInfo(l_Ds1_DzM) < 1.884"
    cutonD1var += " and [extraInfo(l_Ds1_Dz_Khits) > 30 or extraInfo(l_Ds1_Dz_Pihits) > 30]"

    mA.cutAndCopyList('K+:dst1',  'K+:calib',  cutonD1var, path=fpath_Ds1)
    mA.cutAndCopyList('pi+:dst1', 'pi+:calib', cutonD1var, path=fpath_Ds1)

    mA.variablesToDaughterExtraInfo('D*+:cal2', 'D*+ -> [D0 -> ^K- ^pi+ ^pi+ ^pi-] ^pi+', {'M': 'l_Ds2M'}, path=fpath_Ds2)
    mA.variablesToDaughterExtraInfo('D*+:cal2',
                                    'D*+ -> [D0 -> ^K- ^pi+ ^pi+ ^pi-] ^pi+',
                                    {'useCMSFrame(p)': 'l_Ds2p'},
                                    path=fpath_Ds2)
    mA.variablesToDaughterExtraInfo('D*+:cal2',
                                    'D*+ -> [D0 -> ^K- ^pi+ ^pi+ ^pi-] ^pi+',
                                    {'massDifference(0)': 'l_Ds2mDiff'},
                                    path=fpath_Ds2)
    mA.variablesToDaughterExtraInfo('D*+:cal2',
                                    'D*+ -> [D0 -> ^K- ^pi+ ^pi+ ^pi-] ^pi+',
                                    {'daughter(0,M)': 'l_Ds2_DzM'},
                                    path=fpath_Ds2)
    mA.variablesToDaughterExtraInfo('D*+:cal2',
                                    'D*+ -> [D0 -> ^K- ^pi+ ^pi+ ^pi-] ^pi+',
                                    {'daughter(0,daughter(0,nCDCHits))': 'l_Ds2_Dz_Khits'},
                                    path=fpath_Ds2)
    mA.variablesToDaughterExtraInfo('D*+:cal2',
                                    'D*+ -> [D0 -> ^K- ^pi+ ^pi+ ^pi-] ^pi+',
                                    {'daughter(0,daughter(1,nCDCHits))': 'l_Ds2_Dz_Pi1hits'},
                                    path=fpath_Ds2)
    mA.variablesToDaughterExtraInfo('D*+:cal2',
                                    'D*+ -> [D0 -> ^K- ^pi+ ^pi+ ^pi-] ^pi+',
                                    {'daughter(0,daughter(2,nCDCHits))': 'l_Ds2_Dz_Pi2hits'},
                                    path=fpath_Ds2)
    mA.variablesToDaughterExtraInfo('D*+:cal2',
                                    'D*+ -> [D0 -> ^K- ^pi+ ^pi+ ^pi-] ^pi+',
                                    {'daughter(0,daughter(3,nCDCHits))': 'l_Ds2_Dz_Pi3hits'},
                                    path=fpath_Ds2)

    cutonD2var = "extraInfo(l_Ds2M) > 1.99 and extraInfo(l_Ds2M) < 2.03"
    cutonD2var += " and extraInfo(l_Ds2p) > 2.0"
    cutonD2var += " and extraInfo(l_Ds2mDiff) > 0.14442 and extraInfo(l_Ds2mDiff) < 0.14612"
    cutonD2var += " and extraInfo(l_Ds2_DzM) > 1.844 and extraInfo(l_Ds2_DzM) < 1.884"
    cutonD2var += " and [extraInfo(l_Ds2_Dz_Khits) > 25 or extraInfo(l_Ds2_Dz_Pi1hits) > 25 "
    cutonD2var += " or extraInfo(l_Ds2_Dz_Pi2hits) > 25 or extraInfo(l_Ds2_Dz_Pi3hits) > 25]"

    mA.cutAndCopyList('K+:dst2',  'K+:calib',  cutonD2var, path=fpath_Ds2)
    mA.cutAndCopyList('pi+:dst2', 'pi+:calib', cutonD2var, path=fpath_Ds2)

    pion_kaon_list = ['pi+:dst1', 'pi+:dst2', 'K+:dst1', 'K+:dst2']

    return pion_kaon_list
