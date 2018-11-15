#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" Skim list building functions for systematics studies """

__authors__ = [
    "Sam Cunliffe",
    "Torben Ferber",
    "Ilya Komarov",
    "Yuji Kato"
]

from basf2 import *
from modularAnalysis import *


def SystematicsRadEEList(path, prescale_all=1, prescale_fwd_electron=1):
    """
    Note:
        * Systematics skim(s) for radiative electron pairs
        * Skim code: 10600700
        * Physics channel: ee → ee(γ)
        * Skim category: systematics (photon calibration)

    Build the list of radiative electron pairs for photon systematics. In
    particular this is for the endcaps where we have no track triggers, we
    require one cluster-matched electron (the other is not required to match
    a cluster). No selection on the photon as the sample must be unbiased.

    As this retains a lot of bhabha events (by construction) we allow for
    prescaling (and prefer prescaled rather than a biased sampe by requiring
    any selection on the photon or too much of a cut on the recoil momentum)

    Prescales are given in standard trigger terms (reciprocal), so prescale of
    100 is 1% of events kept, etc.

    Parameters:
        path (basf2.Path): skim lists are added to this path
        prescale_all (int): the global prescale for this skim
        prescale_fwd_electron (int): the prescale electrons (e-) in
                                     the forward endcap

    Returns:
        list name of the skim candidates
    """
    __author__ = "Sam Cunliffe"

    # convert prescales from trigger convention
    prescale_all = str(float(1.0 / prescale_all))
    prescale_fwd_electron = str(float(1.0 / prescale_fwd_electron))

    # require a pair of good electrons one of which must be cluster-matched
    # with 3 GeV of energy
    goodtrack = 'abs(dz) < 2.0 and abs(dr) < 0.5 and nCDCHits > 0'
    goodtrackwithcluster = '%s and clusterE > 3.0' % goodtrack
    cutAndCopyList('e+:skimtight', 'e+:all', goodtrackwithcluster, path=path)
    cutAndCopyList('e+:skimloose', 'e+:all', goodtrack, path=path)

    # a minimum momentum of 75 MeV/c recoiling against the pair,
    # and require that the recoil is within the CDC acceptance
    recoil = 'pRecoil > 0.075 and 0.296706 < pRecoilTheta < 2.61799'  # GeV/c, rad
    reconstructDecay('vpho:radee -> e+:skimtight e-:skimloose', recoil, path=path)

    # apply event cuts (exactly two clean tracks in the event, and prescale
    # the whole event regardless of where the electron went)
    event_cuts = '[nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 2]'  # cm, cm
    event_cuts += ' and [eventRandom <= %s]' % prescale_all

    # now prescale the *electron* (e-) in the forward endcap (for bhabhas)
    # note this is all done with cut strings to circumnavigate BII-3607
    fwd_encap_border = '0.5480334'  # rad (31.4 deg)
    electron_is_first = 'daughter(0, charge) < 0'
    first_in_fwd_endcap = 'daughter(0, theta) < %s' % fwd_encap_border
    first_not_in_fwd_endcap = 'daughter(0, theta) > %s' % fwd_encap_border
    electron_is_second = 'daughter(1, charge) < 0'
    second_in_fwd_endcap = 'daughter(1, theta) < %s' % fwd_encap_border
    second_not_in_fwd_endcap = 'daughter(1, theta) > %s' % fwd_encap_border
    passes_prescale = 'eventRandom <= %s' % prescale_fwd_electron
    #
    # four possible scenarios:
    # 1) electron first in the decaystring and in fwd endcap: prescale these
    prescale_logic = '[%s and %s and %s]' \
        % (electron_is_first, first_in_fwd_endcap, passes_prescale)
    # 2) electron second in string and in fwd endcap: prescale these
    prescale_logic += ' or [%s and %s and %s]' \
        % (electron_is_second, second_in_fwd_endcap, passes_prescale)
    # 3) electron first in string and not in fwd endcap (no prescale)
    prescale_logic += ' or [%s and %s]' % (electron_is_first, first_not_in_fwd_endcap)
    # 4) electron second in string and not in fwd endcap (no prescale)
    prescale_logic += ' or [%s and %s]' % (electron_is_second, second_not_in_fwd_endcap)

    # final candidate building with cuts and prescales
    prescale_logic = '[%s]' % prescale_logic
    applyCuts('vpho:radee', event_cuts + ' and ' + prescale_logic, path=path)
    return ['vpho:radee']


