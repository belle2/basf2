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

    return (DsList + DstarList + SigmacList)


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
