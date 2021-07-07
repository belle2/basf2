##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
from softwaretrigger import constants
from softwaretrigger.test_support import test_folder


if __name__ == "__main__":
    test_folder(location=constants.Location.expressreco,
                run_type=constants.RunTypes.beam,
                exp_number=1003,
                phase="phase3")
