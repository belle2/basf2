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
# Imports a VXDTF quality parameters to local data base.
#
# Usage: python3 VXDTFQualityParameterDataBaseImporter.py --estimationMethod=tripletFit --maxPt=0.1 --materialBudgetFactor=1.2
##############################################################################


import basf2 as b2
from ROOT import Belle2
import ROOT


import argparse


def parse_args():
    """Parse command line arguments."""
    parser = argparse.ArgumentParser('VXDTFQualityParameterDataBaseImporter.py')
    add_arg = parser.add_argument
    add_arg('--estimationMethod', default='circleFit', type=str)
    add_arg('--maxPt', default=0.01, type=float)
    add_arg('--materialBudgetFactor', default=1.2, type=float)
    return parser.parse_args()


# set some random seed
b2.set_random_seed(10346)


def importVXDTFQualityParameters(
        estimationMethod="tripletFit",
        maxPt=0.01,
        materialBudgetFactor=1.2,
        expLow=0,
        runLow=0,
        expHigh=-1,
        runHigh=-1):
    """
    This function imports an empty payloads for the VXDTF2
    triplet fit quality indicator.

    This function works for every Belle 2 geometry
    """

    QualityParams = Belle2.VXDQualityEstimatorParameters()
    QualityParams.setEstimationMethod(estimationMethod)
    QualityParams.setMaterialBudgetFactor(materialBudgetFactor)
    QualityParams.setMaxPt(maxPt)

    # Set the iov for the mask
    iov = Belle2.IntervalOfValidity(expLow, runLow, expHigh, runHigh)
    Belle2.Database.Instance().storeData('VXDQualityEstimatorParameters', QualityParams, iov)


if __name__ == "__main__":

    # Parse the command line
    args = parse_args()

    importVXDTFQualityParameters(
        estimationMethod=args.estimationMethod,
        maxPt=args.maxPt,
        materialBudgetFactor=args.materialBudgetFactor
    )
