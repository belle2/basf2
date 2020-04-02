#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from importlib import import_module
from inspect import getmembers, isclass
from pathlib import Path
import unittest

from basf2 import find_file
from skim.registry import Registry, combined_skims
import skimExpertFunctions as expert

__authors__ = ["Sam Cunliffe", "Phil Grace"]


class TestSkimCodes(unittest.TestCase):
    """Test case for skim registry."""

    ExistentModulePaths = Path(find_file("skim/scripts/skim")).glob("*.py")
    ExistentModules = [
        module.stem
        for module in ExistentModulePaths
        if module.stem not in ["__init__", "registry"]
    ]

    def assertIsSubclass(self, cls, parent_cls, msg=None):
        """Fail if `cls` is not a subclass of `parent_cls`."""
        if not issubclass(cls, parent_cls):
            standardMsg = "%r is not a subclass of %r" % (cls, parent_cls)
            self.fail(self._formatMessage(msg, standardMsg))

    def test_code_format(self):
        """Check the codes are the correct format (8 digits)."""
        # https://confluence.desy.de/x/URdYBQ
        for code in Registry.codes:
            self.assertEqual(len(code), 8, "Incorrect length skim code")
            self.assertTrue(code.isdigit(), "Must consist of digits")

    def test_unique_codes(self):
        """Check that there aren't two skims registered with the same code."""
        self.assertEqual(
            len(Registry.codes), len(set(Registry.codes)), "Duplicated skim code"
        )

    def test_unique_names(self):
        """Check that there aren't two skims registered with the same name."""
        self.assertEqual(
            len(Registry.names), len(set(Registry.names)), "Duplicated skim name"
        )

    def test_encode(self):
        """Check that we raise a LookupError if the skim name doesn't exist."""
        with self.assertRaises(LookupError):
            expert.encodeSkimName("SomeNonExistentSkimName")

    def test_decode(self):
        """Check that we raise a LookupError if the skim code doesn't exist."""
        with self.assertRaises(LookupError):
            expert.decodeSkimName("1337")

    def test_modules_exist(self):
        """Check that all modules listed in registry exist in skim/scripts/skim/."""
        for module in Registry.modules:
            self.assertIn(
                module,
                self.ExistentModules,
                (
                    f"Module {module} listed in registry, but does not exist in "
                    "skim/scripts/skim/."
                ),
            )

    def test_skims_exist(self):
        """Check that the registry is correct about the location of every skim.

        This test uses the information from the registry, and checks for missing skims
        in the modules.
        """
        for ModuleName in Registry.modules:
            SkimModule = import_module(f"skim.{ModuleName}")
            for SkimName in Registry.get_expected_skims_in_module(ModuleName):
                # Check the skim is defined in the module
                self.assertIn(
                    SkimName,
                    SkimModule.__dict__.keys(),
                    (
                        f"Registry lists {SkimName} as existing in skim.{ModuleName}, "
                        "but no such skim found!"
                    )
                )

                # Check that it is defined as a subclass of BaseSkim
                SkimClass = getattr(SkimModule, SkimName)
                self.assertIsSubclass(
                    SkimClass,
                    expert.BaseSkim,
                    f"Skim {SkimName} must be defined as a subclass of BaseSkim."
                )

    def test_undocumented_skims(self):
        """Check that every skim defined in a module is listed in the registry.

        This test uses the information from the modules, and checks for missing or
        incorrect skim information in the registry.
        """
        for ModuleName in self.ExistentModules:
            SkimModule = import_module(f"skim.{ModuleName}")

            # Inspect the module, and find all BaseSkim subclasses
            SkimNames = [
                obj[0]
                for obj in getmembers(SkimModule, isclass)
                if (
                    issubclass(obj[1], expert.BaseSkim)
                    and obj[1] is not expert.BaseSkim
                    and obj[0] != "CombinedSkim"
                )
            ]

            # Check the skim is listed in the registry with the same name
            for SkimName in SkimNames:
                self.assertIn(
                    SkimName,
                    Registry.names,
                    (
                        f"Skim {SkimName} defined in skim/scripts/skim/{ModuleName}.py, "
                        "but not listed in registry."
                    ),
                )

            # Check the module we found the skim in is the module listed in the registry
            for SkimName in SkimNames:
                ExpectedModuleName = Registry.get_skim_module(SkimName)
                self.assertEqual(
                    ExpectedModuleName,
                    ModuleName,
                    (
                        f"Skim {SkimName} defined in "
                        f"skim/scripts/skim/{ModuleName}.py, but listed in registry as "
                        f"belonging to skim/scripts/skim/{ExpectedModuleName}.py."
                    ),
                )

    def test_combined_skims(self):
        for CombinedName, skimlist in combined_skims.items():
            # Check for duplicated values in list
            self.assertEqual(
                len(skimlist),
                len(set(skimlist)),
                f"Duplicated skim in combined skim {CombinedName}."
            )

            # Check the combined skims only contain valid skim names
            for skim in skimlist:
                self.assertIn(
                    skim,
                    Registry.names,
                    (f"Skim {skim} in combined skim {CombinedName} is not a "
                     "registered skim.")
                )


if __name__ == "__main__":
    unittest.main()
