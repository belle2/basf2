#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Perform code quality checks for every commit to the analysis package.
Eventually these checks can be included as git hooks.
"""

import subprocess
import re


def split_and_remove_empty_strings(output):
    """Split output into list of str (one for each line). Remove the emptys."""
    output = str(output).split('\\n')
    while "" in output:
        output.remove("")
    return output


def check_error_free(cleaned_output_log, tool, toolname):
    """Check that the provided code tool's log has no comments or warnings"""
    if cleaned_output_log:
        message = """\
The analysis package has some {toolname} issues, which is now not allowed.
Please run:

  $ {tool} analysis

and fix any issues you have introduced. Here is what the test {toolname} found:\
    """.format(toolname=toolname, tool=tool)
        print(message)
        for line in cleaned_output_log:
            print(line)
        assert False


# run cppcheck
try:
    cppcheck_output = subprocess.check_output(["b2code-cppcheck", "analysis"])
except subprocess.CalledProcessError as error:
    cppcheck = error.output

cppcheck_output = split_and_remove_empty_strings(cppcheck_output)

# ignore the missingInclude that is always at the end of cppcheck
for line in cppcheck_output:
    if re.match('nofile.*', line):
        cppcheck_output.remove(line)

# throw an error if the remaining cppcheck list is not empty
check_error_free(cppcheck_output, "b2code-cppcheck", "cppcheck")

# check doxygen
try:
    doxygen_output = subprocess.check_output(["b2code-doxygen-warnings", "analysis"])
except subprocess.CalledProcessError as error:
    doxygen_output = error.output

doxygen_output = split_and_remove_empty_strings(doxygen_output)

# special case to ignore OrcaKinFit
doxygen_output = [el for el in doxygen_output if not re.findall('Belle2::OrcaKinFit', el)]

# throw an error if the remaining doxygen list is not empty
check_error_free(doxygen_output, "b2code-doxygen-warnings", "doxygen")
