##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Print the parameters of all the modules registered in basf2.
'''

import basf2
import b2test_utils
import pybasf2


if __name__ == "__main__":

    with b2test_utils.clean_working_directory():
        with b2test_utils.show_only_errors():

            # Get all the modules (this is a dictionary: module name and shared object)
            all_modules = pybasf2.list_available_modules()

            # Create a path and add all the modules to the path
            path = basf2.Path()
            for m, so in all_modules.items():
                path.add_module(m)
                print(f'Log level for module {m}:', basf2.logging.module(m).log_level)

            # And print the path: this automatically prints also the module parameters
            basf2.print_path(path)
