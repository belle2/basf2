#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2017

import basf2_mva
from basf2 import *
from modularAnalysis import *

if __name__ == "__main__":

    variables = ['p', 'pt', 'pz', 'phi',
                 'daughter(0, p)', 'daughter(0, pz)', 'daughter(0, pt)', 'daughter(0, phi)',
                 'daughter(1, p)', 'daughter(1, pz)', 'daughter(1, pt)', 'daughter(1, phi)',
                 'daughter(2, p)', 'daughter(2, pz)', 'daughter(2, pt)', 'daughter(2, phi)',
                 'chiProb', 'dr', 'dz', 'dphi',
                 'daughter(0, dr)', 'daughter(1, dr)', 'daughter(0, dz)', 'daughter(1, dz)',
                 'daughter(0, dphi)', 'daughter(1, dphi)',
                 'daughter(0, chiProb)', 'daughter(1, chiProb)', 'daughter(2, chiProb)', 'daughter(2, M)',
                 'daughter(0, atcPIDBelle(3,2))', 'daughter(1, atcPIDBelle(3,2))',
                 'daughterAngle(0, 1)', 'daughterAngle(0, 2)', 'daughterAngle(1, 2)',
                 'daughter(2, daughter(0, E))', 'daughter(2, daughter(1, E))',
                 'daughter(2, daughter(0, clusterLAT))', 'daughter(2, daughter(1, clusterLAT))',
                 'daughter(2, daughter(0, clusterHighestE))', 'daughter(2, daughter(1, clusterHighestE))',
                 'daughter(2, daughter(0, clusterNHits))', 'daughter(2, daughter(1, clusterNHits))',
                 'daughter(2, daughter(0, clusterE9E25))', 'daughter(2, daughter(1, clusterE9E25))',
                 'daughter(2, daughter(0, minC2HDist))', 'daughter(2, daughter(1, minC2HDist))',
                 'daughterInvariantMass(1, 2)', 'daughterInvariantMass(0, 1)', 'daughterInvariantMass(0, 2)'
                 ]

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train_mc.root")
    general_options.m_identifier = "MVABaseline"
    general_options.m_treename = "tree"
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "isSignal"

    fastbdt_options = basf2_mva.FastBDTOptions()
    basf2_mva.teacher(general_options, fastbdt_options)
