#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
This module defines the uDST (user-defined data summary table) file format.
A uDST contains all dataobjects from mdst plus Particles and ParticleLists.

.. seealso:: :ref:`mdst`
"""

import basf2
import mdst
import pdg
from modularAnalysis import removeParticlesNotInLists
from b2test_utils.datastoreprinter import DataStorePrinter, PrintObjectsModule


def add_udst_output(
    path, filename, particleLists=None, additionalBranches=None, dataDescription=None, mc=True,
):
    """
    Save uDST (user-defined Data Summary Tables) = MDST + Particles + ParticleLists
    The charge-conjugate lists of those given in particleLists are also stored.
    Additional Store Arrays and Relations to be stored can be specified via additionalBranches
    list argument.

    See also: `mdst.add_mdst_output`

    Note:
        that this does not reduce the amount of Particle objects saved,
        see `udst.add_skimmed_udst_output` for a function that does.

    :param basf2.Path path: Path to add the output modules to.
    :param str filename: Name of the output file.
    :param list(str) particleLists: Names of the particle lists to write out.
    :param list(str) additionalBranches: datastore arrays/objects to write to the output
        file in addition to mdst and particle information
    :param dict dataDescription: Additional data descriptions to add to the output
        file. For example {"mcEventType":"mixed"}
    :param bool mc: Save Monte Carlo quantities? (MCParticles and corresponding relations)
    """

    if particleLists is None:
        particleLists = []
    if additionalBranches is None:
        additionalBranches = []

    # also add anti-particle lists
    plSet = set(particleLists)
    for List in particleLists:
        name, label = List.split(":")
        plSet.add(pdg.conjugate(name) + ":" + label)

    # define the dataobjects to be included
    partBranches = (
        [
            "Particles",
            "ParticlesToMCParticles",
            "ParticlesToPIDLikelihoods",
            "ParticleExtraInfoMap",
            "EventExtraInfo",
        ]
        + additionalBranches
        + list(plSet)
    )

    # set dataDescription: dictionary is mutable and thus not a good
    # default argument.
    if dataDescription is None:
        dataDescription = {}

    dataDescription.update(dataLevel="udst")

    return mdst.add_mdst_output(
        path,
        mc=mc,
        filename=filename,
        additionalBranches=partBranches,
        dataDescription=dataDescription,
    )


def add_skimmed_udst_output(
    path,
    skimDecayMode,
    skimParticleLists=None,
    outputParticleLists=None,
    additionalBranches=None,
    outputFile=None,
    dataDescription=None,
    mc=True,
):
    """
    Create a new path for events that contain a non-empty particle list specified via skimParticleLists.
    Write the accepted events as a udst file, saving only particles from skimParticleLists
    and from outputParticleLists.
    Additional Store Arrays and Relations to be stored can be specified via additionalBranches
    list argument.

    :param basf2.Path path: Path to add the skim output to.
    :param str skimDecayMode: Name of the skim. If no outputFile is given this is
        also the name of the output filename. This name will be added to the
        FileMetaData as an extra data description "skimDecayMode"
    :param list(str) skimParticleLists: Names of the particle lists to skim for.
        An event will be accepted if at least one of the particle lists is not empty
    :param list(str) outputParticleLists: Names of the particle lists to store in
        the output in addition to the ones in skimParticleLists
    :param list(str) additionalBranches: datastore arrays/objects to write to the output
        file in addition to mdst and particle information
    :param str outputFile: Name of the output file if different from the skim name
    :param dict dataDescription: Additional data descriptions to add to the output
        file. For example {"mcEventType":"mixed"}
    :param bool mc: Save Monte Carlo quantities? (MCParticles and corresponding relations)
    """

    if skimParticleLists is None:
        skimParticleLists = []
    if outputParticleLists is None:
        outputParticleLists = []
    if additionalBranches is None:
        additionalBranches = []
    # if no outputfile is specified, set it to the skim name
    if outputFile is None:
        outputFile = skimDecayMode

    # make sure the output filename has the correct extension
    if not outputFile.endswith(".udst.root"):
        outputFile += ".udst.root"

    skimfilter = basf2.register_module("SkimFilter")
    skimfilter.set_name("SkimFilter_" + skimDecayMode)
    skimfilter.param("particleLists", skimParticleLists)
    path.add_module(skimfilter)
    filter_path = basf2.create_path()
    skimfilter.if_value("=1", filter_path, basf2.AfterConditionPath.CONTINUE)

    # add_independent_path() is rather expensive, only do this for skimmed events
    skim_path = basf2.create_path()
    saveParticleLists = skimParticleLists + outputParticleLists
    removeParticlesNotInLists(saveParticleLists, path=skim_path)

    # set dataDescription: dictionary is mutable and thus not a good
    # default argument.
    if dataDescription is None:
        dataDescription = {}

    dataDescription.setdefault("skimDecayMode", skimDecayMode)
    add_udst_output(
        skim_path,
        outputFile,
        saveParticleLists,
        additionalBranches,
        dataDescription=dataDescription,
        mc=mc,
    )
    filter_path.add_independent_path(skim_path, "skim_" + skimDecayMode)


def add_udst_dump(path, print_untested=False):
    """
    Add a PrintObjectsModule to a path for printing the uDST contents to
    test and maintain backwards compatibility.

    See also: mdst.add_mdst_dump

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
                "getMomentumScalingFactor",
                "print",
            ],
            {
                "getRelationsWith": [
                    "MCParticles",
                    "TrackFitResults",
                    "PIDLikelihoods"
                ],
                "getMdstArrayIndices": eparticlesourceobjects,
                "addExtraInfo": [["foo", 1337]],
                "hasExtraInfo": ["foo", "bar"],
                "getExtraInfo": ["foo"],
                "getDaughter": [0, 1],
                "getCosHelicityDaughter": [0],
                "getParticleFromGeneralizedIndexString": ["0:0", "0:1"],
                "updateMass": [13],
            },
        ),
        DataStorePrinter(
            "ParticleExtraInfoMap",
            ["getNMaps"],
            {
                "getMap": [0],
                "getIndex": [[0, "foo"], [0, "bar"]],
                "getMapForNewVar": [["foo"], ["bar"]],
            },
            array=False,
        ),
        DataStorePrinter(
            "EventExtraInfo",
            [
                "getNames",
                "getInfoHTML",
            ],
            {
                "addExtraInfo": [["foo", 1337]],
                "hasExtraInfo": ["foo"],
                "getExtraInfo": ["foo"],
            },
            array=False,
        ),
    ]
    path.add_module(PrintObjectsModule(protected_udst_dataobjects, print_untested))
