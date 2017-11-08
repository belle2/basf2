#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2016

import basf2_mva
import basf2_mva_util
import time

if __name__ == "__main__":

    variables = ['M', 'p', 'pt', 'pz',
                 'daughter(0, p)', 'daughter(0, pz)', 'daughter(0, pt)',
                 'daughter(1, p)', 'daughter(1, pz)', 'daughter(1, pt)',
                 'daughter(2, p)', 'daughter(2, pz)', 'daughter(2, pt)',
                 'chiProb', 'dr', 'dz',
                 'daughter(0, dr)', 'daughter(1, dr)',
                 'daughter(0, dz)', 'daughter(1, dz)',
                 'daughter(0, chiProb)', 'daughter(1, chiProb)', 'daughter(2, chiProb)',
                 'daughter(0, kaonID)', 'daughter(0, pionID)',
                 'daughterInvariantMass(0, 1)', 'daughterInvariantMass(0, 2)', 'daughterInvariantMass(1, 2)']

    # Train a MVA method and directly upload it to the database
    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
    general_options.m_treename = "tree"
    general_options.m_identifier = "TMVA"
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "isSignal"

    tmva_bdt_options = basf2_mva.TMVAOptionsClassification()
    tmva_bdt_options.m_config = ("!H:!V:CreateMVAPdfs:NTrees=100:BoostType=Grad:Shrinkage=0.2:UseBaggedBoost:"
                                 "BaggedSampleFraction=0.5:nCuts=1024:MaxDepth=3:IgnoreNegWeightsInTraining")

    training_start = time.time()
    basf2_mva.teacher(general_options, tmva_bdt_options)
    training_stop = time.time()

    training_time = training_stop - training_start
    method = basf2_mva_util.Method(general_options.m_identifier)

    inference_start = time.time()
    test_data = ["test.root"] * 10
    p, t = method.apply_expert(basf2_mva.vector(*test_data), general_options.m_treename)
    inference_stop = time.time()
    inference_time = inference_stop - inference_start

    auc = basf2_mva_util.calculate_roc_auc(p, t)
    print("TMVA", training_time, inference_time, auc)
