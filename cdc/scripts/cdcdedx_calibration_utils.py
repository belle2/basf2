##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
from reconstruction import prepare_cdst_analysis
import modularAnalysis as mA


# radiative electron selection
def make_electron_collection(path_electron):
    prepare_cdst_analysis(path=path_electron)

    # Add a TriggerSkim module to apply a specific trigger line
    trg_bhabhaskim = path_electron.add_module("TriggerSkim", triggerLines=["software_trigger_cut&skim&accept_radee"])
    trg_bhabhaskim.if_value("==0", basf2.Path(), basf2.AfterConditionPath.END)

    # Add the CDCDedxCorrection module to correct dE/dx (no saturation)
    path_electron.add_module(
        'CDCDedxCorrection',
        relativeCorrections=False,  # Disable relative corrections
        scaleCor=True,
        runGain=True,   # Enable run gain corrections
        timeGain=True,  # Enable injection time gain corrections
        cosineCor=True,  # Enable cosine corrections
        wireGain=True,  # Enable wire gain corrections
        twoDCell=True,  # Enable 2D cell corrections
        oneDCell=True)  # Enable 1D cell corrections

    # Fill the particle list
    mA.fillParticleList('e+:calib', ' ', path=path_electron)
    return ['e+:calib']

# ------------------------------------------------------------------------------------------------------------------

# radiative muon selection


def make_muon_collection(path_muon):

    prepare_cdst_analysis(path=path_muon)

    # Add the CDCDedxCorrection module to correct dE/dx (no saturation)
    path_muon.add_module(
        'CDCDedxCorrection',
        relativeCorrections=False,  # Disable relative corrections
        scaleCor=True,
        runGain=True,   # Enable run gain corrections
        timeGain=True,  # Enable injection time gain corrections
        cosineCor=True,  # Enable cosine corrections
        wireGain=True,  # Enable wire gain corrections
        twoDCell=True,  # Enable 2D cell corrections
        oneDCell=True)  # Enable 1D cell corrections

    # Creating a particle list with basic cuts
    goodTrack = 'abs(dr) < 0.5 and abs(dz) < 0.5 and nCDCHits > 0'
    goodTrack += ' and clusterE < 0.40 and clusterEoP < 0.40 and inCDCAcceptance==1'
    goodTrack += ' and [[isPIDAvailableFrom(KLM) == 0 and clusterE < 0.25] or isPIDAvailableFrom(KLM) == 1]'

    # Create a particle list for muons that satisfy the selection criteria
    mA.fillParticleList('mu+:calib', goodTrack, path=path_muon)

    # Define cuts for the track pair (mu+ and mu-)
    track_cuts = ''
    track_cuts += 'daughterProductOf(charge) < 0'
    track_cuts += ' and daughterLowest(clusterE) <= 0.25'

    # One of the muons is valid in KLM
    track_cuts += ' and [daughter(0,isPIDAvailableFrom(KLM)) == 1 or daughter(1,isPIDAvailableFrom(KLM))  == 1]'

    # Reconstruct the decay 'vpho -> mu+ mu-'
    mA.reconstructDecay('vpho:mumu -> mu+:calib mu-:calib', track_cuts, path=path_muon)

    # Apply event-level cuts to ensure there are exactly two cleaned tracks, and less than three total tracks
    event_cuts = '[nCleanedTracks('+goodTrack+') == 2]'
    mA.applyEventCuts(event_cuts, path=path_muon)
    radmumulist = ['vpho:mumu']

    # Apply SkimFilter
    skimfilter = basf2.register_module('SkimFilter')
    skimfilter.param('particleLists', radmumulist)
    path_muon.add_module(skimfilter)

    filter_path = basf2.create_path()
    skimfilter.if_value('=1', filter_path, basf2.AfterConditionPath.CONTINUE)

    # Define additional cuts for the muon pair
    cutonPairs = ''
    mA.cutAndCopyList('mu+:cal', 'mu+:calib', cutonPairs, path=filter_path)

    # Apply event-level cuts for the copied muons
    event_cutscopy = '[nCleanedTracks('+goodTrack+') == 2]'
    mA.applyEventCuts(event_cutscopy, path=filter_path)

    return ['mu+:cal']

