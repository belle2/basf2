#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import re
import os
import unittest
from basf2 import find_file


def convert_value_to_float(value):
    if value == "pi":
        return 3.141592653589793238462643383279502884
    elif value == "-pi":
        return -3.141592653589793238462643383279502884
    else:
        return float(value)


def get_metadata_from_file(file_content, pattern_str):
    """
    Extracts the bounds and number of bits for the variables from the header file content.

    Args:
        file_content (str): The full content of the ECLCluster.h or MdstRounderModule.cc file as a string.
        pattern_str (str): Regex pattern by which to identify relevant lines and capture rounding values.
                           Must capture variable name and three numbers (lower bound, upper bound, mantissa bits).

    Returns:
        dict: A dict containing (lower_bound, upper_bound, num_bits) tuples for each found variable
    """

    pattern = re.compile(pattern_str)
    matches = pattern.findall(file_content)

    variables = dict()
    for match in matches:
        values = (
            convert_value_to_float(match[1]),  # lower bound
            convert_value_to_float(match[2]),  # upper bound
            convert_value_to_float(match[3])   # mantissa bits
        )
        variables[match[0]] = values

    return variables


def get_ecl_variable_list():
    """
    Names of member variables of ECLCluster and associated expressions in MdstRounderModule.cc

    Returns:
        list: List of pairs of names as stated above.
    """

    return [
        ("m_covmat_10", "covMat(1, 0)"),
        ("m_covmat_20", "covMat(2, 0)"),
        ("m_covmat_21", "covMat(2, 1)"),
        ("m_sqrtcovmat_00", "cluster->getUncertaintyEnergy()"),
        ("m_sqrtcovmat_11", "cluster->getUncertaintyPhi()"),
        ("m_sqrtcovmat_22", "cluster->getUncertaintyTheta()"),
        ("m_deltaL", "cluster->getDeltaL()"),
        ("m_minTrkDistance", "cluster->getMinTrkDistance()"),
        ("m_absZernike40", "cluster->getAbsZernike40()"),
        ("m_absZernike51", "cluster->getAbsZernike51()"),
        ("m_zernikeMVA", "cluster->getZernikeMVA()"),
        ("m_E1oE9", "cluster->getE1oE9()"),
        ("m_E9oE21", "cluster->getE9oE21()"),
        ("m_secondMoment", "cluster->getSecondMoment()"),
        ("m_LAT", "cluster->getLAT()"),
        ("m_numberOfCrystals", "cluster->getNumberOfCrystals()"),
        ("m_time", "cluster->getTime()"),
        ("m_deltaTime99", "cluster->getDeltaTime99()"),
        ("m_theta", "cluster->getTheta()"),
        ("m_phi", "cluster->getPhi()"),
        ("m_r", "cluster->getR()"),
        ("m_PulseShapeDiscriminationMVA", "cluster->getPulseShapeDiscriminationMVA()"),
        ("m_NumberOfHadronDigits", "cluster->getNumberOfHadronDigits()"),
        ("m_logEnergy", "log(cluster->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons))"),
        ("m_logEnergyRaw", "log(cluster->getEnergyRaw())"),
        ("m_logEnergyHighestCrystal", "log(cluster->getEnergyHighestCrystal())")
    ]


