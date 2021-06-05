#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Perform code doxygen checks for every commit to the analysis package.
Eventually these checks can be included as git hooks.
"""

import re
from b2test_utils import check_error_free
from b2test_utils import skip_test

skip_test("New doxygen version in latest externals.")

if __name__ == "__main__":
    # run the check ignoring OrcaKinFit errors
    ignoreme = 'Belle2::OrcaKinFit'
    check_error_free("b2code-doxygen-warnings", "doxygen", "analysis",
                     lambda x: re.findall(ignoreme, x) or x == "'")
