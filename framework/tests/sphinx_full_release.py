#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Create sphinx documentation for a full release and make sure that there are no warnings.
However, a few warnings can not easily be omitted and are therefore accepted for now.
"""

import os
import re
from b2test_utils import check_error_free, skip_test_if_light


if __name__ == "__main__":

    skip_test_if_light()

    #: ignore strange backward compatibility warning
    ignorebackward = '_BACKWARD_BACKWARD_WARNING_H'
    #: ignore false-positive warning from sphinx-argparse
    ignoreprocessstatistics = 'ProcessStatistics.ModuleStatistics'

    sphinx_output_dir = os.getenv("BELLE2_SPHINX_OUTPUTDIR")
    check_error_free(
        "b2code-sphinx-warnings", "sphinx", None,
        lambda x:
        re.findall(ignorebackward, x) or
        re.findall(ignoreprocessstatistics, x),
        ["-o", sphinx_output_dir] if sphinx_output_dir else []
    )
