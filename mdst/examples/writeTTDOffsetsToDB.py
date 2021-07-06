#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from ROOT import Belle2


def writeTTDOffsetsToDB(iovList=(0, 0, 0, 0), injectionDelay=(0, 0), triggeredBunchOffset=(0, 0)):
    """
    run this script to create db file storing the payload information of the TTDOffsets
    see `mdst/dbobjects/include/TTDOffsets.h` for definition of the parameters
    Parameters `injectionDelay` and `triggeredBunchOffset` are tuples for (HER, LER)
    """

    # just a few small sanity checks, e.g. iovList=(expLow, runLow, expHigh, runHigh)
    if len(iovList) != 4:
        return
    if len(injectionDelay) != 2:
        return
    if len(triggeredBunchOffset) != 2:
        return

    # create the iov
    iov = Belle2.IntervalOfValidity(*iovList)
    # and the payload object
    dbobj = Belle2.TTDOffsets()
    # then set the parameters it contains
    dbobj.setInjectionDelayHER(injectionDelay[0])
    dbobj.setInjectionDelayLER(injectionDelay[1])
    dbobj.setTriggeredBunchOffsetHER(triggeredBunchOffset[0])
    dbobj.setTriggeredBunchOffsetLER(triggeredBunchOffset[1])

    # write db object to 'localdb/'
    Belle2.Database.Instance().storeData("TTDOffsets", dbobj, iov)


if __name__ == "__main__":
    writeTTDOffsetsToDB((0, 0, -1, -1), (0, 0), (0, 0))
