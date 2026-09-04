#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Test for IR region visualization scripts.
"""

import tempfile
from pathlib import Path
from b2test_utils import check_error_free

if __name__ == "__main__":
    with tempfile.NamedTemporaryFile(suffix=".svg") as tmp_file:
        # Check that visualize_ir_region.py runs without error
        # and we pass the temporary file path as argument so it doesn't pollute the source directory.
        script_path = str(Path(__file__).resolve().parents[1] / "examples/visualization/visualize_ir_region.py")
        check_error_free("python3", script_path, tmp_file.name)
