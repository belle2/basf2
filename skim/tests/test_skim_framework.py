#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Tests to ensure that the structure of the skim package is maintained.
"""


from importlib import import_module
from inspect import getmembers, isclass
import os
from pathlib import Path
import unittest

from basf2 import find_file
from skim.registry import Registry
from skim import BaseSkim


class TestSkimRegistry(unittest.TestCase):
    """Test case for skim registry."""

    ExistentModulePaths = Path(find_file("skim/scripts/skim/WGs")).glob("*.py")
    ExistentModules = [
        module.stem
        for module in ExistentModulePaths
        if module.stem not in ["__init__", "registry"]
    ]

    def assertIsSubclass(self, cls, parent_cls, msg=None):
        """Fail if `cls` is not a subclass of `parent_cls`."""
        if not issubclass(cls, parent_cls):
            standardMsg = f"{cls!r} is not a subclass of {parent_cls!r}"
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

    def test_invalid_names(self):
        """Check that that no registered skims have invalid names."""
        for name in Registry.names:
            self.assertFalse(
                name.startswith("Base"),
                (
                    f"Invalid skim name in registry: {name}. Registed skim names cannot"
                    " begin with 'Base'; this word is reserved for subclassing purposes."
                ),
            )

    def test_encode(self):
        """Check that we raise a LookupError if the skim name doesn't exist."""
        with self.assertRaises(LookupError):
            Registry.encode_skim_name("SomeNonExistentSkimName")

    def test_decode(self):
        """Check that we raise a LookupError if the skim code doesn't exist."""
        with self.assertRaises(LookupError):
            Registry.decode_skim_code("1337")

    def test_modules_exist(self):
        """Check that all modules listed in registry exist in skim/scripts/skim/."""
        for module in Registry.modules:
            if module == 'flagged':
                continue

            self.assertIn(
                module,
                self.ExistentModules,
                (
                    f"Module {module} listed in registry, but does not exist in "
                    "skim/scripts/skim/."
                ),
            )

    def test_clashing_skim_and_module_names(self):
        """Check that there is no overlap between skim and module names."""
        duplicates = set(Registry.modules).intersection(Registry.names)
        self.assertEqual(
            set(),
            duplicates,
            f"Name used for both a skim and a module: {', '.join(duplicates)}",
        )

    def test_skims_exist(self):
        """Check that the registry is correct about the location of every skim.

        This test uses the information from the registry, and checks for missing skims
        in the modules.
        """
        for ModuleName in Registry.modules:
            if ModuleName == 'flagged':
                continue  # flagged category are not actual skims

            SkimModule = import_module(f"skim.WGs.{ModuleName}")
            for SkimName in Registry.get_skims_in_module(ModuleName):
                # Check the skim is defined in the module
                self.assertIn(
                    SkimName,
                    SkimModule.__dict__.keys(),
                    (
                        f"Registry lists {SkimName} as existing in skim.WGs.{ModuleName}, "
                        "but no such skim found!"
                    ),
                )

                # Check that it is defined as a subclass of BaseSkim
                SkimClass = getattr(SkimModule, SkimName)
                self.assertIsSubclass(
                    SkimClass,
                    BaseSkim,
                    f"Skim {SkimName} must be defined as a subclass of BaseSkim.",
                )

    def test_undocumented_skims(self):
        """Check that every skim defined in a module is listed in the registry.

        This test uses the information from the modules, and checks for missing or
        incorrect skim information in the registry.
        """
        for ModuleName in self.ExistentModules:
            if ModuleName == 'flagged':
                continue  # flagged category are not actual skims

            SkimModule = import_module(f"skim.WGs.{ModuleName}")

            # Inspect the module, and find all BaseSkim subclasses
            SkimNames = [
                obj[0]
                for obj in getmembers(SkimModule, isclass)
                if (
                    issubclass(obj[1], BaseSkim)
                    and obj[1] is not BaseSkim
                    and obj[0] != "CombinedSkim"
                    # Allow "Base" at beginning of skims, for subclassing
                    and not obj[0].startswith("Base")
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


class TestSkimValidation(unittest.TestCase):
    def test_validation_scripts_exist(self):
        """
        Check that all skims with a ``validation_histograms`` method defined have a
        script in skim/validation/, and vice versa. This unit test exists to make sure
        that the code auto-generated by ``b2skim-generate-validation`` stays up to date.
        """
        SkimsWithValidationMethod = [
            skim
            for skim in Registry.names
            if skim[:2] != "f_" and not Registry.get_skim_function(skim)()._method_unchanged(
                "validation_histograms"
            )
        ]
        SkimsWithValidationScript = [script.stem for script in Path(find_file("skim/validation")).glob("*.py")]

        for skim in SkimsWithValidationMethod:
            self.assertIn(
                skim,
                SkimsWithValidationScript,
                (
                    f"Skim {skim} has a `validation_histograms` method defined, but no "
                    "script has been added to skim/validation/ yet. Please add this "
                    "script using:\n"
                    f"    $ b2skim-generate-validation --skims {skim} --in-place"
                ),
            )
        for skim in SkimsWithValidationScript:
            self.assertIn(
                skim,
                SkimsWithValidationMethod,
                (
                    f"Skim {skim} has a script in skim/validation/, but no "
                    "`validation_histograms` method defined."
                ),
            )

    @unittest.skipIf(
        not os.getenv("BELLE2_VALIDATION_DATA_DIR"),
        "BELLE2_VALIDATION_DATA_DIR environment variable not set."
    )
    def test_validation_samples(self):
        """
        Check that all ``validation_sample`` attributes of skims point to existing files.
        """
        for skim in Registry.names:
            if skim[:2] == "f_":
                continue

            SkimObject = Registry.get_skim_function(skim)()
            # Don't bother checking sample if no `validation_histograms` method is defined
            if SkimObject._method_unchanged("validation_histograms"):
                continue

            try:
                find_file(SkimObject.validation_sample, data_type="validation")
            except FileNotFoundError:
                self.fail(f"{skim}.validation_sample does not point to an existing validation file.")


if __name__ == "__main__":
    unittest.main()
