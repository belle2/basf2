#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import multiprocessing
import tempfile
import basf2.core as b2
import modularAnalysis as ma


def fork_process(*args, target=b2.process):
    """Run function in forked child to eliminate side effects like B2FATAL"""
    # stolen from framework/tests/logging.py
    b2.set_random_seed("1337")
    sub = multiprocessing.Process(target=target, args=args)
    sub.start()
    sub.join()


print(ma.analysis_main._deprecation_warning)

with tempfile.TemporaryDirectory() as tempdir:
    ma.ntupleFile("hello.root")
    ma.ntupleTree(tree_name="", list_name="", tools=[])
    fork_process(ma.analysis_main)
