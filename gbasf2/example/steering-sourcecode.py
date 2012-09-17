#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *
set_log_level(LogLevel.INFO)
project = 'Temp4'
type = 'gen-mc'
swver = 'build-2012-07-18'
# site = 'LCG.KISTI.kr'
site = 'LCG.DESY.de'
# site = 'OSG.FNAL.us'
# site = 'LCG.CYFRONET.pl'
# site = 'OSG.TTU.us'
# site = 'OSG.Nebraska.us'
# site = 'LCG.SIGNET.si'
# site = 'OSG.VT.us'
# site = 'LCG.KIT.de'
# site = 'OSG.UTA.us'
# site = 'LCG.KEK2.jp'
query = 'id<2'
inputsandboxfiles = 'MyPackage'
evtmetagen = register_module('EvtMetaGen')
tmp = register_module('Template')
evtmetagen.param('EvtNumList', [3])
evtmetagen.param('RunList', [1])
evtmetagen.param('ExpList', [1])
main = create_path()
main.add_module(evtmetagen)
main.add_module(tmp)
process(main)
LFN = 'None'
