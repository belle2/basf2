#!/usr/bin/env python3
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
    sys.exit('Must provide 2 arguments: [R/W option] [global tag] - check README_DB for more info'
             )

# use local/central database
# use_central_database(sys.argv[2], LogLevel.WARNING)
use_local_database("test_database.txt", "test_payloads", LogLevel.WARNING)

# register modules
# set parameters in modules
eventinfosetter = register_module("EventInfoSetter")
param_eventinfosetter = {'expList': [0],
                         'runList': [0],
                         'evtNumList': [1]}
eventinfosetter.param(param_eventinfosetter)

eventinfoprinter = register_module("EventInfoPrinter")

# set parameter 'wr' to read (R) or write (W) mode - Wagel, Ragel, WhapdQA, RhapdQA, Wasic, Rasic, WhapdInfo
arichdb = register_module("ARICHDatabase")
arichdb.param('wr', sys.argv[1])
rootfiles = glob.glob('/home/manca/belle2/head/arich/dbdata/*_data.root')
arichdb.param('InputFileNames', rootfiles)

# conditions database
conditionsdb = register_module("Conditions")

# load parameters from xml/root files - check README_DB for more info
paramloader = register_module('Gearbox')
# xmlfile = 'file:///home/manca/belle2/head/arich/dbdata/aerogelData/AerogelData.xml'
xmlfile = 'file:///home/manca/belle2/head/arich/dbdata/hapdXmlData/hapdData.xml'
paramloader.param('fileName', xmlfile)

xdir = '/home/manca/belle2/head/arich/dbdata/asicData/asicsA'

if (sys.argv[1] == "Wasic"):
    rootfiles2a = []
    rootfiles2b = []
    rootfiles2c = []
    rootfiles2d = []
    rootfiles2e = []
    for path, subdirs, files in os.walk(xdir):
        path1, path2 = path.split("/asics")
        pathasic = path2.split("/")
        stringasic = ''.join(pathasic)
        print(pathasic, ", ", stringasic)
        for name in files:
            name1, name2 = name.split(".root")
            name3, name4 = name1.split(stringasic)
            nameNUM = int(name4)
            if (nameNUM < 20):
                rootfiles2a.append(os.path.join(path, name))
            if ((nameNUM > 19) and (nameNUM < 40)):
                rootfiles2b.append(os.path.join(path, name))
            if ((nameNUM > 39) and (nameNUM < 60)):
                rootfiles2c.append(os.path.join(path, name))
            if ((nameNUM > 59) and (nameNUM < 80)):
                rootfiles2d.append(os.path.join(path, name))
            if (nameNUM > 79):
                rootfiles2e.append(os.path.join(path, name))
    for i in range(0, 5):
        globalTag = sys.argv[2] + "_" + stringasic + "_" + str(i)
        if (i == 0):
            arichdb.param('InputFileNames2', rootfiles2a)
            arichdb.param('GlobalTag', globalTag)
            param_conditionsdb = {'experimentName': '0',
                                  'runName': '0',
                                  'globalTag': globalTag
                                  }
            conditionsdb.param(param_conditionsdb)
            main = create_path()
            main.add_module(eventinfosetter)
            main.add_module(eventinfoprinter)
            main.add_module(conditionsdb)
            main.add_module(paramloader)
            main.add_module(arichdb)
            process(main)
        if (i == 1):
            arichdb.param('InputFileNames2', rootfiles2b)
            arichdb.param('GlobalTag', globalTag)
            param_conditionsdb = {'experimentName': '0',
                                  'runName': '0',
                                  'globalTag': globalTag
                                  }
            conditionsdb.param(param_conditionsdb)
            main = create_path()
            main.add_module(eventinfosetter)
            main.add_module(eventinfoprinter)
            main.add_module(conditionsdb)
            main.add_module(paramloader)
            main.add_module(arichdb)
            process(main)
        if (i == 2):
            arichdb.param('InputFileNames2', rootfiles2c)
            arichdb.param('GlobalTag', globalTag)
            param_conditionsdb = {'experimentName': '0',
                                  'runName': '0',
                                  'globalTag': globalTag
                                  }
            conditionsdb.param(param_conditionsdb)
            main = create_path()
            main.add_module(eventinfosetter)
            main.add_module(eventinfoprinter)
            main.add_module(conditionsdb)
            main.add_module(paramloader)
            main.add_module(arichdb)
            process(main)
        if (i == 3):
            arichdb.param('InputFileNames2', rootfiles2d)
            arichdb.param('GlobalTag', globalTag)
            param_conditionsdb = {'experimentName': '0',
                                  'runName': '0',
                                  'globalTag': globalTag
                                  }
            conditionsdb.param(param_conditionsdb)
            main = create_path()
            main.add_module(eventinfosetter)
            main.add_module(eventinfoprinter)
            main.add_module(conditionsdb)
            main.add_module(paramloader)
            main.add_module(arichdb)
            process(main)
        if (i == 4):
            arichdb.param('InputFileNames2', rootfiles2e)
            arichdb.param('GlobalTag', globalTag)
            param_conditionsdb = {'experimentName': '0',
                                  'runName': '0',
                                  'globalTag': globalTag
                                  }
            conditionsdb.param(param_conditionsdb)
            main = create_path()
            main.add_module(eventinfosetter)
            main.add_module(eventinfoprinter)
            main.add_module(conditionsdb)
            main.add_module(paramloader)
            main.add_module(arichdb)
            process(main)

elif (sys.argv[1] == "Rasic"):
    param_conditionsdb = {'experimentName': '0',
                          'runName': '0',
                          'globalTag': sys.argv[2]
                          }
    arichdb.param('GlobalTag', sys.argv[2])
    conditionsdb.param(param_conditionsdb)
    main = create_path()
    main.add_module(eventinfosetter)
    main.add_module(eventinfoprinter)
    main.add_module(conditionsdb)
    main.add_module(paramloader)
    main.add_module(arichdb)
    process(main)

else:
    param_conditionsdb = {'experimentName': '0',
                          'runName': '0',
                          'globalTag': sys.argv[2]
                          }
    arichdb.param('GlobalTag', sys.argv[2])
    conditionsdb.param(param_conditionsdb)
    main = create_path()
    main.add_module(eventinfosetter)
    main.add_module(eventinfoprinter)
    main.add_module(conditionsdb)
    main.add_module(paramloader)
    main.add_module(arichdb)
    process(main)
