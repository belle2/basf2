#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2017

# The MVA package does support arbitrary python-based mva frameworks.
# You just have to:
# Install them (e.g. via pip3)
# Provide all necessary hook functions (see below)

import numpy as np


class MyFancyClassifier(object):
    """ Let's assume we have writte our own classifier (or installed something from github) """

    def __init__(self, *my_fancy_parameters):
        """ Just print the passed parameters """
        print(my_fancy_parameters)

    def fit(self, X, y):
        """ Our method is so good, it doesn't even have to look at the data! """
        return self

    def predict(self, X):
        """ Always return 1, this will boost our signal efficiency to the max """
        return np.ones(len(X))


import basf2_mva
import basf2_mva_util


# These are the hooks you should implement


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):
    """
    This is the first function which is called.
    It must return a python object representing your method in memory, this object will be passed to all other hook functions.
    In this case we return our FancyClassifier object.
    @param number_of_features the total number of features
    @param number_of_spectators the total number of spectators
    @param number_of_events the total number of events
    @param training_fraction the signal fraction in the training (if you do a classification, otherwise the number is meaningless)
    @param parameters a python object which is created from a json string the user can pass via the m_config argument
    """
    return MyFancyClassifier(parameters)


def begin_fit(state, Xtest, Stest, ytest, wtest):
    """
    Is called once per training after get_model.
    You can initialize your training here.
    In addition a validation sample is passed, which you can use during the training (if the user set m_training_fraction != 1.0)
    @param state the return value of get_model
    @param Xtest numpy array containing the features of the validation sample
    @param Stest numpy array containing the spectators of the validation sample
    @param ytest numpy array containing the target values of the validation sample
    @param wtest numpy array containing the weights of the validation sample

    Since our method does not support out-of-core fitting, the usual thing is to add
    some arrays which collect the data passed to partial_fit.
    Our method doesn't use the validation sample either, so we just don't use it.
    """
    state.X = []
    state.y = []
    return state


def partial_fit(state, X, S, y, w, epoch):
    """
    Can be called multiple times per training depending on the user configuration:
    If m_nIterations == 1 and m_mini_batch_size == 0 (these are the default values)
        partial_fit is called once with the complete training data
    If m_nIterations == 1 and m_mini_batch_size != 0
        partial_fit is called multiple times with only a subset of the training data of the desired size,
        until the complete dataset was streamed via partial_fit
    If m_nIterations > 1 and m_mini_batch_size == 0
        partial_fit is called multiple times, each time with the complete training data
    If m_nIterations > 1 and m_mini_batch_size != 0
        partial_fit is called multiple times with only a subset of the trianing data of the desired size,
        until the complete dataset was streamed m_nIterations times
    If m_nIterations == 0
        partial_fit is called multiple times until partial_fit returns False
    As soon as partial_fit returns False the streaming of data is stopped.
    @param state the return value of begin_fit
    @param X numpy array containing the features of the training sample
    @param S numpy array containing the spectators of the training sample
    @param y numpy array containing the target values of the training sample
    @param w numpy array containing the weights of the training sample
    @param epoch the total number of previous calls to partial_fit

    Since our method doesn't use the streaming capability,
    we just collect the data in our state object.
    """
    state.X.append(X)
    state.y.append(y)
    return True


def end_fit(state):
    """
    Is called once per training.
    Here you can finish the training.
    You must return a pickable object, which is saved in the weightfile,
    later you must be able to create your estimator from this pickled object in the load function hook (see below).
    @param state the return value of begin_fit

    We can fit our method here. And since our state object is pickable,
    we can just return it. You might want to use better mechanism in a real world example,
    you can look at the implementations of the other methods (like tensorflow) howto save models
    to files, read them and return them as a pickable object.
    """
    state.fit(state.X, state.y)
    pickable_object_for_weightfile = state
    return pickable_object_for_weightfile


def feature_importance(state):
    """
    Called after end_fit.
    Should return a list containing the feature importances.
    The feature importances are saved in the weightfile and can be read out by the user.
    If your method doesn't support feature importance estimation return an empty list.
    """
    return []