# ------------------------------------------------------------------------------------------------------------------

# proton selection


def make_proton_collection(path_hadrons):

    # Trigger Skim Module to apply the specified trigger lines
    trg_skim = path_hadrons.add_module("TriggerSkim", triggerLines=["software_trigger_cut&skim&accept_lambda"])
    trg_skim.if_value("==0", basf2.Path(), basf2.AfterConditionPath.END)

    prepare_cdst_analysis(path=path_hadrons)

    # Add the CDCDedxCorrection module to correct dE/dx (no saturation)
    path_hadrons.add_module(
        'CDCDedxCorrection',
        relativeCorrections=False,  # Disable relative corrections
        scaleCor=True,
        runGain=True,   # Enable run gain corrections
        timeGain=True,  # Enable injection time gain corrections
        cosineCor=True,  # Enable cosine corrections
        wireGain=True,  # Enable wire gain corrections
        twoDCell=True,  # Enable 2D cell corrections
        oneDCell=True)  # Enable 1D cell corrections

    # Define a selection criteria for good proton tracks
    goodProtonTrack = 'dr< 0.50 and abs(dz)<0.50'
    mA.fillParticleList("p+:calib", goodProtonTrack, path=path_hadrons)
    return "p+:calib"

# ------------------------------------------------------------------------------------------------------------------

# pion kaon selection


