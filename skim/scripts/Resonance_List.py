#######################################################
#
# Resonance rediscovery skim
# Y. Kato, Mar/2018
#
######################################################

from basf2 import *
from modularAnalysis import *


def ResonanceList():
    DsList = getDsList()
    DstarList = getDstarList()
    SigmacList = getSigmacList()
    MuMugList = getmumugList()
    BZeroList = getBZeroList()
    BPlusList = getBPlusList()
    return (DsList + DstarList + SigmacList + MuMugList + BZeroList + BPlusList)


def getDsList():
    DsCuts = '1.90 < M < 2.04'

    reconstructDecay('phi:loose -> K+:loose K-:loose', '1.01 < M < 1.03')
    reconstructDecay('K*0:loose -> K+:loose pi-:loose', '0.7 < M < 1.1')

    DsChannel = ['phi:loose pi+:loose']
    DsList = []
    for chID, channel in enumerate(DsChannel):
        particlename = 'D_s+:Resonance%d' % (chID)
        reconstructDecay(particlename + ' -> ' + channel, DsCuts, chID)
        matchMCTruth(particlename)
        DsList.append(particlename)

    return DsList


def getDstarList():
    DplusCuts = '1.8 < M < 1.93'
    DstarCuts = 'massDifference(0)<0.16 and useCMSFrame(p)>2.0'

    DplusChannel = ['K-:loose pi+:loose pi+:loose']

    DplusList = []
    for chID, channel in enumerate(DplusChannel):
        reconstructDecay('D+:resonance' + str(chID) + ' -> ' + channel, DplusCuts, chID)
        vertexRave('D+:resonance' + str(chID), 0.0)
        DplusList.append('D+:resonance' + str(chID))

    DstarChannel = []
    for channel in DplusList:
        DstarChannel.append(channel + ' pi0:loose')

    DstarList = []
    for chID, channel in enumerate(DstarChannel):
        reconstructDecay('D*+:resonance' + str(chID) + ' -> ' + channel, DstarCuts, chID)
        DstarList.append('D*+:resonance' + str(chID))
        matchMCTruth('D*+:resonance0')

    return DstarList


def getSigmacList():
    LambdacCuts = '2.24 < M < 2.33'
    SigmacCuts = 'massDifference(0)<0.28 and useCMSFrame(p) > 2.5'

    LambdacChannel = ['p+:loose K-:loose pi+:loose']
    LambdacList = []
    for chID, channel in enumerate(LambdacChannel):
        reconstructDecay('Lambda_c+:resonance' + str(chID) + ' -> ' + channel, LambdacCuts, chID)
        vertexRave('Lambda_c+:resonance' + str(chID), 0.0)
        LambdacList.append('Lambda_c+:resonance' + str(chID))

    SigmacList = []
    SigmacPlusChannel = []
# Sigma_c++
    for channel in LambdacList:
        SigmacPlusChannel.append(channel + ' pi+:loose')

    for chID, channel in enumerate(SigmacPlusChannel):
        reconstructDecay('Sigma_c++:resonance' + str(chID) + ' -> ' + channel, SigmacCuts, chID)
        SigmacList.append('Sigma_c++:resonance' + str(chID))
        matchMCTruth('Sigma_c++:resonance0')

# Sigma_c0
    Sigmac0Channel = []
    for channel in LambdacList:
        Sigmac0Channel.append(channel + ' pi-:loose')

    Sigmac0List = []
    for chID, channel in enumerate(Sigmac0Channel):
        reconstructDecay('Sigma_c0:resonance' + str(chID) + ' -> ' + channel, SigmacCuts, chID)
        SigmacList.append('Sigma_c0:resonance' + str(chID))
        matchMCTruth('Sigma_c0:resonance0')

    return SigmacList


def getmumugList():

    vphoChannel = ['mu+:loose mu-:loose']
    vphocuts = ''
    vphoList = []
    for chID, channel in enumerate(vphoChannel):
        resonanceName = 'vpho:resonance' + str(chID)
        reconstructDecay('vpho:resonance' + str(chID) + ' -> ' + channel, vphocuts, chID)
        applyCuts(resonanceName, 'nTracks == 2 and M < formula(Ecms*0.9877)')
        matchMCTruth(resonanceName)
        vertexRave(resonanceName, 0.0)
        applyCuts(resonanceName, 'M < formula(Ecms*0.9877)')
        vphoList.append(resonanceName)

    return vphoList


def getBPlusList():
    antiDZeroCut = '1.82 < M < 1.90'
    antiDZeroChannel = ['K+:loose pi-:loose']
    antiDZeroList = []

    for chID, channel in enumerate(antiDZeroChannel):
        resonanceName = 'anti-D0:resonance' + str(chID)
        reconstructDecay(resonanceName + ' -> ' + channel, antiDZeroCut, chID)
        vertexRave(resonanceName, 0.0)
        antiDZeroList.append(resonanceName)

    BPlusChannel = []
    for channel in antiDZeroList:
        BPlusChannel.append(channel + ' pi+:loose')

    BPlusCuts = 'Mbc > 5.2 and abs(deltaE) < 0.3'
    BPlusList = []
    for chID, channel in enumerate(BPlusChannel):
        reconstructDecay('B+:resonance' + str(chID) + ' -> ' + channel, BPlusCuts, chID)
        BPlusList.append('B+:resonance' + str(chID))
        matchMCTruth('B+:resonance' + str(chID))

    return BPlusList


def getBZeroList():
    BZeroCuts = 'Mbc > 5.2 and abs(deltaE) < 0.3'
    BZeroChannel = ['D-:resonance0 pi+:loose']
    BZeroList = []

    for chID, channel in enumerate(BZeroChannel):
        resonanceName = 'B0:resonance' + str(chID)
        reconstructDecay(resonanceName + ' -> ' + channel, BZeroCuts, chID)
        BZeroList.append(resonanceName)
        matchMCTruth(resonanceName)

    return BZeroList