def SystematicsRadMuMuList(path):
    """
    Note:
        * Systematics skim(s) for radiative muon pairs
        * Skim code: 10600500
        * Physics channel: ee → mumu(γ)
        * Skim category: systematics (photon calibration)

    Build the list of radiative muon pairs for photon systematics.
    We require one cluster-matched electron (the other is not required to match
    a cluster). No selection on the photon as the sample must be unbiased.

    Parameters:
        path (basf2.Path): skim lists are added to this path

    Returns:
        list name of the skim candidates
    """
    __author__ = "Torben Ferber"

    # the tight selection starts with all muons, but they  must be cluster-matched and not be an electron
    MuonTightSelection = 'abs(dz) < 2.0 and abs(dr) < 0.5 and nCDCHits > 0 and clusterE > 0.0 and clusterE < 1.0'
    cutAndCopyList('mu+:skimtight', 'mu+:all', MuonTightSelection, path=path)

    # for the loose selection starts with all muons, but we accept tracks that
    # are not matched to a cluster, but if they are, they must not be an
    # electron
    MuonLooseSelection = 'abs(dz) < 2.0 and abs(dr) < 0.5 and nCDCHits > 0 and clusterE < 1.0'
    cutAndCopyList('mu+:skimloose', 'mu+:all', MuonLooseSelection, path=path)

    # create a list of possible selections
    radmumulist = []

    # selection ID0:
    # the radiative muon pair must be selected without looking at the photon. exclude events with more than two good tracks
    RadMuMuSelection = 'pRecoil > 0.075 and pRecoilTheta > 0.296706 and pRecoilTheta < 2.61799'
    RadMuMuPairChannel = 'mu+:skimtight mu-:skimloose'
    chID = 0
    reconstructDecay('vpho:radmumu' + str(chID) + ' -> ' + RadMuMuPairChannel, RadMuMuSelection, chID, path=path)
    eventCuts = 'nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 2'
    applyCuts('vpho:radmumu' + str(chID), eventCuts, path=path)
    radmumulist.append('vpho:radmumu' + str(chID))

    # selection Id1:
    # todo: include pair conversions?

    return radmumulist


def EELLList(path):
    """
    Note:
        * Systematics skim list for eell events
        * Skim LFN code:   10600600
        * Physics channel: ee → eell
        * Skim category: systematics (lepton ID)

    Returns:
        list name of the skim candidates
    """
    __author__ = "Ilya Komarov"

    # At skim level we avoid any PID-like requirements and just select events
    # with two good tracks coming from the interavtion region.
    eLooseSelection = 'abs(dz) < 2.0 and abs(dr) < 0.5'
    cutAndCopyList('e+:skimloose', 'e+:all', eLooseSelection, path=path)

    # create a list of possible selections
    eelllist = []

    # Lepon pair has low invariant mass and tracks are back-to-back-like
    EELLSelection = 'M < 4 and useCMSFrame(daughterAngle(0,1)) < 0.75'
    eventCuts = 'nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 2'
    reconstructDecay('gamma:eell -> e+:skimloose e-:skimloose', EELLSelection + " and " + eventCuts, path=path)
    eelllist.append('gamma:eell')

    return eelllist


def ResonanceList(path):
    DsList = getDsList(path)
    DstarList = getDstarList(path)
    SigmacList = getSigmacList(path)
    MuMugList = getmumugList(path)
    BZeroList = getBZeroList(path)
    BPlusList = getBPlusList(path)
    return (DsList + DstarList + SigmacList + MuMugList + BZeroList + BPlusList)


def getDsList(path):
    DsCuts = '1.90 < M < 2.04'

    reconstructDecay('phi:res -> K+:loose K-:loose', '1.01 < M < 1.03', path=path)
    reconstructDecay('K*0:res -> K+:loose pi-:loose', '0.7 < M < 1.1', path=path)

    DsChannel = ['phi:res pi+:loose']
    DsList = []
    for chID, channel in enumerate(DsChannel):
        particlename = 'D_s+:Resonance%d' % (chID)
        reconstructDecay(particlename + ' -> ' + channel, DsCuts, chID, path=path)
        matchMCTruth(particlename, path)
        DsList.append(particlename)

    return DsList


