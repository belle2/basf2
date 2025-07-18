##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Simple launcher for skims/tests/test_skims.py_noexec.
This tests whether:
- all the skims are working;
- there is a name clash among ParticlesLists from different skims.
'''

import basf2 as b2
import b2test_utils as b2tu
import subprocess


if __name__ == '__main__':

    b2tu.configure_logging_for_tests()

    test_skims = b2.find_file('skim/tests/test_skims.py_noexec')

    # Here we test if the skims are working
    test_result = subprocess.run(['basf2', test_skims], check=True, capture_output=True, universal_newlines=True)

    # And here we test if there is a name clash among different skims
    offending_error = ('[ERROR] An object ', 'was already created in the DataStore.')
    found_errors = set([e for e in test_result.stdout.split('\n') if offending_error[0] in e and offending_error[1] in e])
    if len(found_errors) > 0:

        b2.B2FATAL(
            'There is one or more name clashes among ParticleLists from different skims, plese fix:\n',
            '\n'.join(found_errors)
        )
