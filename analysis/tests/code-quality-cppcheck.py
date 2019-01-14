#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Perform code quality cppchecks for every commit to the analysis package.
Eventually these checks can be included as git hooks.
"""

import os
import re
from b2test_utils import skip_test, check_error_free

if __name__ == "__main__":

    # skip this test if run on the buildbot (or anywhere where BELLE2_LOCAL_DIR isn't set)
    if os.environ.get('BELLE2_LOCAL_DIR') is None:
        skip_test('skipping test, cannot find local dir (perhaps this is buildbot?)')

    # ignore the nofile .. [missingInclude] that is always at the end of cppcheck
    ignoreme = 'Cppcheck cannot find all the include files'
    check_error_free("b2code-cppcheck", "cppcheck", "analysis",
                     lambda x: re.findall(ignoreme, x) or x is "'")
