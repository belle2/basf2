#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Perform code quality cppchecks for every commit to the analysis package.
Eventually these checks can be included as git hooks.
"""

import re
from b2test_utils import check_error_free, skip_test

if __name__ == "__main__":
    skip_test("new cppcheck version")
    # ignore the nofile .. [missingInclude] that is always at the end of cppcheck
    ignoreme = 'Cppcheck cannot find all the include files'
    check_error_free("b2code-cppcheck", "cppcheck", "analysis",
                     lambda x: re.findall(ignoreme, x) or x is "'")
