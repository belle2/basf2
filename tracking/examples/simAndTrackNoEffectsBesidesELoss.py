#!/usr/bin/env python
# -*- coding: utf-8 -*-

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
geometry.param('Components', ['MagneticField', 'PXD', 'SVD', 'CDC'])

# Simulation
pGun = register_module('ParticleGun')
param_pGun = {  # this angle is only there to prevent curlers
    'pdgCodes': [-13, 13],
    'nTracks': 1,
    'momentumGeneration': 'uniform',
    'momentumParams': [1, 1],
    'thetaGeneration': 'fixed',
    'thetaParams': [120., 120.],
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
# in this example I am switching off the effects only for Muons because energy
# loss for Muons is still active which means they still produce secondary
# particles in the detector. If these particles also do not have any effects
# like decay the will curl in the detector for ever and the simulation needs
# ages to complete
g4sim.param('UICommands', [  # "/process/inactivate     Transportation mu+",
                             # "/process/inactivate             muIoni mu+",
                             # "/process/inactivate            nKiller mu+",
                             # "/process/inactivate      ExtEnergyLoss mu+",
                             # "/process/inactivate G4ErrorStepLengthLimit
                             # mu+", "/process/inactivate
                             # ExtMagFieldLimitProcess mu+",
                             # "/process/inactivate        StepLimiter mu+",
                             # "/process/inactivate     Transportation mu-",
                             # "/process/inactivate             muIoni mu-",
                             # "/process/inactivate            nKiller mu-",
                             # "/process/inactivate      ExtEnergyLoss mu-",
                             # "/process/inactivate G4ErrorStepLengthLimit
                             # mu-", "/process/inactivate
                             # ExtMagFieldLimitProcess mu-",
    '/process/list',
    '/process/inactivate                msc mu+',
    '/process/inactivate              hIoni mu+',
    '/process/inactivate            ionIoni mu+',
    '/process/inactivate              eIoni mu+',
    '/process/inactivate              eBrem mu+',
    '/process/inactivate            annihil mu+',
    '/process/inactivate               phot mu+',
    '/process/inactivate              compt mu+',
    '/process/inactivate               conv mu+',
    '/process/inactivate             hBrems mu+',
    '/process/inactivate          hPairProd mu+',
    '/process/inactivate              muMsc mu+',
    '/process/inactivate            muBrems mu+',
    '/process/inactivate         muPairProd mu+',
    '/process/inactivate        CoulombScat mu+',
    '/process/inactivate    PhotonInelastic mu+',
    '/process/inactivate     ElectroNuclear mu+',
    '/process/inactivate    PositronNuclear mu+',
    '/process/inactivate              Decay mu+',
    '/process/inactivate         hadElastic mu+',
    '/process/inactivate   NeutronInelastic mu+',
    '/process/inactivate           nCapture mu+',
    '/process/inactivate           nFission mu+',
    '/process/inactivate    ProtonInelastic mu+',
    '/process/inactivate  PionPlusInelastic mu+',
    '/process/inactivate PionMinusInelastic mu+',
    '/process/inactivate  KaonPlusInelastic mu+',
    '/process/inactivate KaonMinusInelastic mu+',
    '/process/inactivate KaonZeroLInelastic mu+',
    '/process/inactivate KaonZeroSInelastic mu+',
    '/process/inactivate AntiProtonInelastic mu+',
    '/process/inactivate AntiNeutronInelastic mu+',
    '/process/inactivate    LambdaInelastic mu+',
    '/process/inactivate AntiLambdaInelastic mu+',
    '/process/inactivate SigmaMinusInelastic mu+',
    '/process/inactivate AntiSigmaMinusInelastic mu+',
    '/process/inactivate SigmaPlusInelastic mu+',
    '/process/inactivate AntiSigmaPlusInelastic mu+',
    '/process/inactivate   XiMinusInelastic mu+',
    '/process/inactivate AntiXiMinusInelastic mu+',
    '/process/inactivate    XiZeroInelastic mu+',
    '/process/inactivate AntiXiZeroInelastic mu+',
    '/process/inactivate OmegaMinusInelastic mu+',
    '/process/inactivate AntiOmegaMinusInelastic mu+',
    '/process/inactivate CHIPSNuclearCaptureAtRest mu+',
    '/process/inactivate muMinusCaptureAtRest mu+',
    '/process/inactivate  DeuteronInelastic mu+',
    '/process/inactivate    TritonInelastic mu+',
    '/process/inactivate          ExtNoHits mu+',
    '/process/inactivate       OpAbsorption mu+',
    '/process/inactivate         OpRayleigh mu+',
    '/process/inactivate            OpMieHG mu+',
    '/process/inactivate         OpBoundary mu+',
    '/process/inactivate              OpWLS mu+',
    '/process/inactivate           Cerenkov mu+',
    '/process/inactivate      Scintillation mu+',
    '/process/inactivate                msc mu-',
    '/process/inactivate              hIoni mu-',
    '/process/inactivate            ionIoni mu-',
    '/process/inactivate              eIoni mu-',
    '/process/inactivate              eBrem mu-',
    '/process/inactivate            annihil mu-',
    '/process/inactivate               phot mu-',
    '/process/inactivate              compt mu-',
    '/process/inactivate               conv mu-',
    '/process/inactivate             hBrems mu-',
    '/process/inactivate          hPairProd mu-',
    '/process/inactivate              muMsc mu-',
    '/process/inactivate            muBrems mu-',
    '/process/inactivate         muPairProd mu-',
    '/process/inactivate        CoulombScat mu-',
    '/process/inactivate    PhotonInelastic mu-',
    '/process/inactivate     ElectroNuclear mu-',
    '/process/inactivate    PositronNuclear mu-',
    '/process/inactivate              Decay mu-',
    '/process/inactivate         hadElastic mu-',
    '/process/inactivate   NeutronInelastic mu-',
    '/process/inactivate           nCapture mu-',
    '/process/inactivate           nFission mu-',
    '/process/inactivate    ProtonInelastic mu-',
    '/process/inactivate  PionPlusInelastic mu-',
    '/process/inactivate PionMinusInelastic mu-',
    '/process/inactivate  KaonPlusInelastic mu-',
    '/process/inactivate KaonMinusInelastic mu-',
    '/process/inactivate KaonZeroLInelastic mu-',
    '/process/inactivate KaonZeroSInelastic mu-',
    '/process/inactivate AntiProtonInelastic mu-',
    '/process/inactivate AntiNeutronInelastic mu-',
    '/process/inactivate    LambdaInelastic mu-',
    '/process/inactivate AntiLambdaInelastic mu-',
    '/process/inactivate SigmaMinusInelastic mu-',
    '/process/inactivate AntiSigmaMinusInelastic mu-',
    '/process/inactivate SigmaPlusInelastic mu-',
    '/process/inactivate AntiSigmaPlusInelastic mu-',
    '/process/inactivate   XiMinusInelastic mu-',
    '/process/inactivate AntiXiMinusInelastic mu-',
    '/process/inactivate    XiZeroInelastic mu-',
    '/process/inactivate AntiXiZeroInelastic mu-',
    '/process/inactivate OmegaMinusInelastic mu-',
    '/process/inactivate AntiOmegaMinusInelastic mu-',
    '/process/inactivate CHIPSNuclearCaptureAtRest mu-',
    '/process/inactivate muMinusCaptureAtRest mu-',
    '/process/inactivate  DeuteronInelastic mu-',
    '/process/inactivate    TritonInelastic mu-',
    '/process/inactivate          ExtNoHits mu-',
    '/process/inactivate       OpAbsorption mu-',
    '/process/inactivate         OpRayleigh mu-',
    '/process/inactivate            OpMieHG mu-',
    '/process/inactivate         OpBoundary mu-',
    '/process/inactivate              OpWLS mu-',
    '/process/inactivate           Cerenkov mu-',
    '/process/inactivate      Scintillation mu-',
    ])
g4sim.param('StoreAllSecondaries', True)  # this is need for the MCTrackFinder to work correctly
# "/process/inactivate        StepLimiter mu-"
#
# digitizer
cdcDigitizer = register_module('CDCDigitizer')

# use one gaussian with resolution of 0.01 in the digitizer (to simplify the
# fitting)
param_cdcdigi = {'Fraction': 1, 'Resolution1': 0.01, 'Resolution2': 0.0}
cdcDigitizer.param(param_cdcdigi)
mctrackfinder = register_module('MCTrackFinder')
param_mctrackfinder = {
    'UseCDCHits': 1,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'Smearing': 0,
    }
mctrackfinder.param(param_mctrackfinder)
# mctrackfinder.logging.log_level = LogLevel.WARNING

trackfitter = register_module('GenFitter2')
trackfitter.logging.log_level = LogLevel.WARNING
trackfitter.param('noiseCoulomb', False)
# you do not have to switch off bremsstrahlung in Genfit because it is only
# active for electrons (in Genfit!) anyway and this steering file uses only
# Muons
trackfitter.param('filterIterations', 2)
trackfitchecker = register_module('TrackFitChecker')
trackfitchecker.logging.log_level = LogLevel.INFO  # the results of the
                                                   # statistical tests will
                                                   # only show up at info or
                                                   # debug level
trackfitchecker.param('testSi', False)
trackfitchecker.param('writeToTextFile', True)
trackfitchecker.param('writeToRootFile', True)
trackfitchecker.param('inspectTracks', False)
trackfitchecker.param('truthAvailable', False)
trackfitchecker.param('outputFileName', 'simAndTrackNoEffectsBesidesELoss')

# Create paths
main = create_path()

# Add modules to paths
main.add_module(evtmetagen)
main.add_module(evtmetainfo)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(pGun)
main.add_module(g4sim)
main.add_module(cdcDigitizer)
main.add_module(mctrackfinder)
main.add_module(trackfitter)
main.add_module(trackfitchecker)

# Process events
process(main)
print statistics
