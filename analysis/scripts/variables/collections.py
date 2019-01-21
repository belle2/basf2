#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from variables import utils

#: Placehoder for FT variables collection
#: (not complete, see :issue:`BII-3853`)
flavor_tagging = ["qrOutput(FBDT)"]

#: Replacement for DeltaEMbc
deltae_mbc = ["Mbc", "deltaE"]

#: Replacement to Kinematics tool
kinematics = ['px', 'py', 'pz', 'pt', 'p', 'E']

#: Cluster-related variables
cluster = [
    'PulseShapeDiscriminationMVA',
    'ClusterHasPulseShapeDiscrimination',
    'ClusterNumberOfHadronDigits',
    'clusterDeltaLTemp',
    'maxWeightedDistanceFromAverageECLTime',
    'minC2TDist',
    'nECLClusterTrackMatches',
    'clusterZernikeMVA',
    'clusterReg',
    'clusterTrigger',
    'weightedAverageECLTime',
    'clusterAbsZernikeMoment40',
    'clusterAbsZernikeMoment51',
    'clusterBelleQuality',
    'clusterCRID',
    'clusterClusterID',
    'clusterConnectedRegionID',
    'clusterE1E9',
    'clusterE9E21',
    'clusterE9E25',
    'clusterEoP',
    'clusterErrorE',
    'clusterErrorPhi',
    'clusterErrorTheta',
    'clusterErrorTiming',
    'clusterHighestE',
    'clusterHypothesis',
    'clusterLAT',
    'clusterNHits',
    'clusterPhi',
    'clusterR',
    'clusterSecondMoment',
    'clusterTheta',
    'clusterTiming',
    'clusterTrackMatch',
    'clusterUniqueID',
    'eclExtPhi',
    'eclExtPhiId',
    'eclExtTheta',
    'goodBelleGamma',
    'nECLOutOfTimeCrystals',
    'nECLOutOfTimeCrystalsBWDEndcap',
    'nECLOutOfTimeCrystalsBarrel',
    'nECLOutOfTimeCrystalsFWDEndcap',
    'nRejectedECLShowers',
    'nRejectedECLShowersBWDEndcap',
    'nRejectedECLShowersBarrel',
    'nRejectedECLShowersFWDEndcap',
]

#: Dalitz masses for three body decays
dalitz_3body = [
    'daughterInvM(0, 1)',  # invariant mass of daughters 1 and 2
    'daughterInvM(0, 2)',  # invariant mass of daughters 1 and 3
    'daughterInvM(1, 2)',  # invariant mass of daughters 2 and 3
]

#: Tracking variables, replacement for Track tool
track = ['dr', 'dx', 'dy', 'dz', 'd0', 'z0', 'pValue']

#: Replacement for TrackHits tool
track_hits = ['nCDCHits', 'nPXDHits', 'nSVDHits', 'nVXDHits']

#: Replacement for MCTruth tool
mc_truth = ['isSignal', 'mcErrors', 'mcPDG']


#: Replacement for MCKinematics tool
mc_kinematics = ['mcE', 'mcP', 'mcPT', 'mcPX', 'mcPY', 'mcPZ', 'mcPhi']

#: Truth-matching related variables
mc_variables = [
    'genMotherID',
    'genMotherP',
    'genMotherPDG',
    'genParticleID',
    'isCloneTrack',
    'mcDX',
    'mcDY',
    'mcDZ',
    'mcDecayTime',
    'mcE',
    'mcErrors',
    'mcInitial',
    'mcP',
    'mcPDG',
    'mcPT',
    'mcPX',
    'mcPY',
    'mcPZ',
    'mcPhi',
    'mcVirtual',
    'nMCMatches',
]

#: PID variables
pid = ['kaonID', 'pionID', 'protonID', 'muonID', 'electronID', 'deuteronID']

#: Replacement for ROEMultiplicities tool
roe_multiplicities = ['nROE_KLMClusters']

