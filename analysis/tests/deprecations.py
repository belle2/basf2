#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import b2test_utils
import basf2.core as b2
import modularAnalysis as ma

b2.set_random_seed("1337")

print(ma.analysis_main._deprecation_warning)

ma.ntupleFile("hello.root")
ma.ntupleTree(tree_name="", list_name="", tools=[])

with b2test_utils.clean_working_directory():
    b2test_utils.safe_process(ma.analysis_main)
