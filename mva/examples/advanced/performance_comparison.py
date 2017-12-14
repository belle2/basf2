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
    general_options.m_identifier = "MVADatabaseIdentifier"
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "isSignal"

    trivial_options = basf2_mva.TrivialOptions()

    data_options = basf2_mva.FastBDTOptions()
    data_options.m_nTrees = 0

    fastbdt_options = basf2_mva.FastBDTOptions()
    fastbdt_options.m_nTrees = 100
    fastbdt_options.m_nCuts = 10
    fastbdt_options.m_nLevels = 3
    fastbdt_options.m_shrinkage = 0.2
    fastbdt_options.m_randRatio = 0.5

    fann_options = basf2_mva.FANNOptions()
    fann_options.m_number_of_threads = 1
    fann_options.m_max_epochs = 100
    fann_options.m_validation_fraction = 0.001
    fann_options.m_test_rate = fann_options.m_max_epochs + 1  # Never test
    fann_options.m_hidden_layers_architecture = "N+1"
    fann_options.m_random_seeds = 1

    tmva_bdt_options = basf2_mva.TMVAOptionsClassification()
    tmva_bdt_options.m_config = ("!H:!V:CreateMVAPdfs:NTrees=100:BoostType=Grad:Shrinkage=0.2:UseBaggedBoost:"
                                 "BaggedSampleFraction=0.5:nCuts=1024:MaxDepth=3:IgnoreNegWeightsInTraining")
    tmva_bdt_options.m_prepareOptions = ("SplitMode=block:V:nTrain_Signal=9691:nTrain_Background=136972:"
                                         "nTest_Signal=1:nTest_Background=1")

    tmva_nn_options = basf2_mva.TMVAOptionsClassification()
    tmva_nn_options.m_type = "MLP"
    tmva_nn_options.m_method = "MLP"
    tmva_nn_options.m_config = ("H:!V:CreateMVAPdfs:VarTransform=N:NCycles=100:HiddenLayers=N+1:TrainingMethod=BFGS")
    tmva_nn_options.m_prepareOptions = ("SplitMode=block:V:nTrain_Signal=9691:nTrain_Background=136972:"
                                        "nTest_Signal=1:nTest_Background=1")

    sklearn_bdt_options = basf2_mva.PythonOptions()
    sklearn_bdt_options.m_framework = "sklearn"
    param = '{"n_estimators": 100, "learning_rate": 0.2, "max_depth": 3, "random_state": 0, "subsample": 0.5}'
    sklearn_bdt_options.m_config = param

    xgboost_options = basf2_mva.PythonOptions()
    xgboost_options.m_framework = "xgboost"
    param = ('{"max_depth": 3, "eta": 0.1, "silent": 1, "objective": "binary:logistic",'
             '"subsample": 0.5, "nthread": 1, "nTrees": 400}')
    xgboost_options.m_config = param

    stats = []
    test_data = ["validation.root"]
    for label, options in [("DataLoading", data_options), ("FastBDT", fastbdt_options), ("FANN", fann_options),
                           ("TMVA-BDT", tmva_bdt_options), ("TMVA-NN", tmva_nn_options),
                           ("SKLearn-BDT", sklearn_bdt_options), ("XGBoost", xgboost_options), ("Trivial", trivial_options)]:
        training_start = time.time()
        general_options.m_identifier = label
        basf2_mva.teacher(general_options, options)
        training_stop = time.time()
        training_time = training_stop - training_start
        method = basf2_mva_util.Method(general_options.m_identifier)
        inference_start = time.time()
        p, t = method.apply_expert(basf2_mva.vector(*test_data), general_options.m_treename)
        inference_stop = time.time()
        inference_time = inference_stop - inference_start
        auc = basf2_mva_util.calculate_roc_auc(p, t)
        print(label, training_time, inference_time, auc)
        stats.append((label, training_time, inference_time, auc))

    for l in stats:
        print(*l)