def getDstarList(path):
    DplusCuts = '1.8 < M < 1.93'
    DstarCuts = 'massDifference(0)<0.16 and useCMSFrame(p)>2.0'

    DplusChannel = ['K-:loose pi+:loose pi+:loose']

    DplusList = []
    for chID, channel in enumerate(DplusChannel):
        reconstructDecay('D+:resonance' + str(chID) + ' -> ' + channel, DplusCuts, chID, path=path)
        vertexRave('D+:resonance' + str(chID), 0.0, path=path)
        DplusList.append('D+:resonance' + str(chID))

    DstarChannel = []
    for channel in DplusList:
        DstarChannel.append(channel + ' pi0:loose')

    DstarList = []
    for chID, channel in enumerate(DstarChannel):
        reconstructDecay('D*+:resonance' + str(chID) + ' -> ' + channel, DstarCuts, chID, path=path)
        DstarList.append('D*+:resonance' + str(chID))
        matchMCTruth('D*+:resonance0', path=path)

    return DstarList


def getSigmacList(path):
    LambdacCuts = '2.24 < M < 2.33'
    SigmacCuts = 'massDifference(0)<0.28 and useCMSFrame(p) > 2.5'

    LambdacChannel = ['p+:loose K-:loose pi+:loose']
    LambdacList = []
    for chID, channel in enumerate(LambdacChannel):
        reconstructDecay('Lambda_c+:resonance' + str(chID) + ' -> ' + channel, LambdacCuts, chID, path=path)
        vertexRave('Lambda_c+:resonance' + str(chID), 0.0, path=path)
        LambdacList.append('Lambda_c+:resonance' + str(chID))

    SigmacList = []
    SigmacPlusChannel = []
# Sigma_c++
    for channel in LambdacList:
        SigmacPlusChannel.append(channel + ' pi+:loose')

    for chID, channel in enumerate(SigmacPlusChannel):
        reconstructDecay('Sigma_c++:resonance' + str(chID) + ' -> ' + channel, SigmacCuts, chID, path=path)
        SigmacList.append('Sigma_c++:resonance' + str(chID))
        matchMCTruth('Sigma_c++:resonance0', path=path)

# Sigma_c0
    Sigmac0Channel = []
    for channel in LambdacList:
        Sigmac0Channel.append(channel + ' pi-:loose')

    Sigmac0List = []
    for chID, channel in enumerate(Sigmac0Channel):
        reconstructDecay('Sigma_c0:resonance' + str(chID) + ' -> ' + channel, SigmacCuts, chID, path=path)
        SigmacList.append('Sigma_c0:resonance' + str(chID))
        matchMCTruth('Sigma_c0:resonance0', path=path)

    return SigmacList


def getmumugList(path):

    vphoChannel = ['mu+:loose mu-:loose']
    vphocuts = ''
    vphoList = []
    for chID, channel in enumerate(vphoChannel):
        resonanceName = 'vpho:resonance' + str(chID)
        reconstructDecay('vpho:resonance' + str(chID) + ' -> ' + channel, vphocuts, chID, path=path)
        applyCuts(resonanceName, 'nTracks == 2 and M < formula(Ecms*0.9877)', path=path)
        matchMCTruth(resonanceName, path=path)
        vertexRave(resonanceName, 0.0, path=path)
        applyCuts(resonanceName, 'M < formula(Ecms*0.9877)', path=path)
        vphoList.append(resonanceName)

    return vphoList


def getBPlusList(path):
    antiDZeroCut = '1.82 < M < 1.90'
    antiDZeroChannel = ['K+:loose pi-:loose']
    antiDZeroList = []

    for chID, channel in enumerate(antiDZeroChannel):
        resonanceName = 'anti-D0:resonance' + str(chID)
        reconstructDecay(resonanceName + ' -> ' + channel, antiDZeroCut, chID, path=path)
        vertexRave(resonanceName, 0.0, path=path)
        antiDZeroList.append(resonanceName)

    BPlusChannel = []
    for channel in antiDZeroList:
        BPlusChannel.append(channel + ' pi+:loose')

    BPlusCuts = 'Mbc > 5.2 and abs(deltaE) < 0.3'
    BPlusList = []
    for chID, channel in enumerate(BPlusChannel):
        reconstructDecay('B+:resonance' + str(chID) + ' -> ' + channel, BPlusCuts, chID, path=path)
        BPlusList.append('B+:resonance' + str(chID))
        matchMCTruth('B+:resonance' + str(chID), path=path)

    return BPlusList


