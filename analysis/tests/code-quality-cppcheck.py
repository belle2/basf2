#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Perform code quality checks for every commit to the analysis package.
Eventually these checks can be included as git hooks.
"""

import subprocess
import re
import sys


def check_error_free(log, tool, toolname, filter=lambda x: True):
    """Check that the provided code tool's log has no comments or warnings"""
    clean_log = [e for e in str(log).split('\\n') if e and filter(e)]
    if clean_log:
        message = """\
The analysis package has some {toolname} issues, which is now not allowed.
Please run:

  $ {tool} analysis

and fix any issues you have introduced. Here is what the test {toolname} found:\
    """.format(toolname=toolname, tool=tool)
        print(message, file=sys.stderr)
        for line in clean_log:
            print(line, file=sys.stderr)
        sys.exit(1)
    return

# run cppcheck
try:
    cppcheck_output = subprocess.check_output(["b2code-cppcheck", "analysis"])
except subprocess.CalledProcessError as error:
    cppcheck_output = error.output

# ignore the nofile .. [missingInclude] that is always at the end of cppcheck
ignoreme = 'Cppcheck cannot find all the include files'
check_error_free(cppcheck_output, "b2code-cppcheck", "cppcheck",
                 lambda x: not re.findall(ignoreme, x) and x is not "'")
