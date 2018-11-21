#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import b2test_utils
import basf2.core as b2
import modularAnalysis as ma

b2.set_random_seed("1337")

print(ma.analysis_main._deprecation_warning)

# recommended use of paths should throw no deprecation warnings
print("Checking recommended use")
goodpath = b2.Path()
goodpath.add_module('EventInfoSetter', evtNumList=[10], runList=[0], expList=[0])
with b2test_utils.clean_working_directory():
    b2test_utils.safe_process(goodpath)

# call deprecated functions
print("Checking ntupleTools and analysis_main are deprecated")
ma.ntupleFile("hello.root")
ma.ntupleTree(tree_name="", list_name="", tools=[])

# process analysis_main, ensure deprecation warnings for ntupleTools
# and analysis_main itself
with b2test_utils.clean_working_directory():
    b2test_utils.safe_process(ma.analysis_main)

# now proces another path even though there are modules
# loaded onto analysis_main
anotherpath = b2.Path()
with b2test_utils.clean_working_directory():
    b2test_utils.safe_process(anotherpath)
