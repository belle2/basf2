#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2016

import basf2_mva

if __name__ == "__main__":

    # Train a MVA method and directly upload it to the database
    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafile = "train.root"
    general_options.m_treename = "tree"
    general_options.m_weightfile = "MVADatabaseIdentifier"
    general_options.m_variables = basf2_mva.vector('p', 'pz', 'daughter(0, Kid)')
    general_options.m_target_variable = "isSignal"

    fastbdt_options = basf2_mva.FastBDTOptions()

    basf2_mva.teacher(general_options, fastbdt_options)

    # Download the weightfile from the database and store it on disk in a root file
    basf2_mva.download('MVADatabaseIdentifier', 'weightfile.root')

    # Train a MVA method and store the weightfile on disk in a root file
    general_options.m_weightfile = "weightfile2.root"
    basf2_mva.teacher(general_options, fastbdt_options)

    # Upload the weightfile on disk to the database
    basf2_mva.upload('weightfile2.root', 'MVADatabaseIdentifier2')

    # Apply the trained methods on data
    basf2_mva.expert(basf2_mva.vector('weightfile.root', 'weightfile2.root', 'MVADatabaseIdentifier', 'MVADatabaseIdentifier2'),
                     'train.root', 'tree', 'expert.root')
