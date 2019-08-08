#!/usr/bin/env python3
# -*- coding: utf-8 -*-

################################################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial runs over flat NTuples of reconstructed B->KsPi0 decays,
# which were created running B2A711. The training and test datasets consist of
# a mixture of reconstructed Bd->KsPi0 and qqbqr MC. The apply datasets are
# pure signal and qqbar for running the expert as explained below.
# The actual training model is described in B2A714.
# Also have a look at https://confluence.desy.de/display/BI/Continuum+Suppression+Framework
# The techniques are described in more detail in http://ekp-invenio.physik.uni-karlsruhe.de/record/48934
#
# Usage:
#   basf2 B2A702-ContinuumSuppression_MVATrain.py
#
# Contributors: D. Weyland (November 2017), P. Goldenzweig (October 2016)
#
################################################################################

import basf2_mva
import subprocess
import json


def choose_input_features(use_vertex_features=True, use_charge_and_ROE_features=False, use_continuum_features=1):
    """
    Function to return all names of input features.
    :param use_vertex_features: If Vertex info should be included.
    :param use_charge_and_ROE_features: If charge and ROE should be included as extra features(information already
        included in group structure). This option is only nevessary when using Relation Layers.
    :param use_continuum_features: Use old Continuum Features (0: No, 1: Yes, 2: Use only the old features)
    :return: Array of feature names
    """
    contVar = [
        'R2',
        'thrustBm',
        'thrustOm',
        'cosTBTO',
        'cosTBz',
        'KSFWVariables(et)',
        'KSFWVariables(mm2)',
        'KSFWVariables(hso00)',
        'KSFWVariables(hso02)',
        'KSFWVariables(hso04)',
        'KSFWVariables(hso10)',
        'KSFWVariables(hso12)',
        'KSFWVariables(hso14)',
        'KSFWVariables(hso20)',
        'KSFWVariables(hso22)',
        'KSFWVariables(hso24)',
        'KSFWVariables(hoo0)',
        'KSFWVariables(hoo1)',
        'KSFWVariables(hoo2)',
        'KSFWVariables(hoo3)',
        'KSFWVariables(hoo4)',
        'CleoCone(1)',
        'CleoCone(2)',
        'CleoCone(3)',
        'CleoCone(4)',
        'CleoCone(5)',
        'CleoCone(6)',
        'CleoCone(7)',
        'CleoCone(8)',
        'CleoCone(9)']

    if use_continuum_features == 2:
        return contVar

    basic_variables = ['p', 'phi', 'cosTheta', 'pErr', 'phiErr', 'cosThetaErr']
    vertex_variables = ['distance', 'dphi', 'dcosTheta']

    cluster_specific_variables = ['clusterNHits', 'clusterTiming', 'clusterE9E25', 'clusterReg']
    track_specific_variables = ['kaonID', 'electronID', 'muonID', 'protonID', 'pValue', 'nCDCHits']

    if use_charge_and_ROE_features:
        cluster_specific_variables += ['isInRestOfEvent']
        track_specific_variables += ['isInRestOfEvent', 'charge']

    cluster_specific_variables += ['thrustsig' + var for var in basic_variables]
    track_specific_variables += ['thrustsig' + var for var in basic_variables]

    if use_vertex_features:
        track_specific_variables += ['thrustsig' + var for var in vertex_variables]

    cluster_lists = ['Csig', 'Croe']
    track_lists = ['TPsig', 'TMsig', 'TProe', 'TMroe']

    variables = []
    for plist in track_lists:
        for rank in range(5):
            for var in track_specific_variables:
                variables.append('{}_{}{}'.format(var, plist, rank))

    for plist in cluster_lists:
        for rank in range(10):
            for var in cluster_specific_variables:
                variables.append('{}_{}{}'.format(var, plist, rank))

    if use_continuum_features:
        variables += contVar

    return variables


if __name__ == "__main__":

    # In this path there are already several trained weightfiles. Look at README for a short explanation
    path = '/group/belle2/tutorial/release_01-00-00/inputForDCSTutorial/'

    train_data = path + 'train.root'
    test_data = path + 'test.root'

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector(train_data)
    general_options.m_treename = "tree"
    general_options.m_identifier = "Deep_Feed_Forward.xml"
    general_options.m_variables = basf2_mva.vector(*choose_input_features(True, False, 1))
    general_options.m_spectators = basf2_mva.vector('Mbc', 'DeltaZ')
    general_options.m_target_variable = "isNotContinuumEvent"

    specific_options = basf2_mva.PythonOptions()
    specific_options.m_framework = "contrib_keras"
    specific_options.m_steering_file = 'analysis/examples/tutorials/B2A714-DeepContinuumSuppression_MVAModel.py'
    specific_options.m_training_fraction = 0.9

    # These options are custom made in B2A714. You can also add your own parameters.
    # Try different options and compare them by handing multiple weightfiles in basf2_mva_evaluation.py
    keras_dic = {
        # If Relation layer should be used instead of Feed Forward.
        # Only works with choose_input_features(True, True, 1)
        'use_relation_layers': False,
        # The following options are for using Adversaries. To disable them leave lambda to zero.
        # See mva/examples/keras/adversary_network.py for details
        'lambda': 0,  # Use 500 as starting point to try the Adversaries out
        'number_bins': 10,
        'adversary_steps': 5}
    specific_options.m_config = json.dumps(keras_dic)

    # Train a MVA method and store the weightfile (MVAFastBDT.root) locally.
    basf2_mva.teacher(general_options, specific_options)

    # Evaluate training.
    subprocess.call('basf2_mva_evaluate.py '
                    ' -train ' + train_data +
                    ' -data ' + test_data +
                    ' -id ' + 'Deep_Feed_Forward.xml' +
                    ' --output qqbarSuppressionEvaluation.pdf',
                    shell=True
                    )

    # If you're only interested in the network output distribution, then
    # comment these in to apply the trained methods on train and test sample
    #
    # basf2_mva.expert(basf2_mva.vector('Deep_Feed_Forward.xml'), basf2_mva.vector(train_data), 'tree', 'MVAExpert_train.root')
    # basf2_mva.expert(basf2_mva.vector('Deep_Feed_Forward.xml'), basf2_mva.vector(test_data), 'tree', 'MVAExpert_test.root')
