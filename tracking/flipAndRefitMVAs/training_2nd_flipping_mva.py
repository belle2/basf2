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

    training_data = basf2_mva.vector("train_2ndmva.root")
    test_data = basf2_mva.vector("test_2ndmva.root")

    # training variables
    variables = ["flipped_pz_estimate",
                 "y_variance",
                 "tan_lambda_estimate",
                 "d0_variance",
                 "x_variance",
                 "z_estimate",
                 "phi0_variance",
                 "px_variance",
                 "pz_estimate",
                 "p_value",
                 "pt_estimate",
                 "y_estimate",
                 "d0_estimate",
                 "x_estimate",
                 "py_variance",
                 "pz_variance",
                 "omega_variance",
                 "tan_lambda_variance",
                 "z_variance",
                 "omega_estimate",
                 "pt_resolution",
                 "px_estimate",
                 "pt_variance",
                 "phi0_estimate",
                 "flipped_z_estimate",
                 "py_estimate",
                 "flipped_z_variance",
                 "flipped_pz_variance",
                 "flipped_pt_variance",
                 "flipped_py_estimate",
                 "z0_variance",
                 "flipped_p_value",
                 "flipped_px_variance",
                 "flipped_py_variance",
                 "flipped_x_estimate",
                 "quality_flip_indicator"]
    # "inGoingArmTime",
    # "inGoingArmTimeError",
    # "outGoingArmTime",
    # "outGoingArmTimeError",
    # "timeDiffInAndOutArms"]

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = training_data
    general_options.m_identifier = "TRKTrackFlipAndRefit_MVA2_weightfile"
    general_options.m_treename = "data"
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "isPrimary_misID"
    general_options.m_max_events = 50000

    fastbdt_options = basf2_mva.FastBDTOptions()
    fastbdt_options.m_nTrees = 400
    fastbdt_options.m_nCuts = 16
    fastbdt_options.m_nLevels = 6
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
