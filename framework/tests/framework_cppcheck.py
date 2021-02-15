#!/usr/bin/env python3

"""
Perform code quality cppchecks for every commit to the framework package.
"""

import re
from b2test_utils import check_error_free

if __name__ == "__main__":
    # Ignore the nofile .. [missingInclude] that is always at the end of cppcheck
    ignoreme = 'Cppcheck cannot find all the include files'
    check_error_free("b2code-cppcheck", "cppcheck", "framework",
                     lambda x: re.findall(ignoreme, x) or x == "'")
