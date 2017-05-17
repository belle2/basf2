#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2017

import basf2_mva

if __name__ == "__main__":
    variables = ['M', 'p', 'pt', 'pz',
                 'daughter(0, p)', 'daughter(0, pz)', 'daughter(0, pt)',
                 'daughter(1, p)', 'daughter(1, pz)', 'daughter(1, pt)',
                 'daughter(2, p)', 'daughter(2, pz)', 'daughter(2, pt)',
                 'chiProb', 'dr', 'dz',
                 'daughter(0, dr)', 'daughter(1, dr)',
                 'daughter(0, dz)', 'daughter(1, dz)',
                 'daughter(0, chiProb)', 'daughter(1, chiProb)', 'daughter(2, chiProb)',
                 'daughter(0, Kid)', 'daughter(0, piid)']

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
    general_options.m_treename = "tree"
    general_options.m_variables = basf2_mva.vector(*variables)
    # Spectators are the variables for which the selection should be uniform
    general_options.m_spectators = basf2_mva.vector('daughterInvariantMass(0, 1)', 'daughterInvariantMass(0, 2)')
    general_options.m_target_variable = "isSignal"
    general_options.m_identifier = "uboost"

    specific_options = basf2_mva.PythonOptions()
    specific_options.m_steering_file = 'mva/examples/orthogonal_discriminator/uboost.py'
    # Set the parameters of the uBoostClassifier,
    # defaults are 50, which is reasonable, but I want to have a example runtime < 2 minutes
    import json
    specific_options.m_config = json.dumps({'n_neighbors': 50, 'n_estimators': 50})
    specific_options.m_framework = 'hep_ml'
    basf2_mva.teacher(general_options, specific_options)
