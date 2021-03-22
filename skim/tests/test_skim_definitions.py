#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Tests to enforce rules about how skims are defined.
"""

import ast
import inspect
import unittest

from skim.registry import Registry


class TestSkimRules(unittest.TestCase):
    def test_ast(self):
        SkimsWithApplyEventCuts = set()

        for skim in Registry.names:
            classdef = Registry.get_skim_function(skim)
            source = inspect.getsource(classdef)
            tree = ast.parse(source)

            for node in ast.walk(tree):
                # Search for calls to applyEventCuts
                if not isinstance(node, ast.Call):
                    continue

                # Handle two cases: `applyEventCuts` and `ma.applyEventCuts`
                try:
                    function = node.func.id
                except AttributeError:
                    function = node.func.attr

                if function == "applyEventCuts":
                    SkimsWithApplyEventCuts.add(skim)

        if SkimsWithApplyEventCuts:
            plural = len(SkimsWithApplyEventCuts) > 1
            self.fail(
                f"The following skim{plural*'s'} use{(not plural)*'s'} "
                "modularAnalysis.applyEventCuts, which cannot be used in skims: "
                f"{', '.join(SkimsWithApplyEventCuts)}. Please use "
                "BaseSkim.skim_event_cuts instead."
            )


if __name__ == "__main__":
    unittest.main()
