#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
#
# This is an example file to run ARICHDatabaseModule.
#
##############################################################################

import os
import sys
import glob
from basf2 import *

if len(sys.argv) != 3:
    sys.exit('Must provide enough arguments: [R/W] [global tag] - R/W:Wagel, Ragel, WhapdQA, RhapdQA'
             )

# use central database (comment out to use local payloads)
use_central_database(sys.argv[2], LogLevel.WARNING)


# register modules
# set parameters in modules
eventinfosetter = register_module("EventInfoSetter")
param_eventinfosetter = {'expList': [0],
                         'runList': [0],
                         'evtNumList': [1]}
eventinfosetter.param(param_eventinfosetter)

eventinfoprinter = register_module("EventInfoPrinter")

# set parameter 'wr' to read (R) or write (W) mode - Wagel, Ragel, WhapdQA, RhapdQA
arichdb = register_module("ARICHDatabase")
arichdb.param('wr', sys.argv[1])
rootfiles = glob.glob('/home/manca/belle2/head/arich/dbdata/*_data.root')
arichdb.param('InputFileNames', rootfiles)

# conditions database
conditionsdb = register_module("Conditions")
param_conditionsdb = {'experimentName': '0',
                      'runName': '0',
                      #                      'globalTag': 'testAerogel'
                      'globalTag': sys.argv[2]
                      }
conditionsdb.param(param_conditionsdb)

# load parameters from xml files - available at https://belle2.cc.kek.jp/browse/viewvc.cgi/svn/groups/arich/database/data/aerogel
paramloader = register_module('Gearbox')
xmlfile = 'file:///home/manca/belle2/head/arich/dbdata/AerogelData.xml'
paramloader.param('fileName', xmlfile)


# create path and add modules
main = create_path()
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(conditionsdb)
main.add_module(paramloader)
main.add_module(arichdb)

# process
process(main)
