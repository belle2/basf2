#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This module contains utility functions related to (testing) the uDST file format.
"""

from b2test_utils.datastoreprinter import DataStorePrinter, PrintObjectsModule


def add_udst_dump(path, print_untested=False):
    """
    Add a PrintObjectsModule to a path for printing the uDST contents to
    test and maintain backwards compatibility.

    See also: `mdst.add_mdst_dump`

    Arguments:
        path (basf2.Path): Path to add modules to
        print_untested (bool): If True print the names of all methods which
                               are not explicitly printed to make sure we don't
                               miss addition of new members
    """

    eparticlesourceobjects = [i for i in range(7)]

    # Define a list of all the udst dataobjects we want to print out
    # and all the members we want to check
    protected_udst_dataobjects = [
        # DataStorePrinter("ParticleList", [], {}), # <--- !!
        DataStorePrinter(
            "Particle",
            [
                "getPDGCode",
                "getPDGMass",
                "getCharge",
                "getFlavorType",
                "getParticleSource",
                "getMdstArrayIndex",
                "getProperty",
                "getMass",
                "getEnergy",
                "get4Vector",
                "getMomentum",
                "getMomentumMagnitude",
                "getP",
                "getPx",
                "getPy",
                "getPz",
                "getVertex",
                "getX",
                "getY",
                "getZ",
                "getPValue",
                "getMomentumVertexErrorMatrix",
                "getMomentumErrorMatrix",
                "getVertexErrorMatrix",
                "getCosHelicity",
                "getAcoplanarity",
                "getMdstSource",
                "getNDaughters",
                "getDaughterIndices",
                "getDaughterProperties",
                "getDaughters",
                "getFinalStateDaughters",
                "getTrack",
                "getTrackFitResult",
                "getPDGCodeUsedForFit",
                "wasExactFitHypothesisUsed",
                "getV0",
                "getPIDLikelihood",
                "getECLCluster",
                "getECLClusterEnergy",
                "getECLClusterEHypothesisBit",
                "getKLMCluster",
                "getMCParticle",
                "getName",
                "getInfoHTML",
                "getExtraInfoNames",
                "getExtraInfoMap",
                "getExtraInfoSize",
                # "getArrayPointer",
                "print",
            ],
            {
                "getRelationsWith": [
                    "MCParticles",
                    "TrackFitResults",
                    "PIDLikelihoods"
                    # "Tracks",
                    # "ECLClusters",
                    # "KLMClusters",
                ],
                "getMdstArrayIndices": eparticlesourceobjects,
                # extra info
                # ~~~~~~~~~~
                "addExtraInfo": [["foo", 1337]],
                "hasExtraInfo": ["foo", "bar"],
                "getExtraInfo": ["foo"],
                # "writeExtraInfo": [["foo", 1234]],
                # "removeExtraInfo": ["foo"],
                # daughters
                # ~~~~~~~~~
                "getDaughter": [0, 1],
                "getCosHelicityDaughter": [0],
                "getParticleFromGeneralizedIndexString": ["0:0", "0:1"],
                # "forEachDaughter": [],
                # "appendDaughter": [],
                # "removeDaughter": [],
                # compartison to another particle
                # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                # -- untestable with these tools? but also not an issue for
                #    backwards compatibility
                # "isCopyOf": [],
                # "overlapsWith": [],
                # setters
                # ~~~~~~~
                "updateMass": [13],
                # "updateMomentum": [],
                # "set4Vector": [],
                # "setExtraInfo": [],
                # "setMomentumVertexErrorMatrix": [],
                # "setPValue": [],
                # "setProperty": [],
                # "setVertex": [],
            },
        ),
        DataStorePrinter(
            "ParticleExtraInfoMap",
            ["getNMaps"],
            {
                "getMap": [0],
                "getIndex": [[0, "foo"], [0, "bar"]],
                "getMapForNewVar": [["foo"], ["bar"]],
                # "getMapForNewVar": []
            },
            array=False,
        ),
        DataStorePrinter(
            "EventExtraInfo",
            [
                "getNames",
                "getInfoHTML",
                # removeExtraInfo",
            ],
            {
                "addExtraInfo": [["foo", 1337]],
                "hasExtraInfo": ["foo"],
                "getExtraInfo": ["foo"],
                # "setExtraInfo": []
            },
            array=False,
        ),
    ]
    path.add_module(PrintObjectsModule(protected_udst_dataobjects, print_untested))