def getBZeroList(path):
    BZeroCuts = 'Mbc > 5.2 and abs(deltaE) < 0.3'
    BZeroChannel = ['D-:resonance0 pi+:loose']
    BZeroList = []

    for chID, channel in enumerate(BZeroChannel):
        resonanceName = 'B0:resonance' + str(chID)
        reconstructDecay(resonanceName + ' -> ' + channel, BZeroCuts, chID, path=path)
        BZeroList.append(resonanceName)
        matchMCTruth(resonanceName, path=path)

    return BZeroList


def SystematicsLambdaList(path):
    LambdaCuts = 'M < 1.2'

    fillParticleList('p+:all', '', enforceFitHypothesis=True, path=path)
    fillParticleList('pi-:all', '', enforceFitHypothesis=True, path=path)
    LambdaChannel = ['p+:all pi-:loose']

    LambdaList = []
    for chID, channel in enumerate(LambdaChannel):
        reconstructDecay('Lambda0:syst' + str(chID) + ' -> ' + channel, LambdaCuts, chID, path=path)
        vertexKFit('Lambda0:syst' + str(chID), 0.002, path=path)
        applyCuts('Lambda0:syst' + str(chID), '1.10<M<1.13', path=path)
        applyCuts('Lambda0:syst' + str(chID), 'formula(x*x+y*y)>0.0225', path=path)
        applyCuts('Lambda0:syst' + str(chID), 'formula(x*px+y*py)>0', path=path)
        applyCuts(
            'Lambda0:syst' +
            str(chID),
            'formula([x*px*x*px+2*x*px*y*py+y*py*y*py]/[[px*px+py*py]*[x*x+y*y]])>0.994009',
            path=path)
        applyCuts('Lambda0:syst' + str(chID), 'p>0.2', path=path)
        matchMCTruth('Lambda0:syst0', path=path)
        LambdaList.append('Lambda0:syst' + str(chID))

    return LambdaList


def SystematicsList(path):

    Lists = []
    Lists += JpsimumuTagProbe(path)
    Lists += JpsieeTagProbe(path)
    Lists += PiKFromDstarList(path)
    return Lists


def PiKFromDstarList(path):
    D0Cuts = '1.81 < M < 1.91'
#   DstarCuts = 'massDifference(0)<0.16'
    DstarCuts = 'massDifference(0)<0.16 and useCMSFrame(p) > 1.5'

    D0Channel = ['K-:all pi+:all'
                 ]

    D0List = []
    for chID, channel in enumerate(D0Channel):
        reconstructDecay('D0:syst' + str(chID) + ' -> ' + channel, D0Cuts, chID, path=path)
        vertexRave('D0:syst' + str(chID), 0.0, path=path)
        D0List.append('D0:syst' + str(chID))

    DstarChannel = []
    for channel in D0List:
        DstarChannel.append(channel + ' pi+:all')

    DstarList = []
    for chID, channel in enumerate(DstarChannel):
        reconstructDecay('D*-:syst' + str(chID) + ' -> ' + channel, DstarCuts, chID, path=path)
        DstarList.append('D*-:syst' + str(chID))
        matchMCTruth('D*+:syst0', path=path)

    return DstarList


def JpsimumuTagProbe(path):
    #   Cuts = '2.8 < M < 3.4'
    Cuts = '2.7 < M < 3.4 and useCMSFrame(p) < 2.0'
    Channel = 'mu+:all mu-:loose'
    jpsiList = []
    chID = 0
    reconstructDecay('J/psi:mumutagprobe' + str(chID) + ' -> ' + Channel, Cuts, chID, path=path)
    jpsiList.append('J/psi:mumutagprobe' + str(chID))
    matchMCTruth('J/psi:mumutagprobe0', path=path)
    return jpsiList


def JpsieeTagProbe(path):
    #   Cuts = '2.7 < M < 3.4'
    Cuts = '2.7 < M < 3.4 and useCMSFrame(p) < 2.0'
    Channel = 'e+:all e-:loose'
    jpsiList = []
    chID = 0
    reconstructDecay('J/psi:eetagprobe' + str(chID) + ' -> ' + Channel, Cuts, chID, path=path)
    jpsiList.append('J/psi:eetagprobe' + str(chID))
    matchMCTruth('J/psi:eetagprobe0', path=path)
    return jpsiList


