#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from ROOT import Belle2


def writeROICalculationParametersToDB(iovList=(0, 0, 0, 0),
                                      toleranceZ: float = 0.5,
                                      tolerancePhi: float = 0.15,
                                      sigmaSystU: float = 0.02,
                                      sigmaSystV: float = 0.0,
                                      numSigmaTotU: float = 10,
                                      numSigmaTotV: float = 10,
                                      maxWidthU: float = 0.5,
                                      maxWidthV: float = 0.5,) -> None:
    """
    run this script to create db file storing the payload information of the ROICalculationParameters
    see `tracking/dbobjects/include/ROICalculationParameters.h` for definition of the parameters

    :param iovList: List of IoVs for the parameters
    :param toleranceZ: Tolerance for finding sensor in Z coordinate (cm)
    :param m_tolerancePhi: Tolerance for finding sensor in phi coordinate (radians)
    :param sigmaSystU: Fixed width to add in quadrature to the extrapolation error and obtain the ROI U width
    :param sigmaSystV: Fixed width to add in quadrature to the extrapolation error and obtain the ROI V width
    :param numSigmaTotU: Number of sigma (stat+syst) determining the U width of the ROI
    :param numSigmaTotV: Number of sigma (stat+syst) determining the V width of the ROI
    :param maxWidthU: Maximum U width of the ROI
    :param maxWidthV: Maximum V width of the ROI
    """

    # just a small sanity check (expLow, runLow, expHigh, runHigh)
    if len(iovList) != 4:
        return

    # make sure the defined parameters make sense.
    if toleranceZ <= 0 or tolerancePhi <= 0 or sigmaSystU <= 0 or sigmaSystV <= 0 or \
       numSigmaTotU <= 0 or numSigmaTotV <= 0 or maxWidthU <= 0 or maxWidthV <= 0:
        print("Can't use negative parameters for any of the parameters except for disableROIforEveryNth.")
        print(f"The values set are\n\
              {toleranceZ=},\n\
              {tolerancePhi=},\n\
              {sigmaSystU=},\n\
              {sigmaSystV=},\n\
              {numSigmaTotU=},\n\
              {numSigmaTotV=},\n\
              {maxWidthU=},\n\
              {maxWidthV=}")
        return

    # create the iov
    iov = Belle2.IntervalOfValidity(*iovList)
    # and the payload object
    dbobj = Belle2.ROICalculationParameters()
    # then set the parameters it contains
    dbobj.setToleranceZ(toleranceZ)
    dbobj.setTolerancePhi(tolerancePhi)
    dbobj.setSigmaSystU(sigmaSystU)
    dbobj.setSigmaSystV(sigmaSystV)
    dbobj.setNumSigmaTotU(numSigmaTotU)
    dbobj.setNumSigmaTotV(numSigmaTotV)
    dbobj.setMaxWidthU(maxWidthU)
    dbobj.setMaxWidthV(maxWidthV)

    # write db object to 'localdb/'
    Belle2.Database.Instance().storeData("ROICalculationParameters", dbobj, iov)

    print(f"Successfully wrote payload ROI parameters with {iov=} and parameters\n\
          {toleranceZ=}, {tolerancePhi=}, {sigmaSystU=}, {sigmaSystV=},\n\
          {numSigmaTotU=}, {numSigmaTotV=}, {maxWidthU=}, {maxWidthV=}")


if __name__ == "__main__":
    # We want default ROI selection for experiment 0
    writeROICalculationParametersToDB((0, 0, 0, -1), 0.5, 0.15, 0.02, 0.02, 10, 10, 0.5, 0.5)
    # We want default ROI selection for experiment 1002
    writeROICalculationParametersToDB((1002, 0, 1002, -1), 0.5, 0.15, 0.02, 0.02, 10, 10, 0.5, 0.5)
    # We want default ROI selection for experiment 1003
    writeROICalculationParametersToDB((1003, 0, 1003, -1), 0.5, 0.15, 0.02, 0.02, 10, 10, 0.5, 0.5)
