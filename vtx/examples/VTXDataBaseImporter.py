#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


##############################################################################
#
# Imports a VTX specific payloads to local data base for uploading to GT
#
##############################################################################


import basf2 as b2
from ROOT import Belle2
import ROOT


def importVXDQualityEstimatorParameters(expLow=0, runLow=0, expHigh=-1, runHigh=-1, vtx_geometry='7layers'):
    """
    Imports VXDEstimatorParameters for different VTX geometry options.
    """

    # Create VXDEstimatorParameters
    VXDQualityEstimatorParameters = Belle2.VXDQualityEstimatorParameters()
    if vtx_geometry == '7layers':
        VXDQualityEstimatorParameters.setEstimationMethod("circleFit")
    elif vtx_geometry == '5layers':
        VXDQualityEstimatorParameters.setEstimationMethod("tripletFit")
        VXDQualityEstimatorParameters.setMaterialBudgetFactor(1.2)
        VXDQualityEstimatorParameters.setMaxPt(0.01)
        VXDQualityEstimatorParameters.setMinRequiredQuality(0.0)
    else:
        VXDQualityEstimatorParameters.setEstimationMethod("tripletFit")
        VXDQualityEstimatorParameters.setMaterialBudgetFactor(1.2)
        VXDQualityEstimatorParameters.setMaxPt(0.01)
        VXDQualityEstimatorParameters.setMinRequiredQuality(0.0)

    # Set the iov for the mask
    iov = Belle2.IntervalOfValidity(expLow, runLow, expHigh, runHigh)
    Belle2.Database.Instance().storeData('VXDQualityEstimatorParameters', VXDQualityEstimatorParameters, iov)


if __name__ == "__main__":

    importVXDQualityEstimatorParameters()
