#!/usr/bin/env python
# -*- coding: utf-8 -*-

# WARNING THIS EXAMPLES ONLY WORKS IF ONE MANULAY EDITS THE FILES SVD.xml IN
# svd/data AND PXD.xml IN pxd/data MORE PRESISELY ONE HAS TO CHANGE THE LINE
# <SeeNeutrons desc="Set to true to make sensitive detectors see
# neutrons">False</SeeNeutrons> TO <SeeNeutrons desc="Set to true to make
# sensitive detectors see neutrons">True</SeeNeutrons> IN BOTH FILES

import os
from basf2 import *
set_log_level(LogLevel.ERROR)
set_random_seed(1)
# Register necessary modules
evtmetagen = register_module('EvtMetaGen')
evtmetainfo = register_module('EvtMetaInfo')
evtmetagen.param('ExpList', [0])
evtmetagen.param('RunList', [1])
evtmetagen.param('EvtNumList', [5000])

# Geometry parameter loader
gearbox = register_module('Gearbox')
# Geometry builder
geometry = register_module('Geometry')
geometry.param('Components', ['MagneticField', 'PXD', 'SVD'])

# Simulation
pGun = register_module('ParticleGun')
param_pGun = {  # this angle is only there to prevent curlers
    'pdgCodes': [-211, 211],
    'nTracks': 1,
    'momentumGeneration': 'uniform',
    'momentumParams': [1, 1],
    'thetaGeneration': 'fixed',
    'thetaParams': [110., 110.],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
    }
pGun.param(param_pGun)
g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)  # this is need for the MCTrackFinder to work correctly
g4sim.param('UICommands', [  # "/process/inactivate     Transportation",
                             # "/process/inactivate            nKiller",
                             # "/process/inactivate G4ErrorStepLengthLimit",
                             # "/process/inactivate ExtMagFieldLimitProcess",
                             # "/process/inactivate          ExtNoHits",
    '/process/list',
    '/process/inactivate                msc',
    '/process/inactivate              hIoni',
    '/process/inactivate            ionIoni',
    '/process/inactivate              eIoni',
    '/process/inactivate              eBrem',
    '/process/inactivate            annihil',
    '/process/inactivate               phot',
    '/process/inactivate              compt',
    '/process/inactivate               conv',
    '/process/inactivate             hBrems',
    '/process/inactivate          hPairProd',
    '/process/inactivate              muMsc',
    '/process/inactivate             muIoni',
    '/process/inactivate            muBrems',
    '/process/inactivate         muPairProd',
    '/process/inactivate        CoulombScat',
    '/process/inactivate    PhotonInelastic',
    '/process/inactivate     ElectroNuclear',
    '/process/inactivate    PositronNuclear',
    '/process/inactivate              Decay',
    '/process/inactivate         hadElastic',
    '/process/inactivate   NeutronInelastic',
    '/process/inactivate           nCapture',
    '/process/inactivate           nFission',
    '/process/inactivate    ProtonInelastic',
    '/process/inactivate  PionPlusInelastic',
    '/process/inactivate PionMinusInelastic',
    '/process/inactivate  KaonPlusInelastic',
    '/process/inactivate KaonMinusInelastic',
    '/process/inactivate KaonZeroLInelastic',
    '/process/inactivate KaonZeroSInelastic',
    '/process/inactivate AntiProtonInelastic',
    '/process/inactivate AntiNeutronInelastic',
    '/process/inactivate    LambdaInelastic',
    '/process/inactivate AntiLambdaInelastic',
    '/process/inactivate SigmaMinusInelastic',
    '/process/inactivate AntiSigmaMinusInelastic',
    '/process/inactivate SigmaPlusInelastic',
    '/process/inactivate AntiSigmaPlusInelastic',
    '/process/inactivate   XiMinusInelastic',
    '/process/inactivate AntiXiMinusInelastic',
    '/process/inactivate    XiZeroInelastic',
    '/process/inactivate AntiXiZeroInelastic',
    '/process/inactivate OmegaMinusInelastic',
    '/process/inactivate AntiOmegaMinusInelastic',
    '/process/inactivate CHIPSNuclearCaptureAtRest',
    '/process/inactivate muMinusCaptureAtRest',
    '/process/inactivate  DeuteronInelastic',
    '/process/inactivate    TritonInelastic',
    '/process/inactivate      ExtEnergyLoss',
    '/process/inactivate       OpAbsorption',
    '/process/inactivate         OpRayleigh',
    '/process/inactivate            OpMieHG',
    '/process/inactivate         OpBoundary',
    '/process/inactivate              OpWLS',
    '/process/inactivate           Cerenkov',
    '/process/inactivate      Scintillation',
    ])
# "/process/inactivate        StepLimiter"

mctrackfinder = register_module('MCTrackFinder')
param_mctrackfinder = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'Smearing': 0,
    }
mctrackfinder.param(param_mctrackfinder)
trackfitter = register_module('GenFitter2')
trackfitter.logging.log_level = LogLevel.WARNING
trackfitter.param('energyLossBetheBloch', False)
trackfitter.param('noiseBetheBloch', False)
trackfitter.param('noiseCoulomb', False)
# you do not have to switch off bremsstrahlung in Genfit becuse it is only
# active for electrons (in Genfit!) anyway and this steering file uses only
# Pions
trackfitter.param('filterIterations', 2)
trackfitchecker = register_module('TrackFitChecker')
trackfitchecker.logging.log_level = LogLevel.INFO  # the reults of the
                                                   # statistical tests will
                                                   # only show up at info or
                                                   # debug level
trackfitchecker.param('testSi', False)
trackfitchecker.param('writeToTextFile', True)
trackfitchecker.param('writeToRootFile', True)
trackfitchecker.param('inspectTracks', False)
trackfitchecker.param('truthAvailable', False)
trackfitchecker.param('outputFileName', 'simAndTrackNoEffectsAtAll')

# Create paths
main = create_path()

# Add modules to paths
main.add_module(evtmetagen)
main.add_module(evtmetainfo)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(pGun)
main.add_module(g4sim)
main.add_module(mctrackfinder)
main.add_module(trackfitter)
main.add_module(trackfitchecker)

# Process events
process(main)
print statistics
