#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import set_random_seed, create_path, register_module, process, find_file
from b2test_utils import clean_working_directory, require_file, configure_logging_for_tests
import modularAnalysis as ma
import subprocess


configure_logging_for_tests()
set_random_seed('pid_priors')

with clean_working_directory():

    main = create_path()

    inputfile = require_file('analysis/1000_B_DstD0Kpi_skimmed.root', 'validation')
    ma.inputMdst(inputfile, path=main)

    ma.fillParticleList('pi+', '', path=main)
    ma.matchMCTruth('pi+', path=main)

    ntupler = register_module('VariablesToNtuple')
    ntupler.param('fileName', 'pid_priors.root')
    ntupler.param('variables', ['cosTheta', 'p', 'mcPDG'])
    ntupler.param('particleList', 'pi+')
    main.add_module(ntupler)

    process(main)

    assert 0 == subprocess.run(['analysis-train-priors', '-i', 'pid_priors.root', '-k', 'ntuple', '-o',
                               'pid_priors.pth', '-p', '211', '321'], stdout=subprocess.DEVNULL).returncode

    assert 0 == subprocess.run(['basf2', find_file('analysis/tests/pid_priors.py_noexec')]).returncode
