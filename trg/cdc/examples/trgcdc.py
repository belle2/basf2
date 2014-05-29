#!/user/bin/env python
#
# 2012/10/11 : param_cdcdigi, Threshold added
# 2013/11/05 : Updated for release-00-03-00
# 2014/02/12 : Updated for build-2014-01-19 //JB

from basf2 import *

#...suppress messages and warnings during processing...
set_log_level(LogLevel.ERROR)
set_random_seed(0)
basf2datadir = os.path.join(os.environ.get('BELLE2_LOCAL_DIR', None), 'data')

#...Particle Gun...
particlegun = register_module('ParticleGun')
#particlegun.param('randomSeed', 3452346)
#particlegun.param('randomSeed', 345)
#particlegun.param('randomSeed', 346)
particlegun.param('pdgCodes', [13])
particlegun.param('nTracks', 1)
particlegun.param('momentumGeneration', 'uniformPt')
#particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [10.0, 10.0])
#particlegun.param('momentumParams', [0.3, 0.3])
#particlegun.param('thetaGeneration', 'uniform')
#particlegun.param('thetaParams', [35, 127])
particlegun.param('thetaParams', [90, 90])
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [90,90])
#particlegun.param('vertexGeneration', 'fixed')
#particlegun.param('vertexGeneration', 'normal')
particlegun.param('vertexGeneration', 'uniform')
particlegun.param('xVertexParams', [-25, 25])
particlegun.param('yVertexParams', [0, 0])
particlegun.param('zVertexParams', [0, 0])

#Register modules
evtmetagen  = register_module('EventInfoSetter')
evtmetainfo = register_module('Progress')
#evtmetainfo = fw.register_module("EvtMetaInfo")
paramloader = register_module('Gearbox')
geobuilder = register_module('Geometry')
#geobuilder.log_level = LogLevel.INFO
g4sim       = register_module('FullSim')
cdcdigitizer = register_module('CDCDigitizer')
#out         = register_module('SimpleOutput')
cdctrg      = fw.register_module("TRGCDC")
#mcparticle  = fw.register_module('PrintMCParticles')

#...G4Sim...
# Turn off physics processes
#    "physics.mac" is located at "trg/examples/".
#g4sim.param('UICommands',['/control/execute physics.mac'])
# or below line can be used when trgcdc.py is not in trg/examples directory //JB
#g4sim.param('UICommands',['/control/execute ' + os.path.join(os.environ.get('BELLE2_LOCAL_DIR', None),"trg/cdc/examples/physics.mac")])

#...EvtMetaGen...
evtmetagen.param({'evtNumList': [2], 'runList': [1]})

#...GeoBuilder... Exclude detectors other than CDC
geobuilder.param('Components', ['MagneticField', 'CDC'])

#...CDC Trigger...
#cdctrg.param('ConfigFile', os.path.join(basf2datadir,"trg/cdc/TRGCDCConfig_0_20101111_1051.dat"))
cdctrg.param('ConfigFile', os.path.join(basf2datadir,"trg/cdc/TRGCDCConfig_0_20101111_1051_2013beamtest.dat"))


#cdctrg.param('makeRootFile', True)

cdctrg.param('InnerTSLUTDataFile', os.path.join(basf2datadir,"trg/cdc/LRLUTIN.coe"))
cdctrg.param('OuterTSLUTDataFile', os.path.join(basf2datadir,"trg/cdc/LRLUT.coe"))
cdctrg.param('TSFLUTSL0DataFile', os.path.join(basf2datadir,"trg/cdc/TSF.FPGA.SL0.coe"))
cdctrg.param('TSFLUTSL1DataFile', os.path.join(basf2datadir,"trg/cdc/TSF.FPGA.SL1.coe"))
cdctrg.param('TSFLUTSL2DataFile', os.path.join(basf2datadir,"trg/cdc/TSF.FPGA.SL2.coe"))
cdctrg.param('TSFLUTSL3DataFile', os.path.join(basf2datadir,"trg/cdc/TSF.FPGA.SL3.coe"))
cdctrg.param('TSFLUTSL4DataFile', os.path.join(basf2datadir,"trg/cdc/TSF.FPGA.SL4.coe"))
cdctrg.param('TSFLUTSL5DataFile', os.path.join(basf2datadir,"trg/cdc/TSF.FPGA.SL5.coe"))
cdctrg.param('TSFLUTSL6DataFile', os.path.join(basf2datadir,"trg/cdc/TSF.FPGA.SL6.coe"))
cdctrg.param('TSFLUTSL7DataFile', os.path.join(basf2datadir,"trg/cdc/TSF.FPGA.SL7.coe"))
cdctrg.param('TSFLUTSL8DataFile', os.path.join(basf2datadir,"trg/cdc/TSF.FPGA.SL8.coe"))
cdctrg.param('DebugLevel', 10)
cdctrg.param('CurlBackStop', 1)
cdctrg.param('SimulationMode', 2)
cdctrg.param('FastSimulationMode', 2)
cdctrg.param('2DFinderPerfect',1)
#cdctrg.param('HoughFinderMeshX',180)
#cdctrg.param('HoughFinderMeshY',24)
#cdctrg.param('SimulationMode',0x11)
#cdctrg.param('RootTRGCDCFile', 'TRGCDC1.root')
#cdctrg.param('RootFitter3DFile', 'Fitter3D.root')
#cdctrg.param('Fitter3DLRLUT', 0)
#cdctrg.param('TRGCDCRootFile',1)
#cdctrg.param('TSFRootFile',1)
#cdctrg.param('Hough3DRootFile',1)
#cdctrg.param('Fitter3DRootFile',1)
# 0: perfect finder, 1: Hough3DFinder, 2: (Default) GeoFinder, 3: VHDL GeoFinder
#cdctrg.param('Finder3DMode',2)
# 0: (Default) Logic TSF, 1: LUT TSF
#cdctrg.param('TSFLogicLUT', 1)

#set mcprinter
mcparticleprinter = register_module('PrintMCParticles')
mcparticleprinter.param('maxLevel',-1)

#set geometry(geobuilder)
geobuilder.param('Components', ['MagneticField', 'CDC'
                ])

#set digitizer to no smearing
param_cdcdigi = {'Fraction': 1,
                 'Resolution1': 0.,
                 'Resolution2': 0.,
                 'Threshold': -10.0}
cdcdigitizer.param(param_cdcdigi)
cdcdigitizer.param('AddInWirePropagationDelay', True)
cdcdigitizer.param('AddTimeOfFlight', True)

# For B Bbar events.
evtgeninput = register_module('EvtGenInput')
#evtgeninput.param('userDECFile', 'USER.DEC')
evtgeninput.param('boost2LAB', True)


##Create paths
main = create_path()

#Add modules to paths
main.add_module(evtmetagen)
main.add_module(evtmetainfo)
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(particlegun)
#main.add_module(evtgeninput)
main.add_module(mcparticleprinter)
main.add_module(g4sim)
main.add_module(cdcdigitizer)
main.add_module(cdctrg)
#main.add_module(out)

#Process events
process(main)

#Print call statistics
print statistics
