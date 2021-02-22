#!/usr/bin/env python3
# -*- coding: utf-8 -*-

################################
# Save all good charged tracks #
################################
import basf2
import sys
import statistics
from modularAnalysis import inputMdst, fillParticleList, matchMCTruth, variablesToNtuple
from variables import variables

charged_path = basf2.Path()

variables.addAlias('kBinaryID', 'pidPairProbabilityExpert(321,211,ALL)')
variables.addAlias('eBinaryID', 'pidPairProbabilityExpert(11,211,ALL)')
variables.addAlias('muBinaryID', 'pidPairProbabilityExpert(13,211,ALL)')
variables.addAlias('pBinaryID', 'pidPairProbabilityExpert(2212,211,ALL)')

if len(sys.argv) not in [2, 3]:
    sys.exit('Must provide an input file and (optionally) an output file')

inFile = sys.argv[1]
if len(sys.argv) == 3:
    outFile = sys.argv[2]
else:
    outFile = './truth_charged.root'

inputMdst('default', inFile, path=charged_path)

# --------------------------------------------------
# Create and fill final state ParticleLists
# --------------------------------------------------
trackQuality = 'thetaInCDCAcceptance and nCDCHits > 20'
ipCut = 'd0 < 0.5 and abs(z0) < 2'
mcCharged = '(abs(mcPDG)==11 or abs(mcPDG)==13 or abs(mcPDG)==211 or abs(mcPDG)==321 or abs(mcPDG)==2212)'
goodTrack = trackQuality + ' and ' + ipCut

fillParticleList('pi+:any', goodTrack, path=charged_path)
matchMCTruth('pi+:any', path=charged_path)

fillParticleList('K+:any', goodTrack, path=charged_path)
matchMCTruth('K+:any', path=charged_path)

fillParticleList('e+:any', goodTrack, path=charged_path)
matchMCTruth('e+:any', path=charged_path)

fillParticleList('mu+:any', goodTrack, path=charged_path)
matchMCTruth('mu+:any', path=charged_path)

fillParticleList('p+:any', goodTrack, path=charged_path)
matchMCTruth('p+:any', path=charged_path)

# --------------------------------------------------
# write out useful information to a ROOT file
# --------------------------------------------------
ntupleOfInterest = ['px', 'py', 'pz', 'E', 'p', 'pt', 'theta', 'phi',  # event kinematics
                    'mcPX', 'mcPY', 'mcPZ', 'mcE', 'mcP', 'mcPT', 'mcTheta', 'mcPhi',  # mc kinematics
                    'pionID', 'kaonID', 'electronID', 'muonID', 'protonID',  # global PID
                    'kBinaryID', 'eBinaryID', 'muBinaryID', 'pBinaryID',  # binary PID
                    'thetaInTOPAcceptance', 'thetaInECLAcceptance', 'thetaInKLMAcceptance',  # angular acceptance
                    'ptInTOPAcceptance', 'ptInBECLAcceptance', 'ptInBKLMAcceptance',  # momentum acceptance
                    'mcPDG',  # true PDG number
                    ]

variablesToNtuple('pi+:any', ntupleOfInterest, 'pion', outFile, path=charged_path)
variablesToNtuple('K+:any', ntupleOfInterest, 'kaon', outFile, path=charged_path)
variablesToNtuple('e+:any', ntupleOfInterest, 'electron', outFile, path=charged_path)
variablesToNtuple('mu+:any', ntupleOfInterest, 'muon', outFile, path=charged_path)
variablesToNtuple('p+:any', ntupleOfInterest, 'proton', outFile, path=charged_path)

# --------------------------------------------------
# Process the events and print call statistics
# --------------------------------------------------
basf2.process(charged_path)
print(statistics)
