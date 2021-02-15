#!/usr/bin/env python3

"""
Check that ROOT returns the correct values (ints, floats) when calling a C++
function from python
"""

import sys
import ROOT
import numpy as np

testclass = [
    "#include <cstdint>",
    "#include <limits>",
    "struct ReturnValueTests {"
]

results = []
# what integer sizes to check
integral_types = [8, 16, 32, 64]
# which floating types to check
floating_types = ["float", "double"]
# nan and inf need to be expressed differently so make a mapping
floating_expressions = {
    -np.inf: "-std::numeric_limits<{0}>::infinity()",
    np.inf: "std::numeric_limits<{0}>::infinity()",
    np.nan: "std::numeric_limits<{0}>::quiet_NaN()",
}

for width in integral_types:
    # check signed types, we want min, max -1, 0 and 1
    dtype = np.dtype("int%d" % width)
    for i, value in enumerate([-2**(width-1), -1, 0, 1, 2**(width-1) - 1]):
        # create a unique member name
        func_name = f"int{width}test{i}"
        # do we need a prefix to the literal?
        prefix = "ull" if width > 32 else ""
        # append it to the test class
        testclass.append(f"int{width}_t {func_name}() const {{ return {value}{prefix}; }}")
        testclass.append("int{0}_t& ref_{1}() {{ static int{0}_t val = {2}{3}; return val; }}".format(
            width, func_name, value, prefix))
        # and remember name and result for checking
        results.append([func_name, value, dtype])
        results.append(['ref_' + func_name, value, dtype])

    # check unsigned types, just 0, 1, and max
    dtype = np.dtype("uint%d" % width)
    for i, value in enumerate([0, 1, 2**(width) - 1]):
        # create a unique member name
        func_name = f"uint{width}test{i}"
        # do we need a prefix to the literal?
        prefix = "ull" if width > 32 else ""
        # append it to the test class
        testclass.append(f"uint{width}_t {func_name}() const {{ return {value}ull; }}")
        testclass.append("uint{0}_t& ref_{1}() {{ static uint{0}_t val = {2}{3}; return val; }}".format(
            width, func_name, value, prefix))
        # and remember name and result for checking
        results.append([func_name, value, dtype])
        results.append(['ref_' + func_name, value, dtype])

# now add floating types
for t in floating_types:
    # and exploit that numpy offers a numeric_limits equivalent
    info = np.finfo(t[0])

    # check some values
    for i, value in enumerate([-np.inf, info.min, -1, 0, info.tiny, info.eps, 1, info.max, np.inf, np.nan]):
        func_name = f"{t}test{i}"
        # see if we can just have the literal or if we need a mapping
        expression = repr(value) if value not in floating_expressions else floating_expressions[value].format(t)
        testclass.append(f"{t} {func_name}() const {{ return {expression}; }}")
        testclass.append("{0}& ref_{1}() {{ static {0} val = {2}; return val; }}".format(t, func_name, expression))
        results.append([func_name, value, info.dtype])
        results.append(['ref_' + func_name, value, info.dtype])


# compile the test class
testclass.append("};")
ROOT.gROOT.ProcessLine("\n".join(testclass))
# get an instance
tests = ROOT.ReturnValueTests()
# and do all the checks
failures = 0
for func, value, dtype in results:
    ret = getattr(tests, func)()
    # char is odd and returns strings instead of int. The simple way of using
    # ord(string) will loose the sign so we have to convert it to the correct
    # type using numpy but for that we have to convert the string to 8bit.
    if isinstance(ret, str):
        ret = np.fromstring(ret.encode("latin1"), dtype)[0]

    # print the test
    print(f"check {func}(): {value!r} == {ret!r}: ", end="")
    # nan needs special care because it cannot be compared to itself
    if np.isnan(value):
        passed = np.isnan(ret)
    else:
        passed = (ret == value)

    print("\033[32mOK\033[0m" if passed else "\033[31mFAIL\033[0m")
    if not passed:
        failures += 1

sys.exit(failures)
