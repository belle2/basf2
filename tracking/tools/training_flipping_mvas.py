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
from basf2 import conditions, B2FATAL
import argparse


def get_argument_parser() -> argparse.ArgumentParser:
    """ Parses the command line options of the fliping mva training and returns the corresponding arguments. """
    parser = argparse.ArgumentParser()
    parser.add_argument('-train', default='', type=str,
                        help='Data file containing ROOT TTree used during training. Default: \'\'.')
    parser.add_argument('-data', default='', type=str,
                        help='Data file containing ROOT TTree with independent test data. Default: \'\'.')
    parser.add_argument('-tree', default='', type=str,
                        help='Treename in data file. Default: \'\'.')
    parser.add_argument('-mva', default=1, type=int,
                        help='index of mva to be trainned. Default: 1')

    return parser


def get_variables(index=1):
    var = []
    if index == 1:
        var = ["d0_variance",
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
               "InOutArmTimeDifference",
               "InOutArmTimeDifferenceError",
               "inGoingArmTime",
               "inGoingArmTimeError",
               "outGoingArmTime",
               "outGoingArmTimeError"]

    if index == 2:
        # training variables
        var = [
              "flipped_pz_estimate",
              # "y_variance",
              "tan_lambda_estimate",
              "d0_variance",
              # "x_variance",
              "z_estimate",
              # "phi0_variance",
              "px_variance",
              # "pz_estimate",
              "p_value",
              "pt_estimate",
              "y_estimate",
              "d0_estimate",
              "x_estimate",
              # "py_variance",
              "pz_variance",
              # "omega_variance",
              # "tan_lambda_variance",
              # "z_variance",
              "omega_estimate",
              # "pt_resolution",
              "px_estimate",
              # "pt_variance",
              # "phi0_estimate",
              "flipped_z_estimate",
              "py_estimate",
              # "flipped_z_variance",
              # "flipped_pz_variance",
              # "flipped_pt_variance",
              # "flipped_py_estimate",
              # "z0_variance",
              # "flipped_p_value",
              # "flipped_px_variance",
              # "flipped_py_variance",
              # "flipped_x_estimate",
              "outGoingArmTime",
              # "outGoingArmTimeError",
              "quality_flip_indicator",
              # "InOutArmTimeDifference",
              # "InOutArmTimeDifferenceError",
              "inGoingArmTime",
              # "inGoingArmTimeError"
              ]

    return var


if __name__ == "__main__":

    parser = get_argument_parser()
    args = parser.parse_args()

    if args.train == '' or args.data == '':
        B2FATAL("Missing train or test samples. Terminating here.")

    if args.tree == '':
        B2FATAL("Missing Treename. Terminating here.")

    if args.mva not in [1, 2]:
        B2FATAL("MVA number must be either 1 or 2. Terminating here.")

    conditions.testing_payloads = ['localdb/database.txt']

    print(args.train)
    training_data = basf2_mva.vector(args.train)
    test_data = basf2_mva.vector(args.data)

    # get the variables
    variables = get_variables(args.mva)

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = training_data
    '''
    # the official name of the weight file is `TRKTrackFlipAndRefit_MVA1_weightfile`
    # But the evaluation scripts taking `Weightfile` as a default name, so that's why this line was commented
    general_options.m_identifier = "TRKTrackFlipAndRefit_MVA1_weightfile"
    or "TRKTrackFlipAndRefit_MVA2_weightfile" for second MVA
    '''
    general_options.m_identifier = "Weightfile"
    general_options.m_treename = args.tree
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "ismatched_WC"
    general_options.m_max_events = 0

    fastbdt_options = basf2_mva.FastBDTOptions()
    fastbdt_options.m_nTrees = 400
    fastbdt_options.m_nCuts = 25
    fastbdt_options.m_nLevels = 2
    fastbdt_options.m_shrinkage = 0.6
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
