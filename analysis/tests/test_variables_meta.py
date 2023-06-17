#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import re
import logging
import subprocess
import unittest

import basf2


def findMatchedParenthesis(string: str, openchar: str, closechar: str) -> int:
    """Find matching control token in string.

    Args:
        string (str): input
        openchar (str): opening char e.g '{'
        closechar (str): closing char e.g '}'

    Returns:
        int: position of matching closing char in string.
    """
    end = 1
    if string[0] == openchar:
        count = 1
        while end < len(string) and count > 0:
            if string[end] == openchar:
                count += 1
            elif string[end] == closechar:
                count -= 1
            end += 1
    return end - 1


class MetavariableDataTypeTest(unittest.TestCase):
    """
    Determine metavariable types from source code and assert correctness
    """

    #: we have to hardcode some special cases
    #: as data types can't be obtained from source code directly
    hardcoded = {
        "softwareTriggerResult": "double",
        "formula": "double",
        "softwareTriggerResultNonPrescaled": "double",
        "isDaughterOfList": "bool",
        "isGrandDaughterOfList": "bool",
        "daughterDiffOf": "double",
        "daughterDiffOfPhi": "double",
        "daughterDiffOfClusterPhi": "double",
        "mcDaughterDiffOfPhi": "double",
        "grandDaughterDiffOfPhi": "double",
        "grandDaughterDiffOfClusterPhi": "double",
        "daughterDiffOfPhiCMS": "double",
        "daughterDiffOfClusterPhiCMS": "double",
    }

    # regular expressions
    #: regex for finding the REGISTER_METAVARIABLE statements.
    registering_regex = re.compile(
        r"(?s)REGISTER_METAVARIABLE.*?Manager::VariableDataType::(?:c_double|c_int|c_bool)"  # noqa
    )
    #: regex for extracting the function name and the enum type from REGISTER_METAVARIABLE statements  # noqa
    extract_regex = re.compile(
        r'REGISTER_METAVARIABLE\(".*?",(?P<function_name>[^,]*),.*?Manager::VariableDataType::c_(?P<enumtype>double|bool|int)'  # noqa
    )
    #: regex for extracting the type of the lambda function in metavariable function definition  # noqa
    lambda_type_regex = re.compile(r"-> (?P<lambdatype>double|bool|int)")

    def process_file(self, filepath: str) -> int:
        """Check all metavariable types for specified file.

        Args:
            filepath (str): path to file containing REGISTER_METAVARIABLE

        Raises:
            AssertionError: Raised if no expected function definition is found.
            AssertionError: Rased if lambda function has no associated
                            type information, or no lambda function is defined.

        Returns:
            int: number of metavariables in file.
            Used for sanity checks of the coverage.
        """

        # Read file contents
        with open(filepath) as fp:
            filecontent = fp.read()

        # List for all found registering statements
        registering_statements = self.registering_regex.findall(filecontent)
        # Preprocess all statements by removing spaces and newlines
        # This makes extraction of function name and enum type easier
        registering_statements = list(
            map(
                lambda line: line.replace(" ", "").replace("\n", ""),
                registering_statements,
            ),
        )

        for statement in registering_statements:
            # Extract enum type and name
            match_content = self.extract_regex.match(statement)
            function_name = match_content.groupdict()["function_name"]
            enumtype = match_content.groupdict()["enumtype"]

            if function_name in self.hardcoded:
                self.assertEqual(
                    enumtype,
                    self.hardcoded[function_name],
                    f"Metavariable '{function_name}' in file {filepath}:\n"
                    f"Metavariable function return type and Manager::VariableDataType have to match.\n"  # noqa
                    f"Expected: Manager::VariableDataType::{self.hardcoded[function_name]}, actual: Manager::VariableDataType::{enumtype}",  # noqa
                )
                continue

            # compile regex for finding metavariable function definition
            function_definition_regex = re.compile(
                r"Manager::FunctionPtr %s\(.*\)[^\{]*" % function_name
            )
            # compile regex for regular typed function definition
            regular_definition_regex = re.compile(
                r"(?P<return_type>double|int|bool) %s\(.*?\)" % function_name
            )

            # Find function body start with regex
            definition = function_definition_regex.search(filecontent)
            if definition is not None:
                func_body_start = definition.end()
            else:  # Manager::FunctionPtr definition not found
                # search for a regular typed function
                return_type = (
                    regular_definition_regex.search(filecontent)
                    .groupdict()
                    .get("return_type")
                )  # noqa
                if return_type is not None:
                    self.assertEqual(
                        return_type,
                        enumtype,
                        f"Metavariable '{function_name}' in file {filepath}:\n"
                        "Metavariable function return type and Manager::VariableDataType have to match."  # noqa
                        f"Return type is {return_type} but it is registered as Manager::VariableDataType::c_{enumtype}.\n",  # noqa
                    )
                    continue
                else:
                    raise AssertionError(
                        f"Metavariable '{function_name}' in file {filepath}:\n"
                        "Metavariable function return type and Manager::VariableDataType have to match."  # noqa
                        "Return type of function could not be automatically determined from the source code."  # noqa
                        "You can add an exception by adding the expected return type information to "  # noqa
                        "the 'hardcoded' dictionary of this testcase."
                    )

            # grab function body end
            func_body_end = func_body_start + findMatchedParenthesis(
                filecontent[func_body_start:], "{", "}"
            )
            # Get function body slice from filecontent
            func_body = filecontent[func_body_start:func_body_end]

            # grab lambda type definition
            lambdatype_match = self.lambda_type_regex.search(func_body)
            if lambdatype_match is not None:
                lambdatype = lambdatype_match.groupdict()["lambdatype"]
                self.assertEqual(
                    lambdatype,
                    enumtype,
                    f"Metavariable '{function_name}' in file {filepath}:\n"
                    f"Lambda function has return type {lambdatype} "
                    f"but is registered with Manager::VariableDataType::c_{enumtype}.\n"  # noqa
                    "VariableDataType and lambda return type have to match.",
                )
            else:  # lambda type or definition not found
                raise AssertionError(
                    f"Metavariable '{function_name}' in {filepath}:\n"
                    "VariableDataType and lambda definition have to match.\n"
                    "Either lambda function is missing return type information"
                    ", or lambda definition could not be found.\n"  # noqa
                    "Please add return type annotation '(const Particle * particle) -> double/int/bool' to lambda.\n"  # noqa
                    "Or add this metavariable as exception, by adding the expected return type information in the 'hardcoded' dictionary of this testcase\n"  # noqa
                    f"{func_body}"
                )

        # Return the number of registering statements in this file
        return len(registering_statements)

    def test_metavariable_data_types(self):
        """Metavariables have to be registered with the correct Manager::Variable::VariableDataType enum value. This test makes sure Metavariable definition and variable registration are correct."""  # noqa
        # check if grep is available
        try:
            subprocess.run(["grep", "-V"], check=True, capture_output=True)
        except subprocess.CalledProcessError:
            logging.basicConfig(format="%(message)s")
            logging.error(
                "TEST SKIPPED: MetavariableDataTypeTest skipped because grep is not available."  # noqa
            )
            self.fail()

        # Use grep to find files with REGISTER_METAVARIABLE statements
        analysis_module = basf2.find_file("analysis")
        files = subprocess.run(
            [
                "grep",
                "REGISTER_METAVARIABLE",
                "-r",
                analysis_module,
                "-I",
                "-l",
            ],
            capture_output=True,
        )
        # Decode stdout and extract filenames
        files = files.stdout.decode().split("\n")
        files = list(filter(lambda file: file.endswith(".cc"), files))

        num_files = len(files)
        print(f"Number of files including meta-variables is {num_files}")

        # There should be at least 13 files
        self.assertGreaterEqual(num_files, 13)
        # We track the number of metavariables to make sure we don't miss some
        num_metavariables = 0
        for filepath in files:
            num_metavariables += self.process_file(filepath)

        # We should get at least 229 registering statements
        print(f"Number of meta-variables is {num_metavariables}")
        self.assertGreaterEqual(num_metavariables, 229)


if __name__ == "__main__":
    unittest.main()
