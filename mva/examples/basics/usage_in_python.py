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
    from basf2 import conditions
    import ROOT  # noqa
    # NOTE: do not use testing payloads in production! Any results obtained like this WILL NOT BE PUBLISHED
    conditions.testing_payloads = [
        'localdb/database.txt'
    ]

    # Train a MVA method and directly upload it to the database
    general_options = ROOT.Belle2.MVA.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
    general_options.m_treename = "tree"
    general_options.m_identifier = "MVADatabaseIdentifier"
    general_options.m_variables = basf2_mva.vector('M', 'p', 'pz', 'pt', 'phi', 'daughter(0, kaonID)', 'daughter(0, pionID)',
                                                   'daughter(1, kaonID)', 'daughter(1, pionID)', 'chiProb', 'dr', 'dz', 'dphi')
    general_options.m_target_variable = "isSignal"

    fastbdt_options = ROOT.Belle2.MVA.FastBDTOptions()

    ROOT.Belle2.MVA.teacher(general_options, fastbdt_options)

    # Download the weightfile from the database and store it on disk in a root file
    ROOT.Belle2.MVA.download('MVADatabaseIdentifier', 'weightfile.root')

    # Train a MVA method and store the weightfile on disk in a root file
    general_options.m_identifier = "weightfile2.root"
    ROOT.Belle2.MVA.teacher(general_options, fastbdt_options)

    # Upload the weightfile on disk to the database
    ROOT.Belle2.MVA.upload('weightfile2.root', 'MVADatabaseIdentifier2')

    # Apply the trained methods on data
    ROOT.Belle2.MVA.expert(
        basf2_mva.vector(
            'weightfile.root',
            'weightfile2.root',
            'MVADatabaseIdentifier',
            'MVADatabaseIdentifier2'),
        basf2_mva.vector('train.root'),
        'tree',
        'expert.root')
