#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Perform code quality checks for every commit to the analysis package.
Eventually these checks can be included as git hooks.
"""

from b2test_utils import local_software_directory
import subprocess
import re
import sys


def check_error_free(tool, toolname, package, filter=lambda x: False):
    """
    Check that the provided code tool doesn't procude comments or warnings

    Arguments:
        tool(str): executable to call
        toolname(str): human readable name of the tool
        package(str): package to run over. Also the first argument to the tool
        filter(lambda): function which gets called for each line of output and
           if it returns True the line will be ignored.
    """
    with local_software_directory():
        try:
            output = subprocess.check_output([tool, package], encoding="utf8")
        except subprocess.CalledProcessError as error:
            print(error)
            output = error.output

    clean_log = [e for e in output.splitlines() if e and not filter(e)]
    if len(clean_log) > 0:
        print(f"""\
The {package} package has some {toolname} issues, which is now not allowed.
Please run:

  $ {tool} {package}

and fix any issues you have introduced. Here is what {toolname} found:\n""")
        print("\n".join(clean_log))
        sys.exit(1)


if __name__ == "__main__":
    # ignore the nofile .. [missingInclude] that is always at the end of cppcheck
    ignoreme = 'Cppcheck cannot find all the include files'
    check_error_free("b2code-cppcheck", "cppcheck", "analysis",
                     lambda x: re.findall(ignoreme, x) or x is "'")
