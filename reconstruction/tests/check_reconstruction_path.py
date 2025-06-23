##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Check if the path returned by add_reconstruction is changed. The test fails if:
- a new module is added to the path
- a new parameter is added to a module
- the default value of a parameter is modified
If the test fails, it's enough to reproduce the logfile and commit it.
'''

import basf2 as b2
import b2test_utils as b2tu
import reconstruction as re


b2tu.configure_logging_for_tests()

main = b2.Path()
re.add_reconstruction(main)
b2.print_path(path=main, defaults=True)
