import os
from basf2 import *

#...Modules...
evtmetagen  = register_module("EvtMetaGen")
evtmetainfo = register_module("EvtMetaInfo")
paramloader = register_module("Gearbox")
geobuilder = register_module("Geometry")
geobuilder.log_level = LogLevel.INFO
g4sim       = register_module("FullSim")
cdcdigitizer = register_module("CDCDigi")
out         = register_module("SimpleOutput")
#trasan      = fw.register_module("Trasan")
cdctrg      = register_module("TRGCDC")
pGun        = register_module("PGunInput")
mcparticle  = register_module('PrintMCParticles')

#...Events to generate...
evtmetagen.param({'EvtNumList':[100], 'RunList': [1]})

#...Particle gun...
pGun.param('nTracks',1)
pGun.param('pPar1',3.0)
pGun.param('pPar2',3.0)
pGun.param('thetaPar1',90)
pGun.param('thetaPar2',90)

#...CDC Trigger...
cdctrg.param('ConfigFile', os.path.join(basf2datadir,"trg/TRGCDCConfig_0_20101111_1051.dat"))
cdctrg.param('DebugLevel',2)
cdctrg.param('CurlBackStop',1)
cdctrg.param('HoughFinderPerfect',1)

#...Path...
main = fw.create_path()
main.add_module(evtmetagen)
main.add_module(evtmetainfo)
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(pGun)
main.add_module(g4sim)
main.add_module(cdcdigitizer)
#main.add_module(trasan)
main.add_module(cdctrg)
main.add_module(out)

#...Process events...
fw.process(main)
