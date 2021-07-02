#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# In some cases you create a training outside of basf2
# but you still want to apply the training in basf2
# In this case you can create a fake training which produces a weightfile
# which you can upload to the database, by overriding end_fit
# In addition you have to override load and apply, so that the mva package
# knows how to apply your custom training


import numpy as np
import basf2_mva
import basf2_mva_util


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """ Must be implemented otherwise custom framework will raise an error """
    return None


def begin_fit(state, Xtest, Stest, ytest, wtest):
    """ Must be implemented otherwise custom framework will raise an error """
    return state


def partial_fit(state, X, S, y, w, epoch):
    """ Must be implemented otherwise custom framework will raise an error """
    return True


def feature_importance(state):
    """ Must be implemented otherwise custom framework will raise an error """
    return []


def end_fit(state):
    """
    In end_fit you want to load your training and pass it as a picklable object to basf.
    You do not perform a real training, so your training data can be empty (or a small file).
    Since you do not fit anything here the training data is just ignored.
    However, you want to ensure that the features you are using (and which you will need later)
    are correctly set in the GeneralOptions.
    """
    import pickle
    with open('mytraining.pickle', 'rb') as f:
        my_pickled_classifier = pickle.load(f)
        return my_pickled_classifier


def load(my_pickled_classifier):
    """
    The load function receives your pickled_classifier.
    So maybe you want to initialize your actual classifier here
    using this pickled classifier information.
    In this example I just pass it along
    """
    state = my_pickled_classifier
    return state


def apply(state, X):
    """
    In apply you will get the features you stated in the GeneralOptions.
    In principle you can access the basf2 DataStore yourself in this function
    and extract additional feature information for your classifier.
    However, your method will only be work inside basf2 and not with basf2_mva_expert in this case
    (and also you cannot use basf2_mva_evaluate.py).
    """
    p = state.predict_proba(X)
    return np.require(p, dtype=np.float32, requirements=['A', 'W', 'C', 'O'])


if __name__ == "__main__":
    """
    Here we create the fake training
    We written all the necessary hooks, now we can call the mva framework as usual.
    Other Python-based frameworks like sklearn, tensorflow, xgboost, ... have predefined hooks,
    but you can overwrite all of them.
    """
    from basf2 import conditions
    # NOTE: do not use testing payloads in production! Any results obtained like this WILL NOT BE PUBLISHED
    conditions.testing_payloads = [
        'localdb/database.txt'
    ]

    # First I create an external sklearn weightfile
    from sklearn.ensemble import GradientBoostingClassifier
    clf = GradientBoostingClassifier()
    X = np.random.uniform(size=(1000, 3))
    y = (np.random.uniform(size=1000) > 0.5).astype(int)
    clf.fit(X, y)
    import pickle
    pickle.dump(clf, open('mytraining.pickle', 'wb'))

    variables = ['M', 'p', 'pt']

    # Now we need also a fake input file, we just create one
    # the content doesn't matter, as long as the branches exist.
    import ROOT
    root_file = ROOT.TFile("fake_train.root", "recreate")
    root_file.cd()
    root_tree = ROOT.TTree('tree', 'title')
    value = np.zeros(1, dtype=float)
    for var in variables:
        root_tree.Branch(var, value, var + '/F')
    root_tree.Branch("isSignal", value, 'isSignal/F')
    for i in range(10):
        root_tree.Fill()
    root_file.Write("tree")
    root_file.Close()

    # Now we write the configuration for our fake training
    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("fake_train.root")
    general_options.m_treename = "tree"
    general_options.m_identifier = "MyModel"
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "isSignal"

    python_options = basf2_mva.PythonOptions()
    python_options.m_framework = "custom"
    python_options.m_steering_file = "mva/examples/python/howto_wrap_your_existing_training_into_a_weightfile.py"

    basf2_mva.teacher(general_options, python_options)

    # Apply the training as usual
    method = basf2_mva_util.Method(general_options.m_identifier)
    p, t = method.apply_expert(basf2_mva.vector("test.root"), general_options.m_treename)
    auc = basf2_mva_util.calculate_roc_auc(p, t)
    print("Custom Method", auc)