#: Recoil kinematics relaed variables
recoil_kinematics = [
    'pRecoil',
    'pRecoilPhi',
    'pRecoilTheta',
    'pxRecoil',
    'pyRecoil',
    'pzRecoil',
]

#: Flight info variables
flight_info = [
    'flightTime',
    'flightDistance',
    'flightTimeErr',
    'flightDistanceErr',
]

#: MC true flight info variables
mc_flight_info = utils.create_mctruth_aliases(flight_info)

#: Replacement for Vertex tuple tool
vertex = [
    'distance',
    'significanceOfDistance',
    'dx',
    'dy',
    'dz',
    'x',
    'y',
    'z',
    'x_uncertainty',
    'y_uncertainty',
    'z_uncertainty',
    'dr',
    'dphi',
    'dcosTheta',
    'prodVertexX',
    'prodVertexY',
    'prodVertexZ',
    'prodVertexXErr',
    'prodVertexYErr',
    'prodVertexZErr',
    'pValue',
]

#: Replacement for MCVertex tuple tool
mc_vertex = [
    'mcX',
    'mcY',
    'mcZ',
    'mcDistance',
    'mcRho',
    'mcProdVertexX',
    'mcProdVertexY',
    'mcProdVertexZ'
]

#: Tag-side related variables
tag_vertex = [
    'TagVLBoost',
    'TagVLBoostErr',
    'TagVOBoost',
    'TagVOBoostErr',
    'TagVpVal',
    'TagVx',
    'TagVxErr',
    'TagVy',
    'TagVyErr',
    'TagVz',
    'TagVzErr',
]

#: Tag-side  related MC true variables
mc_tag_vertex = [
    'MCDeltaT',
    'MCTagBFlavor',
    'TagVmcLBoost',
    'TagVmcOBoost',
    'mcLBoost',
    'mcOBoost',
    'mcTagVx',
    'mcTagVy',
    'mcTagVz',
]

#: Replacement for MomentumUnertainty tool
momentum_uncertainty = [
    'E_uncertainty',
    'pxErr',
    'pyErr',
    'pzErr',
]

#: Replacement for RecoStats tool
reco_stats = [
    'nECLClusters',
    'nTracks',
]

#: Replacement for InvMass tool
inv_mass = ['M', 'ErrM', 'SigM', 'InvM']

#: Extra energy variables
extra_energy = ["ROE_eextra()"]

#: Event level tracking variables
event_level_tracking = [
    "nExtraCDCHits",
    "nExtraCDCHitsPostCleaning",
    "hasExtraCDCHitsInLayer(0)",
    "hasExtraCDCHitsInLayer(1)",
    "hasExtraCDCHitsInLayer(2)",
    "hasExtraCDCHitsInLayer(3)",
    "hasExtraCDCHitsInLayer(4)",
    "hasExtraCDCHitsInLayer(5)",
    "hasExtraCDCHitsInLayer(6)",
    "hasExtraCDCHitsInLayer(7)",
    "nExtraCDCSegments",
    "trackFindingFailureFlag",
]

#: Event shape variables
event_shape = [
    "foxWolframR1",
    "foxWolframR2",
    "foxWolframR3",
    "foxWolframR4",
    "harmonicMomentThrust0",
    "harmonicMomentThrust1",
    "harmonicMomentThrust2",
    "harmonicMomentThrust3",
    "harmonicMomentThrust4",
    "cleoConeThrust0",
    "cleoConeThrust1",
    "cleoConeThrust2",
    "cleoConeThrust3",
    "cleoConeThrust4",
    "cleoConeThrust5",
    "cleoConeThrust6",
    "cleoConeThrust7",
    "cleoConeThrust8",
    "sphericity",
    "aplanarity",
    "thrust",
    "thrustAxisCosTheta",
]

# now register all the variables declared in this file as collections
# TODO: this is still not optimal but better than before. Should we not just
# get rid of the collections in the manager?
for name, value in list(globals().items()):
    if isinstance(value, list):
        utils.add_collection(value, name)
