#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import variableCollectionsTools as vct

#: Placehoder for FT variables collection
#: BII-3853
flavor_tagging = [
    "qrOutput(FBDT)"]


#: Replacement for DeltaEMbc
deltae_mbc = [
    "Mbc",
    "deltaE"]

#: Event variables
#: (Replacement for EventMetaData tool)
event_meta_data = [
    'evtNum',
    'expNum',
    'productionIdentifier',
    'runNum']

#: Replacement to Kinematics tool
kinematics = ['px',
              'py',
              'pz',
              'pt',
              'p',
              'E']

#: Kinematic variables in CMS
ckm_kinematics = vct.wrap_list(kinematics,
                               "useCMSFrame(variable)",
                               "CMS")

#: Cluster-related variables
#: BII-3896
cluster = [
    'clusterE',
    'clusterReg']

#: Tracking variables
#: Replacement for Track tool
track = [
    'dr',
    'dx',
    'dy',
    'dz',
    'd0',
    'z0',
    'pValue']

#: Replacement for TrackHits tool
track_hits = [
    'nCDCHits',
    'nPXDHits',
    'nSVDHits',
    'nVXDHits']

#: Replacement for MCTruth tool
mc_truth = [
    'isSignal',
    'mcErrors',
    'mcPDG']

#: Replacement for MCKinematics tool
mc_kinematics = [
    'mcE',
    'mcP',
    'mcPT',
    'mcPX',
    'mcPY',
    'mcPZ',
    'mcPhi'
]

#: Replacement for MCHierarchy tool
#: What's missing: grandmother and grand-grand-mother ID
#: [BII-3870]
mc_hierarchy = [
    'genMotherID']

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
    'nMCMatches']

#: PID variables
pid = [
    'kaonID',
    'pionID',
    'protonID',
    'muonID',
    'electronID',
    'deuteronID']

#: Replacement for ROEMultiplicities tool
roe_multiplicities = [
    'nROEKLMClusters']

#: Recoil kinematics relaed variables
recoil_kinematics = [
    'pRecoil',
    'pRecoilPhi',
    'pRecoilTheta',
    'pxRecoil',
    'pyRecoil',
    'pzRecoil']

#: see BII-3874
flight_info = []

#: see BII-3874
mc_flight_info = []

#: Replacement for Vertex tuple tool
#: see BII-3876
vertex = [
    'x',
    'x_uncertainty',
    'y',
    'y_uncertainty',
    'z',
    'z_uncertainty',
    'pValue']

#: Replacement for MVVertex tuple tool
#: see BII-3876
mc_vertex = vct.make_mc(vertex)

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
    'pzErr']

#: Replacement for RecoStats tool
#: [BII-3759]
reco_stats = [
    'nECLClusters',
    'nTracks',
]

#: Replacement for InvMass tool
inv_mass = [
    'M',
    'ErrM',
    'SigM',
    'InvM'
]

#: Replacement for MassBeforeFit tool
mass_before_fit = [
    'ErrM',
    'InvM'
]
