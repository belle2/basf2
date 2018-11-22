#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from b2test_utils.datastoreprinter import DataStorePrinter, PrintObjectsModule
from ROOT.Belle2 import Const


def add_mdst_output(
    path,
    mc=True,
    filename='mdst.root',
    additionalBranches=[],
    dataDescription=None,
):
    """
    Add the mDST output module to a path.
    This function defines the mDST data format.

    Arguments:
        path (basf2.Path): Path to add module to
        mc (bool): Save Monte Carlo quantities? (MCParticles and corresponding relations)
        filename (str): Output file name.
        additionalBranches (list): Additional objects/arrays of event durability to save
        dataDescription (dict or None): Additional key->value pairs to be added as data description
           fields to the output FileMetaData
    """

    output = register_module('RootOutput')
    output.param('outputFileName', filename)
    branches = [
        'Tracks',
        'V0s',
        'TrackFitResults',
        'EventLevelTrackingInfo',
        'PIDLikelihoods',
        'TracksToPIDLikelihoods',
        'ECLClusters',
        'EventLevelClusteringInfo',
        'TracksToECLClusters',
        'KLMClusters',
        'KlIds',
        'KLMClustersToKlIds',
        'TRGSummary',
        'SoftwareTriggerResult',
    ]
    persistentBranches = ['FileMetaData']
    if mc:
        branches += ['MCParticles', 'TracksToMCParticles',
                     'ECLClustersToMCParticles', 'KLMClustersToMCParticles']
        persistentBranches += ['BackgroundInfo']
    branches += additionalBranches
    output.param('branchNames', branches)
    output.param('branchNamesPersistent', persistentBranches)
    # set dataDescription correctly
    if dataDescription is None:
        dataDescription = {}
    # set dataLevel to mdst if it's not already set to something else (which
    # might happen for udst output since that calls this function)
    dataDescription.setdefault("dataLevel", "mdst")
    output.param("additionalDataDescription", dataDescription)
    path.add_module(output)
    return output


