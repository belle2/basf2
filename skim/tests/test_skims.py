#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""Test skims by running them all as a single combined steering file."""

from importlib import import_module

import basf2 as b2
import modularAnalysis as ma
from skimExpertFunctions import CombinedSkim, get_test_file
from skim.registry import Registry

__authors__ = ["Phil Grace"]

# TODO: Delete this line once everything is implemented
Registry.names = ["LeptonicUntagged", "SinglePhotonDark"]

# NOTE: Another way this test could have been written is to run the standalone steering
# file for every skim:
#
#     for SkimName in Registry.names:
#         b2.b2test_utils.check_error_free(
#             "b2skim-run", f"Skim {SkimName}", "skim", toolopts=[SkimName, "-n", "10"]
#         )
#
# I think what is written below is preferable, as it makes the overall running time of
# this test smaller.


def get_skim_object(SkimName):
    """Get an instance of the skim class for the given skim.

    This is achieved by importing the module listed alongside the skim name in the
    `skim.registry.Registry`.

    Parameters:
        SkimName (str): Name of the skim to be found.

    Returns:
        SkimObject: The an instance of the requested skim object.
    """
    ModuleName = Registry.get_skim_module(SkimName)
    SkimModule = import_module(f"skim.{ModuleName}")
    SkimClass = getattr(SkimModule, SkimName)
    return SkimClass()


def main():
    path = b2.Path()
    ma.inputMdstList("default", get_test_file("MC12_mixedBGx0"), path=path)

    skim = CombinedSkim(*[get_skim_object(skim) for skim in Registry.names])
    skim(path)

    b2.process(path, max_event=10)


if __name__ == "__main__":
    main()
