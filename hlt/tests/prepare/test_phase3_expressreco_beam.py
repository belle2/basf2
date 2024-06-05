##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import pathlib
import sys
from basf2 import find_file
from softwaretrigger import constants
from softwaretrigger.test_support import get_file_name, generate_input_file


if __name__ == "__main__":

    prepare_path = os.getenv("BELLE2_PREPARE_PATH", "")
    if prepare_path == "":
        parent_path = pathlib.Path(find_file('framework')).parent.absolute()
        prepare_path = os.path.join(parent_path, "prepare_tests")
        try:
            os.mkdir(prepare_path)
        except FileExistsError:
            pass

    run_type = constants.RunTypes.beam
    location = constants.Location.expressreco
    passthrough = False
    simulate_events_of_doom_buster = False
    output_file_name = get_file_name(
        prepare_path, run_type, location, passthrough, simulate_events_of_doom_buster
    )
    sys.exit(generate_input_file(
        output_file_name=output_file_name,
        run_type=run_type,
        location=location,
        exp_number=1003,
        passthrough=passthrough,
        simulate_events_of_doom_buster=simulate_events_of_doom_buster)
    )
