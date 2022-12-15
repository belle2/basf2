#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Use training in a basf2 path
# Run basics/*.py before

import basf2_mva
import basf2_mva_util
import numpy as np
from basf2 import conditions


if __name__ == "__main__":

    conditions.testing_payloads = ['localdb/database.txt']

    training_data = basf2_mva.vector("test_tmp/train_1stmva.root")
    test_data = basf2_mva.vector("test_tmp/test_1stmva.root")

    # rate by the importance
    variables = ["d0_variance",
                 "seed_pz_estimate",
                 "n_hits",
                 "z0_estimate",
                 "seed_pz_variance",
                 "phi0_variance",
                 "seed_z_estimate",
                 "tan_lambda_estimate",
                 "omega_variance",
                 "seed_tan_lambda_estimate",
                 "d0_estimate",
                 "seed_pt_estimate",
                 # "cdc_qualityindicator",
                 "omega_estimate",
                 "z0_variance",
                 "seed_x_estimate",
                 "seed_y_estimate",
                 # "seed_pt_resolution",
                 "seed_py_variance",
                 "seed_d0_estimate",
                 "seed_omega_variance",
                 # "tan_lambda_variance",
                 "svd_layer6_clsTime",
                 "seed_tan_lambda_variance",
                 "seed_z_variance",
                 "n_svd_hits",
                 "phi0_estimate",
                 "n_cdc_hits",
                 "svd_layer3_positionSigma",
                 "first_cdc_layer",
                 "last_cdc_layer",
                 "ndf_hits",
                 "inGoingArmTime",
                 "inGoingArmTimeError",
                 "outGoingArmTime",
                 "outGoingArmTimeError",
                 "InOutArmTimeDifference",
                 "InOutArmTimeDifferenceError"]

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = training_data
    '''
    # the official name of the weight file is `TRKTrackFlipAndRefit_MVA1_weightfile`
    # But the evaluation scripts taking `Weightfile` as a default name, so that's why this line was commented
    general_options.m_identifier = "TRKTrackFlipAndRefit_MVA1_weightfile"
    '''
    general_options.m_identifier = "Weightfile"
    general_options.m_treename = "data"
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "isPrimary_misID"
    general_options.m_max_events = 6000

    fastbdt_options = basf2_mva.FastBDTOptions()
    fastbdt_options.m_nTrees = 400
    fastbdt_options.m_nCuts = 20
    fastbdt_options.m_nLevels = 4
    fastbdt_options.m_shrinkage = 0.1
    fastbdt_options.m_randRatio = 0.5
    fastbdt_options.m_purityTransformation = False
    fastbdt_options.m_sPlot = False
    basf2_mva.teacher(general_options, fastbdt_options)

    m = basf2_mva_util.Method(general_options.m_identifier)
    p, t = m.apply_expert(test_data, general_options.m_treename)
    res = basf2_mva_util.calculate_auc_efficiency_vs_background_retention(p, t)

    print(res)
    print("Variable importances returned my method")

    imp = np.array([m.importances.get(v, 0.0) for v in m.variables])
    width = (np.max(imp) - np.min(imp))

    for var in m.variables:
        print(var, (m.importances.get(var, 0.0) - np.min(imp))/width * 100)
