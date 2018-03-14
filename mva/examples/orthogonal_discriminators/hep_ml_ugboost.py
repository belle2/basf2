#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2017

import basf2_mva
import hep_ml
import hep_ml.losses
import hep_ml.gradientboosting


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """
    Create hep_ml classifier and store it in a State object.
    """
    train_features = list(range(number_of_features))
    uniform_features = [number_of_features + i for i in range(number_of_spectators)]

    loss = hep_ml.losses.AdaLossFunction()
    if parameters is not None and 'uniform_rate' in parameters:
        loss = hep_ml.losses.BinFlatnessLossFunction(uniform_features=uniform_features, uniform_label=[0, 1],
                                                     fl_coefficient=parameters['uniform_rate'])
    clf = hep_ml.gradientboosting.UGradientBoostingClassifier(loss=loss, n_estimators=100, subsample=0.5,
                                                              max_depth=5, train_features=train_features)
    return State(clf)


if __name__ == "__main__":
    variables = ['p', 'pt', 'pz', 'phi',
                 'daughter(0, p)', 'daughter(0, pz)', 'daughter(0, pt)', 'daughter(0, phi)',
                 'daughter(1, p)', 'daughter(1, pz)', 'daughter(1, pt)', 'daughter(1, phi)',
                 'daughter(2, p)', 'daughter(2, pz)', 'daughter(2, pt)', 'daughter(2, phi)',
                 'chiProb', 'dr', 'dz', 'dphi',
                 'daughter(0, dr)', 'daughter(1, dr)', 'daughter(0, dz)', 'daughter(1, dz)',
                 'daughter(0, dphi)', 'daughter(1, dphi)',
                 'daughter(0, chiProb)', 'daughter(1, chiProb)', 'daughter(2, chiProb)',
                 'daughter(0, kaonID)', 'daughter(0, pionID)', 'daughter(1, kaonID)', 'daughter(1, pionID)',
                 'daughterAngle(0, 1)', 'daughterAngle(0, 2)', 'daughterAngle(1, 2)',
                 'daughter(2, daughter(0, E))', 'daughter(2, daughter(1, E))',
                 'daughter(2, daughter(0, clusterTiming))', 'daughter(2, daughter(1, clusterTiming))',
                 'daughter(2, daughter(0, clusterE9E25))', 'daughter(2, daughter(1, clusterE9E25))',
                 'daughter(2, daughter(0, minC2HDist))', 'daughter(2, daughter(1, minC2HDist))',
                 'M']

    variables2 = ['p', 'pt', 'pz', 'phi',
                  'chiProb', 'dr', 'dz', 'dphi',
                  'daughter(2, chiProb)',
                  'daughter(0, kaonID)', 'daughter(0, pionID)', 'daughter(1, kaonID)', 'daughter(1, pionID)',
                  'daughter(2, daughter(0, E))', 'daughter(2, daughter(1, E))',
                  'daughter(2, daughter(0, clusterTiming))', 'daughter(2, daughter(1, clusterTiming))',
                  'daughter(2, daughter(0, clusterE9E25))', 'daughter(2, daughter(1, clusterE9E25))',
                  'daughter(2, daughter(0, minC2HDist))', 'daughter(2, daughter(1, minC2HDist))']

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
    general_options.m_treename = "tree"
    general_options.m_variables = basf2_mva.vector(*variables)
    # Spectators are the variables for which the selection should be uniform
    general_options.m_spectators = basf2_mva.vector('daughterInvariantMass(0, 1)', 'daughterInvariantMass(0, 2)')
    general_options.m_target_variable = "isSignal"
    general_options.m_identifier = "hep_ml_baseline"

    specific_options = basf2_mva.PythonOptions()
    specific_options.m_framework = 'hep_ml'
    specific_options.m_steering_file = 'mva/examples/orthogonal_discriminators/hep_ml_ugboost.py'
    basf2_mva.teacher(general_options, specific_options)

    # Set the parameters of the uBoostClassifier
    import json
    specific_options.m_config = json.dumps({'uniform_rate': 10.0})
    general_options.m_identifier = "hep_ml"
    basf2_mva.teacher(general_options, specific_options)

    specific_options = basf2_mva.PythonOptions()
    general_options.m_identifier = "hep_ml_feature_drop"
    specific_options.m_framework = 'hep_ml'
    specific_options.m_steering_file = 'mva/examples/orthogonal_discriminators/hep_ml_ugboost.py'
    general_options.m_variables = basf2_mva.vector(*variables2)
    basf2_mva.teacher(general_options, specific_options)
