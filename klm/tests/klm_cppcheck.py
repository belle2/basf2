#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Perform code quality cppchecks for every commit to the klm package.
"""

import re
from b2test_utils import check_error_free

if __name__ == "__main__":
    ignoreme = "Cppcheck cannot find all the include files"
    check_error_free("b2code-cppcheck", "cppcheck", "klm",
                     lambda x: re.findall(ignoreme, x) or x == "'")
