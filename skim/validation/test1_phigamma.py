#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Skim for ISR phi gamma events, with phi-->K+K- or KS(pi+pi-)KL
# G. Finocchiaro 2020
#
#######################################################


import basf2 as b2
import modularAnalysis as ma
from skim.systematics import SystematicsPhiGamma

path = b2.Path()

fileList = ["./phigamma_neutral.dst.root"]
# fileList = ["/group/belle2/users/fnc/MC/phi_gamma_*_prod/phi_gamma_*_prod_mdst_0000?.root"]
ma.inputMdstList("default", fileList, path=path)

skim = SystematicsPhiGamma(OutputFileName="./phigamma_neutral_skimmed")
# skim = SystematicsPhiGamma(OutputFileName="./phigamma_MC_charged_neutral_skimmed")
skim(path)
b2.process(path)
