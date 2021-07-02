#!/usr/bin/env python3

"""
Perform code doxygen checks for every commit to the pxd package.
"""

import re
from b2test_utils import check_error_free
from b2test_utils import skip_test

skip_test("New doxygen version in latest externals.")

if __name__ == "__main__":
    #: Ignore nothing
    ignoreme = 'IGNORE_NOTHING'
    check_error_free("b2code-doxygen-warnings", "doxygen", "pxd",
                     lambda x: re.findall(ignoreme, x) or x == "'", ["-j4"])
