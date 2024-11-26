#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2_mva

if __name__ == "__main__":
    from basf2 import conditions, find_file
    # NOTE: do not use testing payloads in production! Any results obtained like this WILL NOT BE PUBLISHED
    conditions.testing_payloads = [
        'localdb/database.txt'
    ]

    train_file = find_file("mva/train_D0toKpipi.root", "examples")
    test_file = find_file("mva/test_D0toKpipi.root", "examples")

    training_data = basf2_mva.vector(train_file)
    testing_data = basf2_mva.vector(test_file)

    # Train a MVA method and directly upload it to the database
    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = training_data
    general_options.m_treename = "tree"
    general_options.m_identifier = "MVADatabaseIdentifier"
    general_options.m_variables = basf2_mva.vector('M', 'p', 'pz', 'pt', 'phi', 'daughter(0, kaonID)', 'daughter(0, pionID)',
                                                   'daughter(1, kaonID)', 'daughter(1, pionID)', 'chiProb', 'dr', 'dz', 'dphi')
    general_options.m_target_variable = "isSignal"

    fastbdt_options = basf2_mva.FastBDTOptions()

    basf2_mva.teacher(general_options, fastbdt_options)

    # Download the weightfile from the database and store it on disk in a root file
    basf2_mva.download('MVADatabaseIdentifier', 'weightfile.root')

    # Train a MVA method and store the weightfile on disk in a root file
    general_options.m_identifier = "weightfile2.root"
    basf2_mva.teacher(general_options, fastbdt_options)

    # Upload the weightfile on disk to the database
    basf2_mva.upload('weightfile2.root', 'MVADatabaseIdentifier2')

    # Apply the trained methods on data
    basf2_mva.expert(
        basf2_mva.vector(
            'weightfile.root',
            'weightfile2.root',
            'MVADatabaseIdentifier',
            'MVADatabaseIdentifier2'),
        testing_data,
        'tree',
        'expert.root')
