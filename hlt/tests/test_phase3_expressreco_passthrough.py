##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os

from softwaretrigger import constants
from softwaretrigger.test_support import test_folder
from b2test_utils import clean_working_directory


if __name__ == "__main__":
    with clean_working_directory() as working_dir:
        os.chmod(working_dir, 0o744)  # To avoid potential writing issues on CI system
        test_folder(location=constants.Location.expressreco,
                    run_type=constants.RunTypes.beam,
                    exp_number=1003,
                    phase="phase3",
                    passthrough=True)
