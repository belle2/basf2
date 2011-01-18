#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *

histoman = fw.register_module('HistoManager')
input = fw.register_module('pRootInput')
evtmetagen = fw.register_module('EvtMetaGen')
evtmetainfo = fw.register_module('EvtMetaInfo')
paramloader = fw.register_module('ParamLoaderXML')
geobuilder = fw.register_module('GeoBuilder')
g4sim = fw.register_module('SimModule')
cdcdigitizer = fw.register_module('CDCDigitizer')
output = fw.register_module('HLTOutput')

param_evtmetagen = {
    'ExpList': [1],
    'RunList': [1],
    'EvtStartList': [0],
    'EvtEndList': [0],
    }

evtmetagen.param(param_evtmetagen)

paramloader.param('InputFileXML', os.path.join(basf2datadir,
                  'simulation/Belle2.xml'))

param_g4sim = {'RandomSeed': 6489, 'MacroName': 'g4sim.macro'}

g4sim.param(param_g4sim)

output.param('dest', '210.119.41.211')
output.param('port', 20000)

main = fw.create_path()

main.add_module(evtmetagen)
# main.add_module (input)
main.add_module(evtmetainfo)
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(g4sim)
main.add_module(cdcdigitizer)
main.add_module(output)

# fw.set_nprocess (2)
fw.process(main)

