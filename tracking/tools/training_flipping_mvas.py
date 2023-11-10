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
        var = ["seed_pz_estimate",
               "seed_pz_variance",
               "seed_z_estimate",
               "seed_tan_lambda_estimate",
               "seed_pt_estimate",
               "seed_x_estimate",
               "seed_y_estimate",
               "seed_py_variance",
               "seed_d0_estimate",
               "seed_omega_variance",
               "svd_layer6_clsTime",
               "seed_tan_lambda_variance",
               "seed_z_variance",
               "n_svd_hits",
               "n_cdc_hits",
               "svd_layer3_positionSigma",
               "first_cdc_layer",
               "last_cdc_layer",
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
              "tan_lambda_estimate",
              "d0_variance",
              "z_estimate",
              "px_variance",
              "p_value",
              "pt_estimate",
              "y_estimate",
              "d0_estimate",
              "x_estimate",
              "pz_variance",
              "omega_estimate",
              "px_estimate",
              "flipped_z_estimate",
              "py_estimate",
              "outGoingArmTime",
              "quality_flip_indicator",
              "inGoingArmTime"]

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
    general_options.m_identifier = f"TRKTrackFlipAndRefit_MVA{args.mva}_weightfile"
    general_options.m_treename = args.tree
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "ismatched_WC"
    general_options.m_max_events = 0

    fastbdt_options = basf2_mva.FastBDTOptions()
    if args.mva == 1:
        # configurations for MVA1
        fastbdt_options.m_nTrees = 150
        fastbdt_options.m_nCuts = 18
        fastbdt_options.m_nLevels = 4
        fastbdt_options.m_shrinkage = 0.2
        fastbdt_options.m_randRatio = 0.5
        fastbdt_options.m_purityTransformation = False
        fastbdt_options.m_sPlot = False

    if args.mva == 2:
        # configurations for MVA2
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
