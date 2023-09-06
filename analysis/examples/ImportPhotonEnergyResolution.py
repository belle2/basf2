#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
from typing import Dict, Union, AnyStr, Any, Tuple, List
import json
import argparse

from ROOT.Belle2 import ECLPhotonEnergyResolution, Database, IntervalOfValidity

PathType = Union[str, AnyStr, os.PathLike]
OutputType = Dict[str, Any]
EnergyResolutionType = List[float]
BinningType = List[Tuple[float, float]]

# Add custom parser for config script
parser = argparse.ArgumentParser(allow_abbrev=True)
parser.add_argument("-f", "--filename", help="PERC summary output file")
args, _ = parser.parse_known_args()
filename = args.filename


def read_json(filename: PathType) -> OutputType:
    with open(filename, "r") as f:
        json_dict = json.load(f)

    return json_dict


def read_resolution_result(
    result_dict: Dict[str, Union[int, float, Dict[str, float]]]
) -> Tuple[Dict[str, Union[int, float]], Dict[str, Union[int, float, Dict[str, float]]]]:
    informationColumns = [
        "experimentLow",
        "experimentHigh",
        "runLow",
        "runHigh",
        "maxTheta",
        "minTheta",
        "maxPhi",
        "minPhi",
    ]
    information_dict = {}  # type: Dict[str, Union[int, float]]

    for column in informationColumns:
        assert column in result_dict.keys(), f"{column} not present in result dict. Please provide correct input."

        informationValue = result_dict.pop(column)
        assert isinstance(informationValue, int) or isinstance(
            informationValue, float
        ), f"Incorrect input format it columns {column}"

        information_dict.update({column: informationValue})

    return information_dict, result_dict


def read_energy_bin_resolution(
    energyBinDict: Dict[str, float], maxTheta: float, minTheta: float, maxPhi: float, minPhi: float
) -> Tuple[EnergyResolutionType, BinningType]:
    # Check for avaibility of requiered result columns
    neededColumns = [
        "RelativeResolution",
        "RelativeResolutionUncertaintyUp",
        "RelativeResolutionUncertaintyDown",
        "Energy",
        "EnergyBinUp",
        "EnergyBinDown",
    ]

    for column in neededColumns:
        assert column in energyBinDict.keys(), f"{column} not present in energy bin dict:\n{energyBinDict}"

    energyBinLow = energyBinDict["EnergyBinDown"]
    energyBinHigh = energyBinDict["EnergyBinUp"]

    energyResolution = [
        energyBinDict["RelativeResolution"],
        energyBinDict["RelativeResolutionUncertaintyUp"],
        energyBinDict["RelativeResolutionUncertaintyDown"],
    ]
    binning = [(energyBinLow, energyBinHigh), (minTheta, maxTheta), (minPhi, maxPhi)]

    return (energyResolution, binning)


def process_resolution(filename: PathType) -> Tuple[ECLPhotonEnergyResolution, Dict[str, Union[int, float]]]:
    input_dict = read_json(filename)
    information_dict, result_dict = read_resolution_result(input_dict)

    photonEnergyResolution = ECLPhotonEnergyResolution()
    for result in result_dict.values():
        assert isinstance(result, dict), f"Formatting issue in presented energy resolution result from {filename}"
        energyResolution, binning = read_energy_bin_resolution(
            energyBinDict=result,
            minTheta=information_dict["minTheta"],
            maxTheta=information_dict["maxTheta"],
            minPhi=information_dict["minPhi"],
            maxPhi=information_dict["maxPhi"],
        )
        photonEnergyResolution.addRelativeEnergyResolution(energyResolution=energyResolution, binning=binning)

    return photonEnergyResolution, information_dict


if __name__ == "__main__":
    if filename:
        photonEnergyResolution, information_dict = process_resolution(filename)

        database = Database.Instance()
        iov = IntervalOfValidity(
            information_dict["experimentLow"],
            information_dict["runLow"],
            information_dict["experimentHigh"],
            information_dict["runHigh"],
        )
        database.storeData("ECLPhotonEnergyResolution", photonEnergyResolution, iov)
