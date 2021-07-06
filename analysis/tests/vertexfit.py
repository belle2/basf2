#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import tempfile
import basf2
import b2test_utils
import modularAnalysis as ma
from vertex import kFit
from ROOT import TFile


def check(filename):
    """
    Verify results make sense.
    """
    ntuplefile = TFile(filename)
    ntuple = ntuplefile.Get('ntuple')

    if ntuple.GetEntries() == 0:
        basf2.B2FATAL("No D0s saved")

    if ntuple.GetEntries("significanceOfDistance < 0 && significanceOfDistance != -1") > 0:
        basf2.B2FATAL("Some entries have negative significanceOfDistance?")

    if ntuple.GetEntries("significanceOfDistance > 0") == 0:
        basf2.B2FATAL("significanceOfDistance never positive?")


testFile = tempfile.NamedTemporaryFile()

main = basf2.create_path()

ma.inputMdst('default', b2test_utils.require_file('analysis/tests/mdst.root'), path=main)

ma.fillParticleList('K-', '', path=main)
ma.fillParticleList('pi+', '', path=main)

ma.fillParticleList('gamma', '', path=main)
ma.reconstructDecay('pi0 -> gamma gamma', '0.11 < M < 0.15', 0, path=main)
ma.matchMCTruth('pi0', path=main)

# KFit
kFit('pi0', 0.0, path=main)

ma.reconstructDecay('D0 -> K- pi+ pi0', '', 0, path=main)
ma.matchMCTruth('D0', path=main)

kFit('D0', 0.0, decay_string='D0 -> ^K- ^pi+ pi0', path=main)

ntupler = basf2.register_module('VariablesToNtuple')
ntupler.param('fileName', testFile.name)
ntupler.param('variables', ['M', 'isSignal', 'distance', 'dr', 'dz', 'significanceOfDistance', 'pValue'])
ntupler.param('particleList', 'D0')
main.add_module(ntupler)

with b2test_utils.clean_working_directory():
    b2test_utils.safe_process(main)
    check(testFile.name)