class TestMdstRounderConsistency(unittest.TestCase):
    """
    Test class to validate consistency between Double32_t members variables of ECLCluster and TrackFitResult
    with MdstRounder module.
    """

    def setUp(self):
        # Initialise file contents to None for later check
        self.ecl_file_content = None
        self.tfr_file_content = None
        self.rounder_file_content = None

        file_path = find_file("include/mdst/dataobjects/ECLCluster.h")
        # Check if the file exists and attempting to read it if so
        if os.path.exists(file_path):
            with open(file_path, "r") as f:
                self.ecl_file_content = f.read()

        file_path = find_file("skim/modules/src/MdstRounderModule.cc")
        # Check if the file exists and attempting to read it if so
        if os.path.exists(file_path):
            with open(file_path, "r") as f:
                self.rounder_file_content = f.read()

        file_path = find_file("include/mdst/dataobjects/TrackFitResult.h")
        # Check if the file exists and attempting to read it if so
        if os.path.exists(file_path):
            with open(file_path, "r") as f:
                self.tfr_file_content = f.read()

        # Define regular expressions to capture variable names and rounding values from files
        # Format example: Double32_t  m_deltaL;  //[-250, 250., 10]
        self.ecl_re = r"Double32_t\s+(m_.*);\s*\/\/\s*\[(-?\w+\.?\d*),\s*(-?\w+\.?\d*),\s*(\d+)\]"
        # Format example: roundToPrecision(cluster->getDeltaL(), -250.0, 250.0, 10)
        self.rounder_re = r"roundToPrecision\((.*),\s*(-?\w+\.?\d*),\s*(-?\w+\.?\d*),\s*(\d+)\)"

        # Names of members of ECLCluster and associated expressions in MdstRounderModule.cc
        self.ecl_variable_names = get_ecl_variable_list()

    def test_metadata_is_correct(self):
        """
        Checks if the bounds and number of bits for each Double32_t variable are as expected, and no new variables are introduced.
        """

        # Test if files exist
        self.assertIsNotNone(
            self.ecl_file_content, "Could not find file include/mdst/dataobjects/ECLCluster.h"
        )
        self.assertIsNotNone(
            self.rounder_file_content, "Could not find file skim/modules/src/MdstRounderModule.cc"
        )
        self.assertIsNotNone(
            self.tfr_file_content, "Could not find file include/mdst/dataobjects/TrackFitResult.h"
        )

        ecl_metadata = get_metadata_from_file(self.ecl_file_content, self.ecl_re)
        rounder_metadata = get_metadata_from_file(self.rounder_file_content, self.rounder_re)

        # Test if expected number of ECL variables exist
        self.assertEqual(
            len(ecl_metadata)-1,  # -1 because there is one unused member variable (m_ClusterHadronIntensity)
            len(self.ecl_variable_names),
            msg="ECLCluster.h does not contain the expected number of member variables " +
                f"(should be {len(self.ecl_variable_names)+1}, found {len(ecl_metadata)}). " +
                "If this is an intentional update, please also update MdstRounder module."
        )
        self.assertEqual(
            len(rounder_metadata),
            len(self.ecl_variable_names),
            msg="MdstRounderModule.cc does not contain the expected number of rounding statements " +
                f"(should be {len(self.ecl_variable_names)}, found {len(rounder_metadata)})."
        )

        for ecl_var, rounder_expr in self.ecl_variable_names:

            # Test if the ECL variable exists in the file
            self.assertIn(
                ecl_var,
                ecl_metadata.keys(),
                msg=f"Could not find variable {ecl_var} in ECLCluster.h. " +
                    "If this is an intentional update, please also update MdstRounder module."
            )
            self.assertIn(
                rounder_expr,
                rounder_metadata.keys(),
                msg=f"Could not find expression {rounder_expr} in MdstRounderModule.cc"
            )

            lower, upper, bits = ecl_metadata[ecl_var]
            expected_lower, expected_upper, expected_bits = rounder_metadata[rounder_expr]

            # Test if bounds and bits are identical
            self.assertEqual(
                lower,
                expected_lower,
                msg=f"Lower bound mismatch for {ecl_var} between ECLCluster.h ({lower}) and MdstRounderModule.cc " +
                    f"({expected_lower}). If this is an intentional update of ECLCluster.h, please also update MdstRounder module."
            )
            self.assertEqual(
                upper,
                expected_upper,
                msg=f"Upper bound mismatch for {ecl_var} between ECLCluster.h ({upper}) and MdstRounderModule.cc " +
                    f"({expected_upper}). If this is an intentional update of ECLCluster.h, please also update MdstRounder module."
            )
            self.assertEqual(
                bits,
                expected_bits,
                msg=f"Mantissa bits mismatch for {ecl_var} between ECLCluster.h ({bits}) and MdstRounderModule.cc " +
                    f"({expected_bits}). If this is an intentional update of ECLCluster.h, please also update MdstRounder module."
            )

        # Format example: Double32_t m_cov5[c_NCovEntries];
        tfr_double32ts = re.compile(r"Double32_t\s+.*;").findall(self.tfr_file_content)
        # Format example: //[0.0, 1., 10]
        tfr_roundings = re.compile(r"//\s*\[\s*-?\w+\.?\d*,\s*-?\w+\.?\d*,\s*\d+\s*\]").findall(self.tfr_file_content)

        # Test that the expected number of Double32_t members exist and no rounding instructions are present
        self.assertEqual(
            len(tfr_double32ts),
            3,
            msg="TrackFitResult.h does not contain the expected number of Double32_t members " +
                f"(should be 3, found {len(tfr_double32ts)}). Please check that MdstRounder module still works."
        )
        self.assertEqual(
            len(tfr_roundings),
            0,
            msg="TrackFitResult.h contains Double32_t rounding instructions. Please update MdstRounder module accordingly."
        )


if __name__ == "__main__":
    unittest.main()
