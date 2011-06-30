import os
from basf2 import *

#...Register modules
evtmetagen  = register_module("EvtMetaGen")
evtmetainfo = register_module("EvtMetaInfo")
paramloader = register_module("ParamLoaderXML")
geobuilder  = register_module("GeoBuilder")
pGun = register_module("PGunInput")
g4sim       = register_module("FullSim")
mcparticle = register_module('PrintMCParticles')
cdcdigitizer = register_module("CDCDigi")
#out         = register_module("SimpleOutput")
#trasan      = register_module("Trasan")
cdctrg      = register_module("TRGCDC")

#...Set parameters
evtmetagen.param('EvtNumList', [100])
paramloader.param('InputFileXML', os.path.join(basf2datadir,"simulation/Belle2.xml"))

# 3 particles with uniform momentum distribution between 0.9 an 1.1 GeV
param_pGun = {'ntracks': 1, 'p_par1': 0.9, 'p_par2': 1.1, 'th_par1':90, 'th_par2':90}
pGun.param(param_pGun)

#...CDC Trigger...
cdctrg.param('DebugLevel', 2)
cdctrg.param('ConfigFile', os.path.join(basf2datadir,"trg/TRGCDCConfig_0_20101111_1051.dat"))
cdctrg.param('CurlBackStop', 1)
cdctrg.param('HoughFinderPerfect', 1)

##Create paths
main = create_path()

##Add modules to paths
main.add_module(evtmetagen)
main.add_module(evtmetainfo)
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(pGun)
main.add_module(g4sim)
main.add_module(mcparticle)
main.add_module(cdcdigitizer)
#main.add_module(trasan)
main.add_module(cdctrg)
#main.add_module(out)

##Process events
process(main)
