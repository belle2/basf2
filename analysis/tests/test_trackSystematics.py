##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import modularAnalysis as ma
import b2test_utils

"""The unit test case for track momentum scaling"""

path = b2.create_path()

b2test_utils.configure_logging_for_tests()
b2.set_random_seed("1103")

ma.inputMdstList(filelist=[b2test_utils.require_file("mdst16.root", "validation")],
                 entrySequences=["0:5"],
                 path=path)

ma.fillParticleList('pi+:sig', "pt > 0.1", path=path)

# print variables before momentum scaling
ma.printVariableValues('pi+:sig', ['p', 'px', 'py', 'pz', 'M', 'E', 'theta', 'phi'], path=path)


# print variables after momentum scaling
ma.scaleTrackMomenta(['pi+:sig'], scale=1.01, path=path)
ma.printVariableValues('pi+:sig', ['p', 'px', 'py', 'pz', 'M', 'E', 'theta', 'phi'], path=path)

with b2test_utils.clean_working_directory():
    with b2test_utils.set_loglevel(b2.LogLevel.INFO):
        b2.process(path)
