#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Add covariance and generation flags to BeamParameters
# for run-independent MC.

from ROOT import Belle2, TMatrixDSym
import basf2

basf2.conditions.prepend_globaltag('B2BII_MC')

db = Belle2.Database.Instance()

cov = TMatrixDSym(3)

# Experiments with run-independent MC.
exp_list = [7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 31, 33, 35, 37, 39,
            41, 43, 45, 47, 49, 51, 53, 55, 61, 63, 65, 67, 69, 71]

for exp in exp_list:
    beam_parameters = db.getData('BeamParameters', exp, 0)
    cov[0][0] = 0.00513 * 0.00513
    beam_parameters.setCovHER(cov)
    cov[0][0] = 0.002375 * 0.002375
    beam_parameters.setCovLER(cov)
    beam_parameters.setGenerationFlags(Belle2.MCInitialParticles.c_smearALL)
    iov = Belle2.IntervalOfValidity(exp, 0, exp, 0)
    db.storeData('BeamParameters', beam_parameters, iov)
