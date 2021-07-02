#!/usr/bin/env python3
# -*- coding: utf-8 -*

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

try:
    import theano
    import theano.tensor
except ImportError:
    print("Please install theano: pip3 install theano")
    import sys
    sys.exit(1)

import numpy


class State(object):
    """
    State class for proper handling of parameters and data during function calls. This is a very brief theano example.
    """

    def __init__(self, x=None, y=None, params=None, cost=None, updates=None, train_function=None, eval_function=None):
        """
        Constructor of the State class
        """
        # TODO: make serializable with __getstate__(), __setstate__()
        #: theano shared variable x
        self.x = x
        #: theano shared variable y
        self.y = y

        #: model params
        self.params = params
        #: model cost
        self.cost = cost

        #: model grad updates
        self.updates = updates

        #: theano function for training
        self.train_function = train_function
        #: theano function for evaluation
        self.eval_function = eval_function


def get_model(number_of_features, number_of_spectators, number_of_events, training_fraction, parameters):

    x = theano.tensor.matrix('x')
    y = theano.tensor.vector('y', dtype='float32')

    # learning_rate = parameters.get('learning_rate', 0.1)
    learning_rate = 0.1

    n_in = number_of_features
    n_out = 1
    rng = numpy.random.RandomState(1234)
    w_values = numpy.asarray(
        rng.uniform(
            low=-numpy.sqrt(6. / (n_in + n_out)),
            high=numpy.sqrt(6. / (n_in + n_out)),
            size=(n_in, n_out)
        ),
        dtype=theano.config.floatX
    )

    w_values *= 4
    w = theano.shared(value=w_values, name='W', borrow=True)

    b_values = numpy.zeros((n_out,), dtype=theano.config.floatX)
    b = theano.shared(value=b_values, name='b', borrow=True)

    activation = theano.tensor.nnet.sigmoid

    output = activation(theano.tensor.dot(x, w) + b)

    cost = theano.tensor.nnet.binary_crossentropy(output.T, y).mean()

    params = [w, b]

    grad_params = [theano.tensor.grad(cost, param) for param in params]

    updates = [(param, param - learning_rate * gparam) for param, gparam in zip(params, grad_params)]

    train_function = theano.function(
        inputs=[x, y],
        outputs=cost,
        updates=updates
    )

    eval_function = theano.function(
        inputs=[x],
        outputs=output
    )

    return State(x, y, params, cost, updates, train_function, eval_function)


def feature_importance(state):
    """
    Return a list containing the feature importances
    """
    return []


def load(obj):
    state = State(eval_function=obj[0])
    return state


def apply(state, X):
    result = state.eval_function(X)
    return numpy.require(result, dtype=numpy.float32, requirements=['A', 'W', 'C', 'O'])


def begin_fit(state, Xvalid, Svalid, yvalid, wvalid):
    return state


def partial_fit(state, X, S, y, w, epoch):
    avg_cost = state.train_function(X, y) / len(y)
    print("Epoch:", '%04d' % (epoch), "cost=", "{:.9f}".format(avg_cost))
    return True


def end_fit(state):
    # FIXME: this might not work as intended
    # a better method can be found at: http://deeplearning.net/software/theano/tutorial/loading_and_saving.html
    return [state.eval_function]