def SystematicsTrackingList(path):

    Lists = []
    Lists += BtoDStarPiList(path)
    Lists += DstarToD0PiPartList(path)

    return Lists


def DstarToD0PiPartList(path):

    fillParticleList("pi+:fromks", "chiProb > 0.001 and pionID > 0.1 and d0 > 0.1", path=path)

# D-
    DminusCuts = '1.0 < M < 1.75'
    DminusChannel = ['pi+:fromks pi+:loose pi-:loose']

    for chID, channel in enumerate(DminusChannel):
        resonanceName = 'D-:loose' + str(chID)
        reconstructDecay(resonanceName + ' -> ' + channel, DminusCuts, chID, path=path)

# Dstar

    DstarCuts = 'massDifference(0)<0.2 and useCMSFrame(p) > 2.0'
    DstarChannel = []
    DstarChannel.append('D-:loose0' + ' pi+:loose')

    DstarList = []
    for chID, channel in enumerate(DstarChannel):
        resonanceName = 'D*0:loose' + str(chID)
        reconstructDecay(resonanceName + ' -> ' + channel, DstarCuts, chID, path=path)
        DstarList.append(resonanceName)
        matchMCTruth(resonanceName, path=path)

    return DstarList


def BtoDStarPiList(path):
    D0Cuts = '1.835 < M < 1.895'
    DstarCuts = 'massDifference(0)<0.16'
    B0Cuts = 'Mbc > 5.2 and abs(deltaE) < 0.3'

# D0

    D0Channel = ['K+:loose pi-:loose', 'K+:loose pi-:loose pi-:loose pi+:loose', 'K+:loose pi-:loose pi0:loose']

    D0List = []
    for chID, channel in enumerate(D0Channel):
        resonanceName = 'anti-D0:loose' + str(chID)
        reconstructDecay(resonanceName + ' -> ' + channel, D0Cuts, chID, path=path)
#        vertexRave(resonanceName, 0.0, path=path)
        matchMCTruth(resonanceName, path=path)
    copyLists('anti-D0:loose', ["anti-D0:loose0", "anti-D0:loose1", "anti-D0:loose2"], path=path)
    D0List.append('anti-D0:loose')
# Dstar

    DstarChannel = []
    for channel in D0List:
        DstarChannel.append(channel + ' pi-:loose')

    DstarList = []
    for chID, channel in enumerate(DstarChannel):
        resonanceName = 'D*-:loose' + str(chID)
        reconstructDecay(resonanceName + ' -> ' + channel, DstarCuts, chID, path=path)
#        vertexRave(resonanceName, 0.0)
        DstarList.append(resonanceName)
        matchMCTruth(resonanceName, path=path)

# B0

    B0Channel = []
    for channel in DstarList:
        B0Channel.append(channel + ' pi+:loose')

    B0List = []
    for chID, channel in enumerate(B0Channel):
        resonanceName = 'B0:sys' + str(chID)
        reconstructDecay(resonanceName + ' -> ' + channel, B0Cuts, chID, path=path)
        B0List.append(resonanceName)
#        vertexRave(resonanceName, 0.0)
        matchMCTruth(resonanceName, path=path)

    return B0List


def XiList(path):
    LambdaCuts = '1.10 < M < 1.13'
    XiCuts = '1.3 < M < 1.34'

    LambdaChannel = ['p+:all pi-:all'
                     ]

    LambdaList = []
    for chID, channel in enumerate(LambdaChannel):
        reconstructDecay('Lambda0:syst' + str(chID) + ' -> ' + channel, LambdaCuts, chID, path=path)
        massVertexRave('Lambda0:syst' + str(chID), 0.001, path=path)
        LambdaList.append('Lambda0:syst' + str(chID))

    XiChannel = []
    for channel in LambdaList:
        XiChannel.append(channel + ' pi-:all')

    XiList = []
    for chID, channel in enumerate(XiChannel):
        reconstructDecay('Xi-:syst' + str(chID) + ' -> ' + channel, XiCuts, chID, path=path)
        XiList.append('Xi-:syst' + str(chID))

    return XiList
