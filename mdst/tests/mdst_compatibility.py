#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Test backwards compatibility for MDST Objects.

This script is intended as test to see that mdst files written with old framework versions
are still useable in the future. To do this we read the mdst file using RootInput module
and print the result of calling (almost) all members of mdst objects and print their values.

This way the internal memory layout of MDST objects may change but if there is a
difference in the resulting output it will show ab and can be verified. This
approach also allows to check for the compatibility of getters of the mdst
objects.
"""

import sys
import bisect
from basf2 import create_path, process, Module, LogLevel, set_log_level, set_random_seed
from datastoreprinter import DataStorePrinter, PrintObjectsModule
import ROOT
from ROOT import Belle2

#: Hard coded list of EDetector value to name
EDetector = ["invalidDetector", "PXD", "SVD", "CDC", "TOP", "ARICH", "ECL",
             "KLM", "IR", "TRG", "DAQ", "BEAST", "TEST"]


# we need to print some Belle2::Const::ParticleTypes and
# Belle2::Const::DetectorSet objects so we override the __repr__
# member of these classes to provide readable output independent of it's memory
# position (in contrast to the default <"ROOT.Belle2.Const.ParticleType" object
# at 0x...>). We override str and repr members for both classes
def str_ParticleType(pt):
    """convert ParticleType objects to string"""
    return "<type: %s>" % pt.getParticlePDG().GetName()


def str_DetectorSet(ds):
    """convert DetectorSet objects to string"""
    det_ids = [EDetector[ds[i]] for i in range(ds.size())]
    return "<set: %s>" % ",".join(det_ids)


def str_vector(vec, dim):
    """convert a vector like object (TVector3, TLorentzVector) to a string"""
    return "(" + ",".join("%.6g" % vec[i] for i in range(dim)) + ")"


# and override the members of the classes to use our own functions in future
Belle2.Const.ParticleType.__repr__ = str_ParticleType
Belle2.Const.DetectorSet.__repr__ = str_DetectorSet
ROOT.TVector3.__repr__ = lambda x: str_vector(x, 3)
ROOT.TLorentzVector.__repr__ = lambda x: str_vector(x, 4)

# prepare a list of PID detector sets, one detector per set
PIDDetector_ids = [Belle2.Const.PIDDetectors.c_set[i] for i in
                   range(Belle2.Const.PIDDetectors.c_size)]
PIDDetector_sets = [Belle2.Const.PIDDetectorSet(e) for e in PIDDetector_ids]

# and also a list of all ConstStable particles defined in Belle2::Const
const_stable = [Belle2.Const.electron, Belle2.Const.muon, Belle2.Const.pion,
                Belle2.Const.kaon, Belle2.Const.proton, Belle2.Const.deuteron]


# Now we define a list of all the mdst_dataobjects we want to print out and all
# the members we want to check
mdst_dataobjects = [
    DataStorePrinter("EventMetaData", [
        "getErrorFlag", "getEvent", "getRun", "getSubrun", "getExperiment",
        "getTime", "getParentLfn", "getGeneratedWeight",
    ], array=False),
    DataStorePrinter("TRGSummary", ["getTimTypeBits"], {
        "getInputBits": range(10),
        "getFtdlBits": range(10),
        "getPsnmBits": range(10),
    }, array=False),
    DataStorePrinter("MCParticle", [
        "getPDG", "getStatus", "getMass", "getCharge", "getEnergy", "hasValidVertex",
        "getProductionTime", "getDecayTime", "getLifetime", "getVertex",
        "getProductionVertex", "getMomentum", "get4Vector", "getDecayVertex",
        "getFirstDaughter", "getLastDaughter", "getNDaughters", "getMother",
        "getSecondaryPhysicsProcess", "getSeenInDetector",
        "isVirtual", "isInitial"
    ]),
    DataStorePrinter("Track", ["getNumberOfFittedHypotheses"], {
        "getTrackFitResult": const_stable,
        "getTrackFitResultWithClosestMass": const_stable,
        "getRelationsWith": ["ECLClusters", "KLMClusters", "MCParticles", "PIDLikelihoods"],
    }),
    DataStorePrinter("V0", ["getTracks", "getTrackFitResults", "getV0Hypothesis"], {
        "getRelationsWith": ["MCParticles"],
    }),
    DataStorePrinter("TrackFitResult", [
        "getPosition", "getMomentum", "get4Momentum", "getEnergy", "getTransverseMomentum",
        "getCovariance6", "getChargeSign", "getPValue", "getD0", "getPhi0",
        "getPhi", "getOmega", "getZ0", "getTanLambda", "getCotTheta",
        "getTau", "getCov", "getCovariance5"
    ]),
    DataStorePrinter("PIDLikelihood", ["getMostLikely"], {
        "getLogL": const_stable,
        "isAvailable": PIDDetector_sets,
    }),
    DataStorePrinter("ECLCluster", [
        "getEnergy", "getTheta", "getPhi", "getR", "getUncertaintyEnergy",
        "getUncertaintyTheta", "getUncertaintyPhi", "getEnergyRaw", "getTime",
        "getDeltaTime99", "getE9oE21", "getEnergyHighestCrystal", "getLAT",
        "getNumberOfCrystals", "getStatus", "getClusterPosition",
        "getCovarianceMatrix3x3", "isTrack", "isNeutral", "getDeltaL",
        "getE1oE9", "getAbsZernike40", "getAbsZernike51", "getZernikeMVA",
        "getSecondMoment"
    ], {
        "getRelationsWith": ["MCParticles"],
    }),
    DataStorePrinter("KLMCluster", [
        "getTime", "getLayers", "getInnermostLayer",  # "getGlobalPosition", skipped because of strange return class
        "getClusterPosition", "getPosition", "getAssociatedEclClusterFlag",
        "getAssociatedTrackFlag",
        # incompatibilities: "getMomentum", "getErrorMatrix"
    ], {
        "getRelationsWith": ["ECLClusters"],
    }),
]


def print_file(filename):
    """
    process a given file and print its mdst contents
    """
    mdst_file = Belle2.FileSystem.findFile(filename)
    main = create_path()
    main.add_module("RootInput", inputFileName=mdst_file, logLevel=LogLevel.WARNING)
    main.add_module("EventInfoPrinter")
    main.add_module(PrintObjectsModule(mdst_dataobjects))
    process(main, 6)


if __name__ == "__main__":
    import ROOT
    # set a constant 1.5T magnetic field and make sure we don't get a warning
    # message for that
    set_log_level(LogLevel.ERROR)
    set_random_seed(1)
    field = Belle2.MagneticField()
    field.addComponent(Belle2.MagneticFieldComponentConstant(Belle2.B2Vector3D(0, 0, 1.5 * ROOT.Belle2.Unit.T)))
    Belle2.DBStore.Instance().addConstantOverride("MagneticField", field, False)
    set_log_level(LogLevel.INFO)
    # now run the test
    print_file("mdst/tests/mdst-v00-05-02.root")