def load(pickable_object_from_weightfile):
    """
    Is called once.
    @param obj the return value of end_fit, which was loaded from the weightfile and unpickled
    This should return again a state object, which is passed to apply later.

    In our case we directly pickled the state, so there's nothing to do here.
    In a real world scenario you might have to create files on disk in a temporary directory
    and recreate your estimator from them. You can look at other methods (like tensorflow) how this is done.
    """
    state = pickable_object_from_weightfile
    return state


def apply(state, X):
    """
    Is called once per inference.
    Should return a numpy array with the predicted values.
    You have to make sure that the numpy array has the correct format (32bit float, C-style ordering)!
    The last line in this function takes care of this, I strongly recommend to keep this line!
    @param state the return value of load
    @param X numpy array containing the features for which a prediction should be returned
    """
    p = state.predict(X)
    return np.require(p, dtype=np.float32, requirements=['A', 'W', 'C', 'O'])


if __name__ == "__main__":
    """
    We written all the necessary hooks, now we can call the mva framework as usual.
    Other Python-based frameworks like sklearn, tensorflow, xgboost, ... have predefined hooks,
    but you can overwrite all of them.
    """

    # Create The GeneralOptions object as always
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

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
    general_options.m_treename = "tree"
    general_options.m_identifier = "MyFancyModel"
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "isSignal"

    # With the PythonOptions you can configure some details how the hook functions are called
    # I describe here every option, but there are reasonable defaults, so usually you only
    # have to set m_framework and m_steering_file
    python_options = basf2_mva.PythonOptions()

    # You have to use "custom" as framework,
    # this will raise a RuntimeError if you forgot to implement any of the hooks
    python_options.m_framework = "custom"

    # The path to the file were you implemented all the hooks,
    # in this case this is the same file were we setup the training itself,
    # but in principle it can be any file, this file will be saved in the weightfile
    # and it will be executed as soon as the weightfile is loaded! (so the above if __name__ == "__main__" is very important)
    python_options.m_steering_file = "mva/examples/python/howto_use_arbitrary_methods.py"

    # You can pass parameters to your get_model hook, in form of a json string
    # You can use json.dumps to find out the right syntax.
    # For example if you want to pass a dictionary with some parameters
    import json
    config_string = json.dumps({'A': 'Python', 'Dictionary': 'With Parameters', 'And': ['A List']})
    print("The json config string", config_string)
    python_options.m_config = config_string

    # You can spit the dataset into a training sample (passed to partial_fit) and a validation sample (passed to begin_fit)
    # Here we use 70% for training and 30% as validation default is 1.0
    python_options.m_training_fraction = 0.7

    # You can normalize the input features before passing them to begin_fit, partial_fit and apply.
    # The normalization is calculated once and saved in the weightfile.
    # Every feature is shifted to mean 0 and a standard deviation of 1
    python_options.m_normalize = False

    # As described in partial_fit, the mva package can stream the data to your method.
    # The following to parameters control the streaming.
    # If you just want the full dataset at once use the following values (which are the default values)
    python_options.m_nIterations = 1
    python_options.m_mini_batch_size = 0

    # Now you can train as usual
    # Of course you can also use the command line command basf2_mva_teacher to do so
    basf2_mva.teacher(general_options, python_options)

    # To validate your method it is convenient to use basf2_mva_util to load a trained method
    method = basf2_mva_util.Method(general_options.m_identifier)

    # Because then it is very easy to apply the method to a test file,
    # of course you can also apply the method using the MVAExpert module directly in basf2
    # Or (if you do reconstruction and not analysis) the corresponding modules.
    p, t = method.apply_expert(basf2_mva.vector("test.root"), general_options.m_treename)

    # We calculate the AUC ROC value of the returned probability and target,
    # our method is very simple, so the AUC won't be good :-)
    auc = basf2_mva_util.calculate_roc_auc(p, t)
    print("Custom Method", auc)
