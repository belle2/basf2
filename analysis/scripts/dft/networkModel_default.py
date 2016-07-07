#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Jochen Gemmler 2016
# based on Baldi, P., P. Sadowski, and D. Whiteson. “Searching for Exotic Particles in High-energy
# Physics with Deep Learning.” Nature Communications 5 (July 2, 2014).
# https://github.com/uci-igb/higgs-susy

import pylearn2
import pylearn2.training_algorithms.sgd
import pylearn2.models.mlp as mlp

import pylearn2.train
import pylearn2.termination_criteria
import pylearn2.training_algorithms.learning_rule

import pylearn2.train_extensions.best_params

import numpy as np


def get_default_params():
    """
    :return: dictionary of default parameters
    """
    default_params = {'save_path': 'mlp_standard.pkl', 'width': None, 'nhid': 4,
                      'termination_step': 10, 'momentum_saturate': 200, 'learning_rate': .05, 'weight_decay': None,
                      'init_momentum': .9, 'feature_number': 70, 'seed': None, 'activation': 'tanh',
                      'dataset_train_monitor': None, 'dataset_test_monitor': None, 'weight_init': 'std', 'nesterov': False,
                      'min_lr': .000001, 'batch_size': 100, 'lr_dec': 1.0000002}
    return default_params


def get_not_json_params():
    """
    :return: parameter names that should not be saved in a json dictionary
    """
    return ['dataset_train_monitor', 'dataset_test_monitor']


def init_train(dataset_train, dataset_valid, save_path='mlp_standard.pkl', width=None, nhid=4,
               termination_step=10, momentum_saturate=200, learning_rate=.05, weight_decay=None,
               init_momentum=.9, feature_number=70, seed=None, activation='tanh',
               dataset_train_monitor=None, dataset_test_monitor=None, weight_init='std', nesterov=False,
               min_lr=.000001, batch_size=100, lr_dec=1.0000002):
    """ returns a pylearn2.train.Train instance with initialized value
    start training with return_value.main_loop()
    creates a json file with {save_path}_mparams.json

    :param dataset_train:
    :param dataset_valid:
    :param save_path:
    :param width:
    :param nhid:
    :param termination_step:
    :param momentum_saturate:
    :param learning_rate:
    :param weight_decay:
    :param init_momentum:
    :param feature_number:
    :param seed:
    :param activation:
    :param dataset_train_monitor:
    :param dataset_test_monitor:
    :param weight_init:
    :param nesterov:
    :return:
    """
    activation_dict = {'tanh': mlp.Tanh}
    activation = activation_dict[activation]

    if width is None:
        width = [300 for _ in range(nhid)]
    else:
        assert len(width) == nhid

    # check if weight decay has the length of nhid + 1, input form [hidden_layers, ..., output_layer]
    if weight_decay is None:
        weight_decay = [.00001 for _ in range(nhid + 1)]
    else:
        assert len(weight_decay) == nhid + 1

    if seed is None:
        seed = np.random.randint(0, 9999999)

    layers = []

    if weight_init is 'std':
        width = np.average(width)
        istdev = 1.0 / np.sqrt(width)
        for i in range(nhid):

            layer = activation(layer_name='h%d' % i, dim=width,
                               istdev=(istdev if i > 0 else 2 * istdev))
            layers.append(layer)

        layers.append(mlp.Sigmoid(layer_name='y', dim=1, istdev=0.02 * istdev))

    elif weight_init is 'uniform':
        istdev = [np.sqrt(6) / np.sqrt(feature_number + width[0])]
        for i in range(len(width) - 1):
            istdev.append(np.sqrt(6) / np.sqrt(width[i] + width[i + 1]))

        istdev.append(np.sqrt(6) / np.sqrt(width[-1] + 1))

        for i in range(nhid):
            layer = activation(layer_name='h%d' % i, dim=width[i], irange=istdev[i])
            layers.append(layer)

        # since this is sigmoid we get 4* tanh_factor
        layers.append(mlp.Sigmoid(layer_name='y', dim=1, irange=istdev[-1]))
    else:
        raise NotImplementedError('Weight intialization type %s' % weight_init)

    model = pylearn2.models.mlp.MLP(layers, nvis=feature_number, seed=seed)

    # Algorithm
    learning_rule = pylearn2.training_algorithms.learning_rule.Momentum(init_momentum, nesterov_momentum=nesterov)

    monitoring_set = {'valid': dataset_valid}
    if dataset_train_monitor is not None:
        monitoring_set['train'] = dataset_train_monitor
    if dataset_test_monitor is not None:
        monitoring_set['test'] = dataset_test_monitor

    algorithm = pylearn2.training_algorithms.sgd.SGD(
        batch_size=batch_size,
        learning_rate=learning_rate,
        learning_rule=learning_rule,
        monitoring_dataset=monitoring_set,
        # termination_criterion=pylearn2.termination_criteria.Or(
        #    criteria=[
        #        pylearn2.termination_criteria.MonitorBased(
        #            channel_name="valid_objective",
        #            prop_decrease=.00001,
        #            N=termination_step),
        #        pylearn2.termination_criteria.EpochCounter(max_epochs=momentum_saturate)
        #    ]),
        termination_criterion=pylearn2.termination_criteria.EpochCounter(2),
        cost=pylearn2.costs.cost.SumOfCosts(
            costs=[
                pylearn2.costs.mlp.Default(),
                pylearn2.costs.mlp.WeightDecay(coeffs=weight_decay)
            ]
        ),

        update_callbacks=pylearn2.training_algorithms.sgd.ExponentialDecay(
            decay_factor=lr_dec,
            min_lr=min_lr
        )
    )

    extensions = [
        pylearn2.training_algorithms.learning_rule.MomentumAdjustor(
            start=0,
            saturate=momentum_saturate,
            final_momentum=.99
        ),
        pylearn2.train_extensions.best_params.MonitorBasedSaveBest(channel_name='valid_objective', save_path=save_path),
    ]

    train = pylearn2.train.Train(
        dataset=dataset_train,
        model=model,
        algorithm=algorithm,
        extensions=extensions,
        save_path=save_path,
        save_freq=50
    )

    return train


def train(train_obj):
    return train_obj.main_loop()
