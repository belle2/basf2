#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Perform code quality cppchecks for every commit to the analysis package.
Eventually these checks can be included as git hooks.
"""

import re
from b2test_utils import skip_test, check_error_free

if __name__ == "__main__":

    # skip this test if being run on the buildbot
    if os.getlogin() is 'b2soft':
        skip_test('no need to run cppcheck test on the buildbot')

    # ignore the nofile .. [missingInclude] that is always at the end of cppcheck
    ignoreme = 'Cppcheck cannot find all the include files'
    check_error_free("b2code-cppcheck", "cppcheck", "analysis",
                     lambda x: re.findall(ignoreme, x) or x is "'")
