#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import numpy as np
import sys
from typing import Any
import ROOT
from ROOT import Belle2


def load_csv_as_numpy(filename: str) -> np.ndarray:
    """
    Load a CSV with possibly trailing commas and spaces into a flat numpy array of floats.
    Ensures exactly 7200 values are read.
    """
    values = []
    with open(filename) as f:
        for line in f:
            # Split on commas, strip spaces, skip empties
            numbers = [float(x) for x in line.strip().split(",") if x.strip() != ""]
            values.extend(numbers)

    values = np.array(values, dtype=np.float64)
    if values.size != 7200:
        raise ValueError(f"Expected 7200 values, but got {values.size}")
    return values


def numpy_to_std_vector(array: np.ndarray) -> Any:
    """
    Convert a 1D numpy array of floats into a ROOT std::vector<double>.
    """
    vec = ROOT.std.vector("double")()
    vec.reserve(len(array))  # reserve memory for speed
    for val in array:
        vec.push_back(float(val))
    return vec


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} data.csv")
        sys.exit(1)

    filename = sys.argv[1]

    # --- load CSV as numpy array ---
    values = load_csv_as_numpy(filename)

    # --- reshape into six chunks of 1200 ---
    vectors = values.reshape(6, 1200)
    pi0ProbsFWD, pi0ProbsBRL, pi0ProbsBWD, etaProbsFWD, etaProbsBRL, etaProbsBWD = vectors

    # --- fill ROOT std::vector<double> ---
    vec_pi0ProbsFWD = numpy_to_std_vector(pi0ProbsFWD)
    vec_pi0ProbsBRL = numpy_to_std_vector(pi0ProbsBRL)
    vec_pi0ProbsBWD = numpy_to_std_vector(pi0ProbsBWD)
    vec_etaProbsFWD = numpy_to_std_vector(etaProbsFWD)
    vec_etaProbsBRL = numpy_to_std_vector(etaProbsBRL)
    vec_etaProbsBWD = numpy_to_std_vector(etaProbsBWD)

    # --- create Belle2 parameters object (ROOT class) ---
    params = Belle2.BellePi0EtaProbParameters(
        vec_pi0ProbsFWD,
        vec_pi0ProbsBRL,
        vec_pi0ProbsBWD,
        vec_etaProbsFWD,
        vec_etaProbsBRL,
        vec_etaProbsBWD,
    )

    # --- store into conditions database ---
    db = Belle2.Database.Instance()
    iov = Belle2.IntervalOfValidity.always()
    db.storeData("BellePi0EtaProbParameters", params, iov)

    print("Successfully loaded parameters and added to DB")
