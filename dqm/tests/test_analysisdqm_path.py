##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Simple test script to check `analysisDQM.add_analysis_dqm` adds the modules it
should, and runs without errors (or problematic database accesses).

"""

import basf2 as b2
import b2test_utils
from analysisDQM import add_analysis_dqm, add_mirabelle_dqm
from IPDQM import add_IP_dqm
from V0DQM import add_V0_dqm


with b2test_utils.clean_working_directory():
    # test logging and seeds just in case we fall back to output comparison
    b2test_utils.configure_logging_for_tests()
    b2.set_random_seed("")

    # require_file skips the test if can't be found
    inputfile = b2test_utils.require_file("mdst14.root", "validation")

    # just run over an mdst to check the modules execute
    testpath = b2.Path()
    testpath.add_module("RootInput", inputFileName=inputfile)

    # need this guy for PhysicsObjectsDQM 🐐
    testpath.add_module("HistoManager")

    # analysis DQM modules to be tested
    add_analysis_dqm(testpath)
    add_mirabelle_dqm(testpath)
    add_IP_dqm(testpath, dqm_environment='expressreco')
    add_V0_dqm(testpath)

    b2.print_path(testpath)

    # fallback to GT replay on the file
    b2.conditions.override_globaltags()
    b2.conditions.reset()

    b2.process(testpath, 1)