def add_mdst_dump(path, print_untested=False):
    """
    Add a PrintObjectsModule to a path for printing the mDST content.

    Arguments:
        path (basf2.Path): Path to add module to
        print_untested (bool): If True print the names of all methods which are not
            explicitly printed to make sure we don't miss addition of new members
    """

    # prepare a list of PID detector sets and charged stable particles
    pid_detectors = [Const.PIDDetectorSet(Const.PIDDetectors.c_set[index]) for index in range(Const.PIDDetectors.c_size)]
    charged_stables = [Const.ChargedStable(Const.chargedStableSet.at(index)) for index in range(Const.chargedStableSet.size())]

    # Now we define a list of all the mdst_dataobjects we want to print out and all
    # the members we want to check
    mdst_dataobjects = [
        DataStorePrinter("EventMetaData", [
            "getErrorFlag", "getEvent", "getRun", "getSubrun", "getExperiment",
            "getProduction", "getTime", "getParentLfn", "getGeneratedWeight",
            ], array=False),
        DataStorePrinter("Track", ["getNumberOfFittedHypotheses", "getQualityIndicator"], {
            "getTrackFitResult": charged_stables,
            "getTrackFitResultWithClosestMass": charged_stables,
            "getRelationsWith": ["ECLClusters", "KLMClusters", "MCParticles", "PIDLikelihoods"],
            }),
        DataStorePrinter("V0", ["getTracks", "getTrackFitResults", "getV0Hypothesis"], {
            "getRelationsWith": ["MCParticles"],
            }),
        DataStorePrinter("TrackFitResult", [
            "getPosition", "getMomentum", "get4Momentum", "getEnergy", "getTransverseMomentum",
            "getCovariance6", "getParticleType", "getChargeSign", "getPValue", "getD0", "getPhi0",
            "getPhi", "getOmega", "getZ0", "getTanLambda", "getCotTheta",
            "getTau", "getCov", "getCovariance5", "getHitPatternCDC", "getHitPatternVXD"
            ]),
        DataStorePrinter("EventLevelTrackingInfo", [
            "getNCDCHitsNotAssigned", "getNCDCHitsNotAssignedPostCleaning",
            "getNCDCSegments", "getSVDFirstSampleTime", "hasAnErrorFlag",
            "hasUnspecifiedTrackFindingFailure", "hasVXDTF2AbortionFlag"], {
                             "hasCDCLayer": range(56)
                             }, array=False),
        DataStorePrinter("PIDLikelihood", ["getMostLikely"], {
            "isAvailable": pid_detectors,
            "getLogL": charged_stables,
            "getProbability": charged_stables,
            }),
        DataStorePrinter("ECLCluster", [
            "isTrack", "isNeutral", "getStatus", "getConnectedRegionId",
            "getHypothesisId", "getClusterId", "getMinTrkDistance", "getDeltaL",
            "getAbsZernike40", "getAbsZernike51", "getZernikeMVA", "getE1oE9",
            "getE9oE21", "getClusterHadronIntensity", "getNumberOfHadronDigits",
            "getSecondMoment", "getLAT", "getNumberOfCrystals", "getTime",
            "getDeltaTime99", "getPhi", "getTheta", "getR", "getEnergy",
            "getEnergyRaw", "getEnergyHighestCrystal", "getUncertaintyEnergy",
            "getUncertaintyTheta", "getUncertaintyPhi", "getClusterPosition",
            "getCovarianceMatrix3x3", "getDetectorRegion", "getUniqueId",
            "isTriggerCluster", "hasTriggerClusterMatching", "hasPulseShapeDiscrimination",
            ], {
                             "getRelationsWith": ["KlIds", "MCParticles"],
                             }),
        DataStorePrinter("EventLevelClusteringInfo", [
            "getNECLCalDigitsOutOfTimeFWD", "getNECLCalDigitsOutOfTimeBarrel",
            "getNECLCalDigitsOutOfTimeBWD", "getNECLCalDigitsOutOfTime",
            "getNECLShowersRejectedFWD", "getNECLShowersRejectedBarrel",
            "getNECLShowersRejectedBWD", "getNECLShowersRejected"
            ], array=False),
        DataStorePrinter("KLMCluster", [
            "getTime", "getLayers", "getInnermostLayer",
            "getClusterPosition", "getPosition", "getMomentumMag", "getEnergy",
            "getMomentum", "getError4x4", "getError7x7",
            "getAssociatedEclClusterFlag", "getAssociatedTrackFlag",
            ], {
                             "getRelationsWith": ["KlIds", "MCParticles"],
                             }),
        DataStorePrinter("KlId", ["isKLM", "isECL", "getKlId"]),
        DataStorePrinter("TRGSummary", ["getTimType"], {
            "getTRGSummary": range(10),
            "getPreScale": [[int(i / 32), i % 32] for i in list(range(320))],
            "getInputBits": range(10),
            "getFtdlBits": range(10),
            "getPsnmBits": range(10),
            }, array=False),
        DataStorePrinter("SoftwareTriggerResult", ["getResults"], array=False),
        DataStorePrinter("MCParticle", [
            "getPDG", "getStatus", "getMass", "getCharge", "getEnergy", "hasValidVertex",
            "getProductionTime", "getDecayTime", "getLifetime", "getVertex",
            "getProductionVertex", "getMomentum", "get4Vector", "getDecayVertex",
            "getIndex", "getArrayIndex",
            "getFirstDaughter", "getLastDaughter", "getDaughters", "getNDaughters", "getMother",
            "getSecondaryPhysicsProcess", "getSeenInDetector",
            "isVirtual", "isInitial", "isPrimaryParticle", "getName"
            ]),
        ]
    path.add_module(PrintObjectsModule(mdst_dataobjects, print_untested))
