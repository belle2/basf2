#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from variables import utils

#: Replacement for DeltaEMbc
deltae_mbc = ["Mbc", "deltaE"]

#: Replacement to Kinematics tool
kinematics = ['px', 'py', 'pz', 'pt', 'p', 'E']

#: Cluster-related variables
cluster = [
    'clusterPulseShapeDiscriminationMVA',
    'clusterHasPulseShapeDiscrimination',
    'clusterNumberOfHadronDigits',
    'clusterDeltaLTemp',
    'minC2TDist',
    'nECLClusterTrackMatches',
    'clusterZernikeMVA',
    'clusterReg',
    'clusterAbsZernikeMoment40',
    'clusterAbsZernikeMoment51',
    'clusterBelleQuality',
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
    'clusterHasFailedErrorTiming',
    'clusterHasFailedTiming',
    'clusterHasNPhotons',
    'clusterHasNeutralHadron',
    'clusterLAT',
    'clusterNHits',
    'clusterPhi',
    'clusterR',
    'clusterSecondMoment',
    'clusterTheta',
    'clusterTiming',
    'clusterTrackMatch',
    'goodBelleGamma',
]

#: Cluster-related event variables
event_level_cluster = [
    'nECLOutOfTimeCrystals',
    'nECLOutOfTimeCrystalsBWDEndcap',
    'nECLOutOfTimeCrystalsBarrel',
    'nECLOutOfTimeCrystalsFWDEndcap',
    'nRejectedECLShowers',
    'nRejectedECLShowersBWDEndcap',
    'nRejectedECLShowersBarrel',
    'nRejectedECLShowersFWDEndcap',
]

#: Cluster averages
cluster_average = [
    'maxWeightedDistanceFromAverageECLTime',
    'weightedAverageECLTime'
]

#: KLM cluster information
klm_cluster = [
    'klmClusterKlId',
    'klmClusterTiming',
    'klmClusterPositionX',
    'klmClusterPositionY',
    'klmClusterPositionZ',
    'klmClusterInnermostLayer',
    'klmClusterLayers',
    'klmClusterEnergy',
    'klmClusterMomentum',
    'klmClusterIsBKLM',
    'klmClusterIsEKLM',
    'klmClusterIsForwardEKLM',
    'klmClusterIsBackwardEKLM',
    'klmClusterTheta',
    'klmClusterPhi',
    'nKLMClusterTrackMatches',
    'nMatchedKLMClusters',
]

#: Dalitz masses for three body decays
dalitz_3body = [
    'daughterInvM(0, 1)',  # invariant mass of daughters 1 and 2
    'daughterInvM(0, 2)',  # invariant mass of daughters 1 and 3
    'daughterInvM(1, 2)',  # invariant mass of daughters 2 and 3
]

#: Tracking variables, replacement for Track tool
track = ['dr', 'dx', 'dy', 'dz', 'd0', 'z0', 'pValue', 'ndf']

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
    'mcDecayVertexX',
    'mcDecayVertexY',
    'mcDecayVertexZ',
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
pid = ['kaonID', 'pionID', 'protonID', 'muonID', 'electronID', 'deuteronID',
       'binaryPID(211, 13)', 'binaryPID(211, 321)', 'binaryPID(211, 2212)', 'binaryPID(321, 2212)']

#: Replacement for ROEMultiplicities tool
roe_multiplicities = [
    'nROE_Charged()',
    'nROE_Photons()',
    'nROE_NeutralHadrons()'
]

#: Rest-Of-Event kinematics
roe_kinematics = [
    'roeE()',
    'roeM()',
    'roeP()',
    'roeMbc()',
    'roeDeltae()'
]

#: Recoil kinematics related variables
recoil_kinematics = [
    'pRecoil',
    'eRecoil',
    'mRecoil',
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
mc_flight_info = ['mcFlightTime', 'mcFlightDistance']

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
    'chiProb',
]

#: Replacement for MCVertex tuple tool
mc_vertex = [
    'mcDecayVertexX',
    'mcDecayVertexY',
    'mcDecayVertexZ',
    'mcDecayVertexFromIPDistance',
    'mcDecayVertexRho',
    'mcProductionVertexX',
    'mcProductionVertexY',
    'mcProductionVertexZ'
]

#: CPV and Tag-side related variables
tag_vertex = [
    'DeltaT',
    'DeltaTErr',
    'DeltaZ',
    'DeltaZErr',
    'DeltaBoost',
    'DeltaBoostErr',
    'TagVLBoost',
    'TagVLBoostErr',
    'TagVOBoost',
    'TagVOBoostErr',
    'TagVpVal',
    'TagVNDF',
    'TagVChi2',
    'TagVChi2IP',
    'TagVx',
    'TagVxErr',
    'TagVy',
    'TagVyErr',
    'TagVz',
    'TagVzErr',
]

#: Tag-side  related MC true variables
mc_tag_vertex = [
    'mcDeltaTau',
    'mcDeltaT',
    'mcDeltaBoost',
    'mcTagVLBoost',
    'mcTagVOBoost',
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
    'nTracks',
]

#: Replacement for InvMass tool
inv_mass = ['M', 'ErrM', 'SigM', 'InvM']

#: Extra energy variables
extra_energy = ["roeEextra()"]

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

#: Variables created by event kinematics module
event_kinematics = [
    "missingMomentumOfEvent",
    "missingMomentumOfEvent_Px",
    "missingMomentumOfEvent_Py",
    "missingMomentumOfEvent_Pz",
    "missingMomentumOfEvent_theta",
    "missingMomentumOfEventCMS",
    "missingMomentumOfEventCMS_Px",
    "missingMomentumOfEventCMS_Py",
    "missingMomentumOfEventCMS_Pz",
    "missingMomentumOfEventCMS_theta",
    "missingEnergyOfEventCMS",
    "missingMass2OfEvent",
    "visibleEnergyOfEventCMS",
    "totalPhotonsEnergyOfEvent"
]

#: Variables created by MC version of event kinematics module
mc_event_kinematics = [
    'genMissingMass2OfEvent',
    'genMissingEnergyOfEventCMS',
    'genMissingMomentumOfEventCMS',
    'genTotalPhotonsEnergyOfEvent',
    'genVisibleEnergyOfEventCMS'
]

#: Belle Track CDC hit variables
belle_track_hit = [
    "BelleFirstCDCHitX",
    "BelleFirstCDCHitY",
    "BelleFirstCDCHitZ",
    "BelleLastCDCHitX",
    "BelleLastCDCHitY",
    "BelleLastCDCHitZ"
]

# now register all the variables declared in this file as collections
# TODO: this is still not optimal but better than before. Should we not just
# get rid of the collections in the manager?
for name, value in list(globals().items()):
    if isinstance(value, list):
        utils.add_collection(value, name)
