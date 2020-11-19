#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
from ROOT.Belle2 import KLMLikelihoodParametersImporter

basf2.set_log_level(basf2.LogLevel.INFO)

eventinfo = basf2.register_module('EventInfoSetter')
eventinfo.param({'evtNumList': [0], 'runList': [0]})
paramloader = basf2.register_module('Gearbox')

main = basf2.create_path()
main.add_module(eventinfo)
main.add_module(paramloader)
basf2.process(main)

# run the MuidParameters writer, create dbobject payload in ./localdb
dbWriter = KLMLikelihoodParametersImporter()
dbWriter.writeLikelihoodParameters()

# run the MuidParameters reader
# dbReader = MuidParameterDBReaderWriter()
# dbReader.readMuidParameters()
