#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""\
General steering file for running skims.
"""

import argparse
from importlib import import_module

import basf2 as b2
import modularAnalysis as ma
import skimExpertFunctions as expert
from skim.registry import skim_registry as registry


def get_argument_parser():
    """Return an ArgumentParser with all defined arguments.
    """
    registeredSkims = [skim for _, skim in registry]

    parser = argparse.ArgumentParser(help=__doc__)
    parser.add_argument("skims", nargs="+", default=[],
                        choices=registeredSkims, help="Skims to run")
    # TODO: other arguments, mirroring default basf2 arguments

    return parser


def get_skim_module(skimName):
    """Get the name of the skim python module containing the skim named ``skimName``.

    Parameters:
        skimName (str): The name of a skim. Must be present in
            `skim.registry.skim_registry`.
    Returns:
        moduleName (str): The name of the module.
    """
    skimModuleDict = {skim: module for _, skim, module in registry}
    try:
        return skimModuleDict[skimName]
    except KeyError:
        basf2.B2FATAL(f"There is no skim named {skimName} in the skim registry! "
                      "Did you forget to add it or make a typo?")


def add_skim_to_path(skimName, path=None):
    """Locate the skim list builder function, and add the skim to the path.

    Args:
        skimName (str): Name of the skim to add to the path.
        path (basf2.Path): Analysis path.

    Returns:
        skimLists (list(str)): A list of the names of the skim particle lists.
    """
    raise NotImplementedError("Can't do this yet sorry. We need to make sure "
                              "the skim package is set up to work this way.")

    # Find the module functions and build the skim list
    moduleName = get_skim_module(skimName)
    skimModule = import_module(f"skim.{moduleName}")
    skimListBuilderFunction = getattr(skimModule, skimName)
    skimLists = skimListBuilderFunction(path)

    # Write out skims
    expert.add_skim(skimName, skimLists, path=path)

    return skimLists


def main():
    parser = get_argument_parser()
    args = parser.parse_args()

    path = b2.Path()

    # Read in data
    fileList = expert.get_test_file("MC12_mixedBGx1")
    ma.inputMdstList("default", fileList, path=path)

    skimLists = []
    for skim in args.skims:
        skimLists += add_skim_to_path(skim, path)

    b2.set_log_level(b2.LogLevel.INFO)
    expert.setSkimLogging(path=path)
    ma.summaryOfLists(skimLists, path=path)

    b2.process(path)


if __name__ == "__main__":
    main()
