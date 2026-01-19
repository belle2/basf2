#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
#                                                                        #
# This tutorial demonstrates how to apply ModeSelector to FEI B meson    #
# candidates for improved signal probability estimation.                  #
#                                                                        #
# ModeSelector is a neural network that considers information from all   #
# B candidates in an event to provide a better signal probability score. #
#                                                                        #
##########################################################################

# import sys
#
# sys.path.insert(0, '/home/pf/basf2/analysis/scripts')
import basf2 as b2
import modeSelector
import modularAnalysis as ma
from stdPi0s import stdPi0s

# Set up logging
b2.set_log_level(b2.LogLevel.INFO)

# Create path
my_path = b2.create_path()

# Input file (should be FEI output with B meson candidates)
# For this example, we assume a file with B+:feiHadronic and B0:feiHadronic lists
ma.inputMdstList(
    filelist=['/home/pf/dataframes/MC16rd_skim/udst_000001_prod00051442_task230000001.root'],
    path=my_path
)

# Prepend the analysis globaltag for accessing payloads
b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())

# FEI list identifier
fei_identifier = 'feiHadronic'

# Define ROE masks for continuum suppression
track_mask = "[[dr < 2] and [abs(dz) < 4] and [pt > 0.2] and [thetaInCDCAcceptance==1]]"
ecl_mask = ("[[[[clusterReg==1] and [E>0.080]] or [[clusterReg==2] and [E > 0.03]] "
            "or [[clusterReg==3] and [E > 0.06]]] and [clusterNHits > 1.5] "
            "and [abs(clusterTiming) < 200] and [thetaInCDCAcceptance==1]]")
cleanMask = ("cleanMask", track_mask, ecl_mask)

# Apply FEI calibration cuts and build continuum suppression
for b in ['B+', 'B0']:
    # Apply cuts (matching FEI calibration)
    ma.applyCuts(f'{b}:{fei_identifier}', '[Mbc > 5.22] and [-0.15 < deltaE < 0.1]', path=my_path)

    # Build the Rest of Event
    ma.buildRestOfEvent(f'{b}:{fei_identifier}', path=my_path)
    ma.appendROEMasks(f'{b}:{fei_identifier}', [cleanMask], path=my_path)

    # Build continuum suppression (provides cosTBTO variable)
    ma.buildContinuumSuppression(f'{b}:{fei_identifier}', 'cleanMask', path=my_path)

    # Apply cosTBTO cut
    ma.applyCuts(f'{b}:{fei_identifier}', 'cosTBTO < 0.9', path=my_path)

# Build event shape variables (sphericity, thrust, etc.)
ma.buildEventShape(
    allMoments=False,
    cleoCones=False,
    jets=False,
    collisionAxis=False,
    harmonicMoments=True,
    foxWolfram=True,
    sphericity=True,
    thrust=True,
    path=my_path
)

# Create standard pi0 list needed for D* veto reconstruction
# Using MC16rd weights for background suppression
beamBackgroundMVAWeight = "MC16rd"
fakePhotonMVAWeight = "MC16rd"
stdPi0s('eff50_May2020Fit', path=my_path, beamBackgroundMVAWeight=beamBackgroundMVAWeight,
        fakePhotonMVAWeight=fakePhotonMVAWeight)

# Apply additional pi0 cuts (matching training preprocessing)
pi0Cuts = '[useCMSFrame(p) < 0.5]'
pi0Cuts += ' and [daughter(0,beamBackgroundSuppression) > 0.5] and [daughter(0,fakePhotonSuppression) > 0.1]'
pi0Cuts += ' and [daughter(1,beamBackgroundSuppression) > 0.5] and [daughter(1,fakePhotonSuppression) > 0.1]'
ma.applyCuts('pi0:eff50_May2020Fit', pi0Cuts, path=my_path)

# Define the particle lists to process
particle_lists = ['B+:feiHadronic', 'B0:feiHadronic']

# Add D* veto reconstruction
for plist in particle_lists:
    modeSelector.addDstarVeto(plist, path=my_path)

# Apply ModeSelector with local model files (for testing/development)
# Set debug=True to print feature values for comparison
modeSelector.modeSelector(
    particleLists=particle_lists,
    cat_model_path='modeSelector_test/cat_model.onnx',
    main_model_path='modeSelector_test/main_model.onnx',
    has_inputs_path='modeSelector_test/has_inputs.txt',
    output_variable='BplusScore',
    debug=True,  # Enable debug output
    debug_max_events=3,  # Print first 3 events
    path=my_path
)

# Define output variables
output_variables = [
    # Basic kinematics
    'Mbc', 'deltaE', 'M',
    # Continuum suppression
    'cosTBTO',
    # FEI signal probability
    'extraInfo(SignalProbability)',
    'extraInfo(decayModeID)',
    # ModeSelector output
    'extraInfo(BplusScore)',
    'extraInfo(BplusScore_catB0)',
    'extraInfo(BplusScore_catBp)',
    'extraInfo(BplusScore_catCont)',
    # D* veto variables (if addDstarVeto was used)
    'extraInfo(Dstp_deltaMassDiff)',
    'extraInfo(Dstp_chiProb)',
    'extraInfo(Dst0_deltaMassDiff)',
    'extraInfo(Dst0_chiProb)',
]

# Write output ntuple for B+ candidates
ma.variablesToNtuple(
    'B+:feiHadronic',
    variables=output_variables,
    filename='modeSelector_output_Bp.root',
    treename='Bp',
    path=my_path
)

# Write output ntuple for B0 candidates
ma.variablesToNtuple(
    'B0:feiHadronic',
    variables=output_variables,
    filename='modeSelector_output_B0.root',
    treename='B0',
    path=my_path
)

# Process events
b2.process(my_path)

# Print statistics
print(b2.statistics)
