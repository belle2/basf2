#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from ROOT import Belle2


def writeROISimulationParametersToDB(IoV=(0, 0, 0, 0),
                                     enableROI: bool = True,
                                     disableROIforEveryNth: int = -1) -> None:
    """
    run this script to create db file storing the payload information of the ROISimulationParameters
    see `simulation/dbobjects/include/ROISimulationParameters.h` for definition of the parameters

    :param IoV: IoV for the parameters
    :param enableROI: Enable ROI finding
    :param disableROIforEveryNth: Disable ROI selection for every n-th event. -1 means ROI selection is always used if
        the parameter enableROI is True.
    """

    # just a small sanity check (expLow, runLow, expHigh, runHigh)
    if len(IoV) != 4:
        return

    # create the iov
    b2IoV = Belle2.IntervalOfValidity(*IoV)
    # and the payload object
    dbobj = Belle2.ROISimulationParameters()
    # then set the parameters it contains
    dbobj.setROIfinding(enableROI)
    dbobj.setDisableROIforEveryNth(disableROIforEveryNth)

    # write db object to 'localdb/'
    Belle2.Database.Instance().storeData("ROISimulationParameters", dbobj, b2IoV)

    print(f"Successfully wrote payload ROISimulationParameters with {IoV=} and \n\
           parameters {enableROI=}, {disableROIforEveryNth=}")


if __name__ == "__main__":
    # We want default ROI selection for experiment 0
    writeROISimulationParametersToDB((0, 0, 0, -1), True, -1)
    # We want default ROI selection for experiment 1002
    writeROISimulationParametersToDB((1002, 0, 1002, -1), False, -1)
    # We want default ROI selection for experiment 1003
    writeROISimulationParametersToDB((1003, 0, 1003, -1), False, -1)
    # We want default ROI selection for experiment 1004
    writeROISimulationParametersToDB((1004, 0, 1004, -1), False, -1)
