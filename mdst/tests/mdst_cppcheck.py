#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Perform code quality cppchecks for every commit to the mdst package.
"""

import re
from b2test_utils import check_error_free

if __name__ == "__main__":
    # Ignore the nofile .. [missingInclude] that is always at the end of cppcheck
    ignoreme = 'Cppcheck cannot find all the include files'
    check_error_free("b2code-cppcheck", "cppcheck", "mdst",
                     lambda x: re.findall(ignoreme, x) or x is "'")