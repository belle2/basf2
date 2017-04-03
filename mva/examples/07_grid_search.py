#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2017

import basf2_mva
import basf2_mva_util
import multiprocessing

if __name__ == "__main__":

    # Train model with default parameters
    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
    general_options.m_treename = "tree"
    general_options.m_identifier = "test.xml"
    general_options.m_variables = basf2_mva.vector('p', 'pz', 'daughter(0, Kid)')
    general_options.m_target_variable = "isSignal"

    fastbdt_options = basf2_mva.FastBDTOptions()
    basf2_mva.teacher(general_options, fastbdt_options)

    # Load the model and train it again searching for the best hyperparameters
    def grid_search(nTrees):
        method = basf2_mva_util.Method("test.xml")
        options = basf2_mva.FastBDTOptions()
        options.m_nTrees = nTrees
        m = method.train_teacher(basf2_mva.vector('train.root'), 'tree', specific_options=options)
        p, t = m.apply_expert(basf2_mva.vector('test.root'), 'tree')
        return nTrees, basf2_mva_util.calculate_roc_auc(p, t)

    p = multiprocessing.Pool(None, maxtasksperchild=1)
    results = p.map(grid_search, [10, 20, 50, 100, 200, 500, 1000])
    for nTrees, auc in results:
        print("nTrees", nTrees, "AUC", auc)
