#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Perform code quality cppchecks for every commit to the generators package.
"""

import re
from b2test_utils import check_error_free

if __name__ == "__main__":
    # Comment: this is a temporary workaround, since this package is affected by cppcheck warnings
    # from other packages. check_error_free skips all the lines identified by findall, so here we skip all the lines
    # that do no not contain the name of the tested package. The output will be incomplete and a bit obscure,
    # but at least the test will correctly check if there are cppcheck warnings affecting this package.
    ignoreme = r"^((?!generators\/).)*$"
    check_error_free("b2code-cppcheck", "cppcheck", "generators",
                     lambda x: re.findall(ignoreme, x) or x == "'")
