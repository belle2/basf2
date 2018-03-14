#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2016

# The mva package has a builtin reweighting mechanism.
# If your data and mc do not match, you can use the MetaOptions to do a meta-training.
# Using 'm_use_reweighting = True' the mva package will train
#  - the provided data files against the provided mc files, and calculate the probability p for each event in the mc files
#  - the provided signal against background in the mc files weighted with the p / (1 - p)
# You can access the first boost training, it is saved with the postfix _boost.xml

# In general you want to write out
#  - train_mc.root containing the reconstructed candidates from MC (Y4S + Continuum)
#  - train_data.root containing the reconstructed candidates from data

# In this example we investigate the decay D -> K pi pi0 in MC and data (e.g. using b2bii)
# It turns out that mostly the continuum description is wrong, so we only correct for this
#  - train_mc_continuum contains only candidates from charm and uds
#  - train_data_continuum contains off resonance data with only continuum
# In the end we also only want to apply the reweighting to continuum events, so we set the
# m_reweighting_variable to 'isContinuumEvent'

import basf2_mva
from basf2 import *
from modularAnalysis import *

if __name__ == "__main__":

    variables = ['p', 'pt', 'pz', 'phi',
                 'chiProb', 'dr', 'dz', 'dphi',
                 'daughter(0, dr)', 'daughter(1, dr)', 'daughter(0, dz)', 'daughter(1, dz)',
                 'daughter(0, dphi)', 'daughter(1, dphi)',
                 'daughter(0, chiProb)', 'daughter(1, chiProb)', 'daughter(2, chiProb)', 'daughter(2, M)',
                 'daughter(0, atcPIDBelle(3,2))', 'daughter(1, atcPIDBelle(3,2))',
                 'daughter(2, daughter(0, E))', 'daughter(2, daughter(1, E))',
                 'daughter(2, daughter(0, clusterLAT))', 'daughter(2, daughter(1, clusterLAT))',
                 'daughter(2, daughter(0, clusterHighestE))', 'daughter(2, daughter(1, clusterHighestE))',
                 'daughter(2, daughter(0, clusterNHits))', 'daughter(2, daughter(1, clusterNHits))',
                 'daughter(2, daughter(0, clusterE9E25))', 'daughter(2, daughter(1, clusterE9E25))',
                 'daughter(2, daughter(0, minC2HDist))', 'daughter(2, daughter(1, minC2HDist))',
                 'daughterInvariantMass(1, 2)', 'daughterInvariantMass(0, 1)', 'daughterInvariantMass(0, 2)'
                 'daughterAngle(0, 1)', 'daughterAngle(0, 2)', 'daughterAngle(1, 2)',
                 'daughter(0, p)', 'daughter(0, pz)', 'daughter(0, pt)', 'daughter(0, phi)',
                 'daughter(1, p)', 'daughter(1, pz)', 'daughter(1, pt)', 'daughter(1, phi)',
                 'daughter(2, p)', 'daughter(2, pz)', 'daughter(2, pt)', 'daughter(2, phi)',
                 ]

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train_mc.root")
    general_options.m_identifier = "MVAReweighted"
    general_options.m_treename = "tree"
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "isSignal"

    fastbdt_options = basf2_mva.FastBDTOptions()

    meta_options = basf2_mva.MetaOptions()
    meta_options.m_use_reweighting = True
    meta_options.m_reweighting_variable = 'isContinuumEvent'
    meta_options.m_reweighting_identifier = "Reweighter"
    meta_options.m_reweighting_mc_files = basf2_mva.vector("train_mc_continuum.root")
    meta_options.m_reweighting_data_files = basf2_mva.vector("train_data_continuum.root")

    basf2_mva.teacher(general_options, fastbdt_options, meta_options)
