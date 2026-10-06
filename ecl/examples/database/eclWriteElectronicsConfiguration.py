#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Write the ECLElectronicsConfiguration payload to the local database ('localdb/').
It is used by ECLDigitizer, ECLWaveformFit and ECLDigitCalibrator to choose
between the current and the upgraded ECL electronics.

Example: ``basf2 eclWriteElectronicsConfiguration.py -- --iov 2004 0 2006 -1`` (add ``--no-upgrade``
for the current electronics)
"""

import argparse

from ROOT import Belle2


def writeECLElectronicsConfigurationToDB(IoV=(2004, 0, 2006, -1), upgrade=True):
    """
    Write the ECLElectronicsConfiguration payload to 'localdb/'.

    :param IoV: IoV for the payload (expLow, runLow, expHigh, runHigh)
    :param upgrade: True if the upgraded ECL electronics is used
    """
    b2IoV = Belle2.IntervalOfValidity(*IoV)
    dbobj = Belle2.ECLElectronicsConfiguration()
    dbobj.setUpgrade(upgrade)
    Belle2.Database.Instance().storeData("ECLElectronicsConfiguration", dbobj, b2IoV)
    print(f"Successfully wrote payload ECLElectronicsConfiguration with {IoV=} and {upgrade=}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--iov", type=int, nargs=4, default=[2004, 0, 2006, -1],
                        metavar=("EXP_LOW", "RUN_LOW", "EXP_HIGH", "RUN_HIGH"), help="IoV of the payload")
    parser.add_argument("--no-upgrade", dest="upgrade", action="store_false",
                        help="Use the current instead of the upgraded ECL electronics")
    args = parser.parse_args()
    writeECLElectronicsConfigurationToDB(tuple(args.iov), args.upgrade)