def make_pion_kaon_collection(path_hadrons):

    # Add the TriggerSkim module to the path with specified trigger lines
    trg_skim = path_hadrons.add_module(
        "TriggerSkim",
        triggerLines=[
            "software_trigger_cut&skim&accept_dstar_1",
            "software_trigger_cut&skim&accept_dstar_3"])

    trg_skim.if_value("==0", basf2.Path(), basf2.AfterConditionPath.END)

    prepare_cdst_analysis(path=path_hadrons)

    # Add the CDCDedxCorrection module to correct dE/dx (no saturation)
    path_hadrons.add_module(
        'CDCDedxCorrection',
        relativeCorrections=False,  # Disable relative corrections
        scaleCor=True,
        runGain=True,   # Enable run gain corrections
        timeGain=True,  # Enable injection time gain corrections
        cosineCor=True,  # Enable cosine corrections
        wireGain=True,  # Enable wire gain corrections
        twoDCell=True,  # Enable 2D cell corrections
        oneDCell=True)  # Enable 1D cell corrections

    # Define cleaning criteria for the Kaon particle list
    clean_K = 'dr < 1.00 and abs(dz) < 1.0 and inCDCAcceptance==1 '
    clean_K += ' and [[cosTheta<-0.45 or cosTheta >= 0.80] or [[cosTheta>=-0.45 and cosTheta <= 0.80 and pt <= 0.50]'
    clean_K += ' or [cosTheta>=-0.45 and cosTheta <= 0.80 and pt > 0.50 and pidProbabilityExpert(321, TOP) > 0.015 ]]]'

    # Fill the cleaned Kaon list
    mA.fillParticleList("K+:calib", clean_K, path=path_hadrons)

    # Define cleaning criteria for the Pion particle list
    clean_Pi = 'dr < 1.0 and abs(dz) < 1.0 and inCDCAcceptance==1 '
    clean_Pi += ' and [[cosTheta<-0.45 or cosTheta > 0.80] or [[cosTheta>=-0.45 and cosTheta <= 0.80 and pt <= 0.50] '
    clean_Pi += ' or [cosTheta>=-0.45 and cosTheta <= 0.80 and pt > 0.50 and pidProbabilityExpert(321, TOP) < 0.60 ]]]'

    # Fill the cleaned Pion list
    mA.fillParticleList("pi+:calib", clean_Pi, path=path_hadrons)

    # -----------------------------------------------------------
    # Reconstruction for the D0 -> K- pi+ decay
    clean_Dz1 = 'abs(dM) < 0.020'    # Mass difference of D0
    clean_Dz1 += ' and [daughter(0,nCDCHits) > 30 or daughter(1, nCDCHits) > 30] '

    # clean_Dz1 += ' and daughterHighest(nCDCHits) > 30 '
    mA.reconstructDecay('D0:cal1 -> K-:calib pi+:calib', clean_Dz1, path=path_hadrons)

    # Reconstruction for the D0 -> K- pi+ pi+ pi- decay
    clean_Dz2 = 'abs(dM) < 0.020'   # Mass difference of D0
    clean_Dz2 += ' and [daughter(0,nCDCHits) > 25 or daughter(1, nCDCHits) > 25 or daughter(2, nCDCHits) > 25 '
    clean_Dz2 += ' or daughter(3, nCDCHits) > 25] '
    # clean_Dz2 += ' and daughterHighest(nCDCHits) > 25 '
    mA.reconstructDecay('D0:cal2 -> K-:calib pi+:calib pi+:calib pi-:calib', clean_Dz2, path=path_hadrons)

    # ------------------------------------------------------------
    # Define cleaning for slow pions
    clean_sPi = 'dr < 1.00 and abs(dz) < 1.00 and p < 1.5 and nCDCHits > 10 and inCDCAcceptance==1'
    mA.fillParticleList("pi+:slow", clean_sPi, path=path_hadrons)   # Fill the slow pion list

    # ------------------------------------------------------------
    # Reconstruct the first D*+ -> D0 [-> K- pi+] and slow pion decay
    # tights are 0.0017 and 0.015
    clean_Ds = 'useCMSFrame(p) > 1.5 and abs(formula(massDifference(0) - 0.14542)) < 0.00070 '
    clean_Ds += ' and abs(dM) < 0.020'  # Mass difference of D*
    mA.reconstructDecay('D*+:cal1 -> D0:cal1 pi+:slow', clean_Ds, path=path_hadrons)

    # Apply SkimFilter for the first D*+ decay
    list_Ds1 = ['D*+:cal1']
    skimf_Ds1 = basf2.register_module('SkimFilter')
    skimf_Ds1.param('particleLists', list_Ds1)
    path_hadrons.add_module(skimf_Ds1)
    fpath_Ds1 = basf2.create_path()
    skimf_Ds1.if_value('=1', fpath_Ds1, basf2.AfterConditionPath.CONTINUE)

    # Reconstruct the second D*+ [-> D0 [-> K- pi+ pi+ pi- ] slow pion] decay and apply SkimFilter
    mA.reconstructDecay('D*+:cal2 -> D0:cal2 pi+:slow', clean_Ds, path=path_hadrons)

    list_Ds2 = ['D*+:cal2']
    skimf_Ds2 = basf2.register_module('SkimFilter')
    skimf_Ds2.param('particleLists', list_Ds2)
    path_hadrons.add_module(skimf_Ds2)
    fpath_Ds2 = basf2.create_path()
    skimf_Ds2.if_value('=1', fpath_Ds2, basf2.AfterConditionPath.CONTINUE)

    # ------------------------------------------------------------
    # Add extra information to daughter particles for first D*+ decay
    mA.variablesToDaughterExtraInfo('D*+:cal1', 'D*+ -> [D0 -> ^K- ^pi+ ] ^pi+', {'dM': 'l_Ds1dM'}, path=fpath_Ds1)
    mA.variablesToDaughterExtraInfo('D*+:cal1', 'D*+ -> [D0 -> ^K- ^pi+ ] ^pi+', {'useCMSFrame(p)': 'l_Ds1p'}, path=fpath_Ds1)
    mA.variablesToDaughterExtraInfo('D*+:cal1', 'D*+ -> [D0 -> ^K- ^pi+ ] ^pi+',
                                    {'massDifference(0)': 'l_Ds1mDiff'}, path=fpath_Ds1)
    mA.variablesToDaughterExtraInfo('D*+:cal1', 'D*+ -> [D0 -> ^K- ^pi+ ] ^pi+', {'daughter(0,dM)': 'l_Ds1_DzdM'}, path=fpath_Ds1)
    mA.variablesToDaughterExtraInfo('D*+:cal1',
                                    'D*+ -> [D0 -> ^K- ^pi+ ] ^pi+',
                                    {'daughter(0,daughter(0,nCDCHits))': 'l_Ds1_Dz_Khits'},
                                    path=fpath_Ds1)
    mA.variablesToDaughterExtraInfo('D*+:cal1',
                                    'D*+ -> [D0 -> ^K- ^pi+ ] ^pi+',
                                    {'daughter(0,daughter(1,nCDCHits))': 'l_Ds1_Dz_Pihits'},
                                    path=fpath_Ds1)

    # Define the cut conditions for first D*+ mass difference, momentum, and decay parameters
    cutonD1var = "abs(extraInfo(l_Ds1dM)) < 0.02"  # D* mass difference
    cutonD1var += "and extraInfo(l_Ds1p) > 1.5 "  # D* momentum
    cutonD1var += " and abs(formula(extraInfo(l_Ds1mDiff) - 0.14542)) < 0.0010"  # \DeltaM
    cutonD1var += " and abs(extraInfo(l_Ds1_DzdM)) < 0.02"  # D0 mass difference
    cutonD1var += " and [extraInfo(l_Ds1_Dz_Khits) > 30 or extraInfo(l_Ds1_Dz_Pihits) > 30]"

    # Apply cuts and copy the filtered particles to new lists
    mA.cutAndCopyList('K+:dst1',  'K+:calib',  cutonD1var, path=fpath_Ds1)
    mA.cutAndCopyList('pi+:dst1', 'pi+:calib', cutonD1var, path=fpath_Ds1)

    # ------------------------------------------------------------
    # Add extra information to daughter particles for second D*+ decay
    mA.variablesToDaughterExtraInfo('D*+:cal2', 'D*+ -> [D0 -> ^K- ^pi+ ^pi+ ^pi-] ^pi+', {'dM': 'l_Ds2dM'}, path=fpath_Ds2)
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
                                    {'daughter(0,dM)': 'l_Ds2_DzdM'},
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

    # Define the cut conditions for second D*+ mass difference, momentum, and decay parameters
    cutonD2var = "abs(extraInfo(l_Ds2dM)) < 0.02"     # D* mass difference
    cutonD2var += "and extraInfo(l_Ds2p) > 2.0"    # D* momentum
    cutonD2var += " and abs(formula(extraInfo(l_Ds2mDiff) - 0.14542)) < 0.0010"  # \DeltaM
    cutonD2var += " and abs(extraInfo(l_Ds2_DzdM)) < 0.02"  # D0 mass difference
    cutonD2var += " and [extraInfo(l_Ds2_Dz_Khits) > 25 or extraInfo(l_Ds2_Dz_Pi1hits) > 25 "
    cutonD2var += " or extraInfo(l_Ds2_Dz_Pi2hits) > 25 or extraInfo(l_Ds2_Dz_Pi3hits) > 25]"

    # Apply cuts and copy the filtered particles to new lists
    mA.cutAndCopyList('K+:dst2',  'K+:calib',  cutonD2var, path=fpath_Ds2)
    mA.cutAndCopyList('pi+:dst2', 'pi+:calib', cutonD2var, path=fpath_Ds2)

    pion_kaon_list = ['pi+:dst1', 'pi+:dst2', 'K+:dst1', 'K+:dst2']

    return pion_kaon_list
