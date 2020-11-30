#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Skim for ISR phi gamma events, with phi-->K+K- or KS(pi+pi-)KL
# G. Finocchiaro 2020
#
#######################################################

"""
<header>
    <input>../phigamma_neutral.dst.root</input>
    <output>../phigamma_neutral_skimmed.udst.root</output>
    <contact>giuseppe.finocchiaro@lnf.infn.it</contact>
</header>
"""

import basf2 as b2
import modularAnalysis as ma
from skim.systematics import SystematicsPhiGamma

path = b2.Path()

fileList = ["../phigamma_neutral.dst.root"]
ma.inputMdstList("default", fileList, path=path)

skim = SystematicsPhiGamma(OutputFileName="../phigamma_neutral_skimmed.udst.root")
skim(path)
b2.process(path)
