#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import ROOT
from ROOT import Belle2

"""
Create the payload with the object of ECLTimeWalkCorrection class.
This payload is used for energy-depented time correction in eclDigitCalibrator module.
"""

FIRST_EXP = 0
FIRST_RUN = 0
LAST_EXP = -1
LAST_RUN = -1
DBOBJECT_NAME = 'ECLTimeWalkCorrection'


def main():
    """
    """
    # Initial set of parameters.
    # params = [0, 88449., 0.20867E+06, 3.1482, 7.4747, 1279.3]
    # Parameters recalculated with exp 24 data.
    params = [-1.966, 46350., 264600., 1.813, 7.532, 428.3]

    vec_params = ROOT.std.vector('double')()
    vec_params += params

    payload = Belle2.ECLTimeWalkCorrection()
    payload.setParams(vec_params)

    db = Belle2.Database.Instance()
    iov = Belle2.IntervalOfValidity(FIRST_EXP, FIRST_RUN, LAST_EXP, LAST_RUN)
    db.storeData(DBOBJECT_NAME, payload, iov)


if __name__ == '__main__':
    main()
