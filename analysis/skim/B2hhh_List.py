#######################################################
#
# Charmless 3-body skims
# C-L. Hsu, 03/Jun/2016
#
######################################################
from basf2 import *
from modularAnalysis import *


def B2hhh_List():
    # Define selection criteria from skim channels
    btocharmlesscuts = '5.2 < Mbc < 5.295 and abs(deltaE) < 0.5'
    bu_PPPChannels = ['pi+:std pi-:std pi+:std',
                      'K+:std K-:std pi+:std',
                      'K+:std K-:std K+:std',
                      'pi+:std pi-:std K+:std'
                      ]

    buPPPList = []
    for chID, channel in enumerate(bu_PPPChannels):
        reconstructDecay('B+:B2HHH' + str(chID) + ' -> ' + channel, btocharmlesscuts, chID)
        buPPPList.append('B+:B2HHH' + str(chID))

    return buPPPList
