#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from b2test_utils.datastoreprinter import DataStorePrinter, PrintObjectsModule


MDST_OBJECTS = (
    'ECLClusters',
    'ECLClustersToTracksNamedBremsstrahlung',
    'EventLevelClusteringInfo',
    'EventLevelTrackingInfo',
    'EventLevelTriggerTimeInfo',
    'KLMClusters',
    'KlIds',
    'PIDLikelihoods',
    'SoftwareTriggerResult',
    'TrackFitResults',
    'Tracks',
    'TRGSummary',
    'V0s',
)


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
    branches = list(MDST_OBJECTS)
    persistentBranches = ['FileMetaData']
    if mc:
        branches += ['MCParticles']
        persistentBranches += ['BackgroundInfo']
    branches += additionalBranches
    # set dataDescription correctly
    if dataDescription is None:
        dataDescription = {}
    # set dataLevel to mdst if it's not already set to something else (which
    # might happen for udst output since that calls this function)
    dataDescription.setdefault("dataLevel", "mdst")
    return path.add_module("RootOutput", outputFileName=filename, branchNames=branches,
                           branchNamesPersistent=persistentBranches, additionalDataDescription=dataDescription)


def add_mdst_dump(path, print_untested=False):
    """
    Add a PrintObjectsModule to a path for printing the mDST content.

    Arguments:
        path (basf2.Path): Path to add module to
        print_untested (bool): If True print the names of all methods which are not
            explicitly printed to make sure we don't miss addition of new members
    """

    # Always avoid the top-level 'import ROOT'.
    from ROOT.Belle2 import Const  # noqa

    # prepare a list of PID detector sets and charged stable particles
    pid_detectors = []
    it = Const.PIDDetectors.c_set.begin()
    while True:
        pid_detectors.append(Const.PIDDetectorSet(it.getDetector()))
        it.increment()
        if it == Const.PIDDetectors.c_set.end():
            break
    charged_stables = [Const.ChargedStable(Const.chargedStableSet.at(index)) for index in range(Const.chargedStableSet.size())]

    # Now we define a list of all the mdst_dataobjects we want to print out and all
    # the members we want to check
    mdst_dataobjects = [
        DataStorePrinter("EventMetaData", [
            "getErrorFlag", "getEvent", "getRun", "getSubrun", "getExperiment",
            "getProduction", "getTime", "getParentLfn", "getGeneratedWeight",
            "isEndOfRun"
        ], array=False),

        DataStorePrinter("Track", [
            "getNumberOfFittedHypotheses", "getQualityIndicator", "isFlippedAndRefitted",
            "getTrackTime", "wasRefined"
            ], {
            "getTrackFitResult": charged_stables,
            "getTrackFitResultWithClosestMass": charged_stables,
            "getRelationsWith": ["ECLClusters", "KLMClusters", "MCParticles", "PIDLikelihoods"],
        }),
        DataStorePrinter("V0", [
            "getTracks", "getTrackFitResults", "getV0Hypothesis",
            "getFittedVertexX", "getFittedVertexY", "getFittedVertexZ"
            ], {
            "getRelationsWith": ["MCParticles"],
        }),
        DataStorePrinter("TrackFitResult", [
            "getPosition", "getMomentum", "get4Momentum", "getEnergy", "getTransverseMomentum",
            "getCovariance6", "getParticleType", "getChargeSign", "getPValue", "getD0", "getPhi0",
            "getPhi", "getOmega", "getZ0", "getTanLambda", "getCotTheta",
            "getTau", "getCov", "getCovariance5", "getHitPatternCDC", "getHitPatternVXD", "getNDF", "getChi2",
        ]),
        DataStorePrinter("EventLevelTrackingInfo", [
            "getNCDCHitsNotAssigned", "getNCDCHitsNotAssignedPostCleaning",
            "getNCDCSegments", "getSVDFirstSampleTime", "hasAnErrorFlag",
            "hasUnspecifiedTrackFindingFailure", "hasVXDTF2AbortionFlag",
            "hasSVDCKFAbortionFlag", "hasPXDCKFAbortionFlag", "hasSVDSpacePointCreatorAbortionFlag"], {
            "hasCDCLayer": range(56)
        }, array=False),
        DataStorePrinter("PIDLikelihood", ["getMostLikely"], {
            "isAvailable": pid_detectors,
            "getLogL": charged_stables,
            "getProbability": charged_stables,
        }),
        DataStorePrinter("ECLCluster", [
            "isTrack", "isNeutral", "getStatus", "getConnectedRegionId",
            "getClusterId", "getUniqueClusterId", "getMinTrkDistance", "getDeltaL",
            "getAbsZernike40", "getAbsZernike51", "getZernikeMVA", "getE1oE9",
            "getE9oE21", "getNumberOfHadronDigits", "getR", "getHypotheses",
            "getSecondMoment", "getLAT", "getNumberOfCrystals", "getTime",
            "getDeltaTime99", "hasFailedFitTime", "hasFailedTimeResolution", "getPhi", "getTheta",
            "getEnergyRaw", "getEnergyHighestCrystal", "getUncertaintyEnergy",
            "getUncertaintyTheta", "getUncertaintyPhi", "getClusterPosition",
            "getCovarianceMatrix3x3", "getDetectorRegion",
            "isTriggerCluster", "hasTriggerClusterMatching", "hasPulseShapeDiscrimination",
            "getPulseShapeDiscriminationMVA", "getMaxECellId", "getMinTrkDistanceID"
        ], {
            "getEnergy": [16, 32],
            "hasHypothesis": [16, 32],
            "getRelationsWith": ["KlIds", "MCParticles"],
        }),
        DataStorePrinter("EventLevelClusteringInfo", [
            "getNECLCalDigitsOutOfTimeFWD", "getNECLCalDigitsOutOfTimeBarrel",
            "getNECLCalDigitsOutOfTimeBWD", "getNECLCalDigitsOutOfTime",
            "getNECLShowersRejectedFWD", "getNECLShowersRejectedBarrel",
            "getNECLShowersRejectedBWD", "getNECLShowersRejected",
            "getNKLMDigitsMultiStripFWD", "getNKLMDigitsMultiStripBarrel",
            "getNKLMDigitsMultiStripBWD", "getNKLMDigitsMultiStrip",
            "getNECLShowersFWD", "getNECLShowersBarrel",
            "getNECLShowersBWD", "getNECLShowers",
            "getNECLLocalMaximumsFWD", "getNECLLocalMaximumsBarrel",
            "getNECLLocalMaximumsBWD", "getNECLLocalMaximums",
            "getNECLTriggerCellsFWD", "getNECLTriggerCellsBarrel",
            "getNECLTriggerCellsBWD", "getNECLTriggerCells"
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
        DataStorePrinter("TRGSummary", [
            # getter functions to be called without arguments
            "getTimType", "getTimQuality", "isPoissonInInjectionVeto"
        ], {
            # getter functions to be called with arguments
            "getTRGSummary": range(10),
            "getPreScale": [[int(i / 32), i % 32] for i in list(range(320))],
            "getInputBits": range(10),
            "getFtdlBits": range(10),
            "getPsnmBits": range(10)
        }, array=False),
        DataStorePrinter("SoftwareTriggerResult", ["getResults", "getNonPrescaledResults"], array=False),
        DataStorePrinter("MCParticle", [
            "getPDG", "getStatus", "getMass", "getCharge", "getEnergy", "hasValidVertex",
            "getProductionTime", "getDecayTime", "getLifetime", "getVertex",
            "getProductionVertex", "getMomentum", "get4Vector", "getDecayVertex",
            "getIndex", "getArrayIndex",
            "getFirstDaughter", "getLastDaughter", "getDaughters", "getNDaughters", "getMother",
            "getSecondaryPhysicsProcess", "getSeenInDetector",
            "isVirtual", "isInitial", "isPrimaryParticle", "getName"
        ], {'getDaughter': [0], 'getParticleFromGeneralizedIndexString': ['0:0']}),
        DataStorePrinter("EventLevelTriggerTimeInfo", [
            "isValid", "hasInjection", "isHER", "isRevo2",
            "getTimeSinceLastInjection", "getTimeSincePrevTrigger", "getBunchNumber",
            "getTimeSinceLastInjectionInMicroSeconds", "getTimeSincePrevTriggerInMicroSeconds",
            "getTimeSinceInjectedBunch", "getTimeSinceInjectedBunchInMicroSeconds", "getTriggeredBunchNumberGlobal"
            ], array=False),
    ]
    path.add_module(PrintObjectsModule(mdst_dataobjects, print_untested))
