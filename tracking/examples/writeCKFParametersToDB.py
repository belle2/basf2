#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from ROOT import Belle2


def writeCKFParametersToDB(iovList=(0, 0, 0, 0), ptThreshold=-1., layerJumpLowPt=1, layerJumpHighPt=1):
    """
    run this script to create db file storing the payload information of the CKFParameters
    see `tracking/dbobjects/include/CKFParameters.h` for definition of the parameters
    """

    # just a small sanity check (expLow, runLow, expHigh, runHigh)
    if len(iovList) != 4:
        return

    # make sure the defined parameters make sense.
    if layerJumpLowPt < 0 or layerJumpHighPt < 0:
        return
    # If you don't want to use different parameters for low/high pt,
    # please set 'ptThreshold' to -1 and assign same value to layerJumpLowPt/layerJumpHighPt
    if ptThreshold <= 0.:
        if (layerJumpLowPt != layerJumpHighPt):
            return

    # create the iov
    iov = Belle2.IntervalOfValidity(*iovList)
    # and the payload object
    dbobj = Belle2.CKFParameters()
    # then set the parameters it contains
    dbobj.setLayerJumpPtThreshold(ptThreshold)
    dbobj.setLayerJumpLowPt(layerJumpLowPt)
    dbobj.setLayerJumpHighPt(layerJumpHighPt)

    # write db object to 'localdb/'
    Belle2.Database.Instance().storeData("PXDCKFSeedHitParameters", dbobj, iov)


if __name__ == "__main__":
    # writeCKFParametersToDB((1002,0,1003,-1), -1, 1, 1)
    writeCKFParametersToDB((0, 0, 0, -1), 0.5, 0, 1)
